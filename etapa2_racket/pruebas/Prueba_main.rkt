#lang racket

;; prueba-main.rkt
;; Pruebas de main.rkt (integracion de lectura, generacion, filtros y exportacion).
;; Ejecutar con:  racket prueba-main.rkt
;;
;; Hay dos tipos de pruebas:
;; Interpretacion: los cursos de interes son opciones; cada combinacion usa un subconjunto
;; de ellos cuyos creditos caen en el rango.
;;
;;  - Con un catalogo sintetico chiquito, donde el resultado correcto se puede contar a mano.
;;  - Con el catalogo real exportado por C (catalogo_salida.json).
;;    Los numeros esperados se verificaron aparte con una fuerza bruta independiente (Python).

(require rackunit
         racket/runtime-path
         racket/stream
         json
         "modelos.rkt"
         "choques.rkt"
         "lectura.rkt"
         "main.rkt")

(define-runtime-path ruta-catalogo-real "catalogo_salida.json")
(define-runtime-path ruta-preferencias-ejemplo "preferencias_ejemplo.json")

;; ------------------------------------------------------------------
;; Ayudas para armar datos de prueba
;; ------------------------------------------------------------------

;; Un grupo con id dado, que choca con la lista de pares (curso . id)
(define (mk-grupo id choca-con)
  (grupo id (~a "00" id) "Regular" '("Profe") '("Lunes - 7:30:9:20")
         (map (lambda (par) (choque (car par) (cdr par))) choca-con)))

;; Un curso habilitado para matricular
(define (mk-curso codigo creditos grupos
                  #:coreq [coreq '()] #:carreras [carreras '("Computadores")]
                  #:aprobado [aprobado #f])
  (curso codigo (~a "Curso " codigo) creditos '() coreq carreras aprobado #t grupos))

(define (prefs minimo maximo . intereses)
  (preferencias minimo maximo (map (lambda (i) (interes (car i) (cdr i))) intereses)))

;; Convierte el stream completo a lista (solo para pruebas chicas)
(define (todas catalogo p) (stream->list (construir-combinaciones catalogo p)))

;; Una combinacion como lista de pares (curso . id-grupo), facil de comparar
(define (pares comb)
  (map (lambda (s) (cons (curso-codigo (seleccion-curso s)) (grupo-id (seleccion-grupo s))))
       (combinacion-selecciones comb)))

;; Catalogo sintetico: A (3 cr) y B (4 cr), 2 grupos cada uno; A1 choca con B1
(define cat-ab
  (list (mk-curso "A" 3 (list (mk-grupo 1 '(("B" . 1))) (mk-grupo 2 '())))
        (mk-curso "B" 4 (list (mk-grupo 1 '(("A" . 1))) (mk-grupo 2 '())))))

;; Escribe un archivo temporal con el texto dado y devuelve su ruta
(define (archivo-temporal texto)
  (define ruta (make-temporary-file "prueba-main-~a.json"))
  (call-with-output-file ruta #:exists 'replace (lambda (o) (write-string texto o)))
  ruta)

;; ------------------------------------------------------------------
;; sumar-creditos / selecciones->combinacion
;; ------------------------------------------------------------------
(test-case "sumar-creditos suma los creditos de los cursos"
  (define a (mk-curso "A" 3 (list (mk-grupo 1 '()))))
  (define b (mk-curso "B" 4 (list (mk-grupo 1 '()))))
  (define sa (seleccion a (car (curso-grupos a))))
  (define sb (seleccion b (car (curso-grupos b))))
  (check-equal? (sumar-creditos '()) 0)
  (check-equal? (sumar-creditos (list sa)) 3)
  (check-equal? (sumar-creditos (list sa sb)) 7)
  (check-equal? (combinacion-creditos (selecciones->combinacion (list sa sb))) 7))

;; ------------------------------------------------------------------
;; Generacion sin choques (catalogo sintetico)
;; ------------------------------------------------------------------
(test-case "A1 choca con B1: de las 4 combinaciones posibles quedan 3"
  ;; rango 7-7: solo sirve tomar A y B juntos
  (define resultado (map pares (todas cat-ab (prefs 7 7 '("A" 1 2) '("B" 1 2)))))
  (check-equal? (length resultado) 3)
  (check-false (member '(("A" . 1) ("B" . 1)) resultado))
  (check-not-false (member '(("A" . 1) ("B" . 2)) resultado))
  (check-not-false (member '(("A" . 2) ("B" . 1)) resultado))
  (check-not-false (member '(("A" . 2) ("B" . 2)) resultado)))

(test-case "cada combinacion trae un grupo por curso y los creditos suman bien"
  (for ([c (todas cat-ab (prefs 7 7 '("A" 1 2) '("B" 1 2)))])
    (check-equal? (map car (pares c)) '("A" "B"))
    (check-equal? (combinacion-creditos c) 7)))

(test-case "si todos los grupos chocan no hay combinaciones"
  (define cat (list (mk-curso "A" 3 (list (mk-grupo 1 '(("B" . 1)))))
                    (mk-curso "B" 4 (list (mk-grupo 1 '(("A" . 1)))))))
  ;; con A y B juntos (7 creditos) no hay nada; por separado no alcanzan el rango
  (check-equal? (todas cat (prefs 7 7 '("A" 1) '("B" 1))) '()))

(test-case "grupos pedidos que no existen se ignoran"
  ;; B solo tiene grupos 1 y 2: pedir el 9 no inventa nada
  (define resultado (todas cat-ab (prefs 7 7 '("A" 2) '("B" 2 9))))
  (check-equal? (map pares resultado) '((("A" . 2) ("B" . 2)))))

;; ------------------------------------------------------------------
;; Filtro por creditos
;; ------------------------------------------------------------------
(test-case "el rango de creditos incluye los extremos"
  (define p (prefs 7 7 '("A" 1 2) '("B" 1 2)))
  (check-equal? (length (todas cat-ab p)) 3))

(test-case "rango que no se puede alcanzar con ningun subconjunto deja cero combinaciones"
  (check-equal? (todas cat-ab (prefs 8 20 '("A" 1 2) '("B" 1 2))) '())
  (check-equal? (todas cat-ab (prefs 0 2 '("A" 1 2) '("B" 1 2))) '()))

;; ------------------------------------------------------------------
;; Subconjuntos de cursos (interpretacion B)
;; ------------------------------------------------------------------
(test-case "si todos los cursos juntos se pasan del rango, se usan subconjuntos"
  ;; A (3) + B (4) = 7 se pasa de 4, pero A sola (3)... no llega al minimo y B sola (4) si
  (define solo-b (map pares (todas cat-ab (prefs 4 4 '("A" 1 2) '("B" 1 2)))))
  (check-equal? solo-b '((("B" . 1)) (("B" . 2))))
  ;; rango 3-4: A sola o B sola, nunca juntos
  (define sueltos (map pares (todas cat-ab (prefs 3 4 '("A" 1 2) '("B" 1 2)))))
  (check-equal? (length sueltos) 4)
  (check-true (andmap (lambda (c) (= (length c) 1)) sueltos)))

(test-case "rango amplio: se generan los subconjuntos y no hay combinaciones repetidas"
  ;; A:2 + B:2 + A y B juntos:3 = 7
  (define resultado (map pares (todas cat-ab (prefs 0 100 '("A" 1 2) '("B" 1 2)))))
  (check-equal? (length resultado) 7)
  (check-equal? (length (remove-duplicates resultado)) 7))

(test-case "subconjuntos: poda por maximo de creditos y siempre incluye el vacio"
  (define (op codigo cr) (opcion (mk-curso codigo cr (list (mk-grupo 1 '()))) (list (mk-grupo 1 '()))))
  (define ops (list (op "X" 1) (op "Y" 2) (op "Z" 3)))
  (define subs (stream->list (subconjuntos ops 3)))
  ;; con maximo 3: {}, {X}, {Y}, {Z}, {X,Y}
  (check-equal? (length subs) 5)
  (check-true (andmap (lambda (s) (<= (creditos-de-opciones s) 3)) subs))
  (check-equal? (length (stream->list (subconjuntos ops 100))) 8)   ; 2^3
  (check-equal? (stream->list (subconjuntos '() 5)) '(())))

;; ------------------------------------------------------------------
;; Casos limite
;; ------------------------------------------------------------------
(test-case "sin cursos de interes NO se genera una combinacion vacia de 0 creditos"
  ;; con minimo 0 una combinacion vacia pasaria el filtro de creditos
  (check-equal? (todas cat-ab (prefs 0 100)) '())
  ;; lo mismo si hay cursos de interes: el subconjunto vacio nunca cuenta
  (check-true (andmap (lambda (c) (pair? (combinacion-selecciones c)))
                      (todas cat-ab (prefs 0 100 '("A" 1 2) '("B" 1 2))))))

(test-case "curso que no existe en el catalogo se omite y se reporta"
  (define p (prefs 0 100 '("A" 1 2) '("ZZ9999" 1)))
  (check-equal? (cursos-omitidos cat-ab p) '("ZZ9999"))
  (check-equal? (length (todas cat-ab p)) 2))

(test-case "curso que el estudiante no puede matricular se omite"
  (define a-bloqueado
    (curso "A" "Curso A" 3 '() '() '("Computadores") #f #f (list (mk-grupo 1 '()))))
  (check-equal? (todas (list a-bloqueado) (prefs 0 100 '("A" 1))) '())
  (check-equal? (cursos-omitidos (list a-bloqueado) (prefs 0 100 '("A" 1))) '("A")))

(test-case "correquisito no cubierto descarta la combinacion"
  (define d (mk-curso "D" 2 (list (mk-grupo 1 '()))))
  (define c (mk-curso "C" 3 (list (mk-grupo 1 '())) #:coreq '("D")))
  ;; C sola, sin D en el horario ni aprobado -> no sirve
  (check-equal? (todas (list c d) (prefs 0 100 '("C" 1))) '())
  ;; C junto con D -> sirve (rango 5-5: solo C+D suma 5)
  (check-equal? (length (todas (list c d) (prefs 5 5 '("C" 1) '("D" 1)))) 1)
  ;; con rango amplio: {C,D} y {D} sirven; {C} sola no
  (define todos (map pares (todas (list c d) (prefs 0 100 '("C" 1) '("D" 1)))))
  (check-equal? (length todos) 2)
  (check-false (member '(("C" . 1)) todos)))

(test-case "correquisito ya aprobado cuenta como cubierto"
  (define d-aprobado (mk-curso "D" 2 (list (mk-grupo 1 '())) #:aprobado #t))
  (define c (mk-curso "C" 3 (list (mk-grupo 1 '())) #:coreq '("D")))
  (check-equal? (length (todas (list c d-aprobado) (prefs 0 100 '("C" 1)))) 1))

;; ------------------------------------------------------------------
;; Pereza del stream
;; ------------------------------------------------------------------
(test-case "el stream es perezoso: pedir una combinacion no genera las 3^12 = 531441"
  (define codigos (for/list ([i 12]) (~a "C" i)))
  (define cat (for/list ([k codigos])
                (mk-curso k 1 (list (mk-grupo 1 '()) (mk-grupo 2 '()) (mk-grupo 3 '())))))
  (define p (preferencias 0 100 (for/list ([k codigos]) (interes k '(1 2 3)))))
  (define inicio (current-inexact-milliseconds))
  (define primera (stream-first (construir-combinaciones cat p)))
  (define ms (- (current-inexact-milliseconds) inicio))
  (check-equal? (combinacion-creditos primera) 12)
  (check-true (< ms 2000) (format "tardo ~a ms, se esta generando todo?" ms)))

;; ------------------------------------------------------------------
;; Catalogo real (el que exporta C)
;; ------------------------------------------------------------------
(define catalogo-real (leer-catalogo ruta-catalogo-real))

(test-case "real: CE1103, CE1105 y MA1103 dan 13 combinaciones sin choque de 11 creditos"
  ;; 13 verificado con fuerza bruta independiente sobre el JSON de C
  (define p (prefs 10 14 '("CE1103" 1 2 3) '("CE1105" 1 2) '("MA1103" 1 2 3)))
  (define resultado (todas catalogo-real p))
  (check-equal? (length resultado) 13)
  (for ([c resultado])
    (check-equal? (combinacion-creditos c) 11)
    (check-true (sin-choques? (combinacion-selecciones c)))))

(test-case "real: FI1101 + FI1201 (correquisitos entre si) dan 3 combinaciones"
  (define p (prefs 4 4 '("FI1101" 1 2 3) '("FI1201" 1 2)))
  (check-equal? (length (todas catalogo-real p)) 3))

(test-case "real: QU1102 + QU1106 (correquisitos entre si) dan 2 combinaciones"
  (define p (prefs 4 4 '("QU1106" 1 2) '("QU1102" 1 2)))
  (check-equal? (length (todas catalogo-real p)) 2))

(test-case "real: el archivo de preferencias de ejemplo da 69 combinaciones"
  ;; 69 verificado con fuerza bruta independiente (subconjuntos de los 7 cursos,
  ;; creditos 10-14, correquisitos cubiertos, sin choques)
  (define p (leer-preferencias ruta-preferencias-ejemplo))
  (define resultado (todas catalogo-real p))
  (check-equal? (length resultado) 69)
  (for ([c resultado])
    (check-true (<= 10 (combinacion-creditos c) 14))
    (check-true (sin-choques? (combinacion-selecciones c)))
    ;; ningun curso se repite dentro de una combinacion
    (define codigos (map car (pares c)))
    (check-equal? (length codigos) (length (remove-duplicates codigos))))
  ;; QU1102 y QU1106 son correquisitos: aparecen juntos o ninguno
  (for ([c resultado])
    (define codigos (map car (pares c)))
    (check-equal? (and (member "QU1102" codigos) #t) (and (member "QU1106" codigos) #t))))

;; ------------------------------------------------------------------
;; ejecutar: de archivo a archivo
;; ------------------------------------------------------------------
(test-case "ejecutar escribe un JSON valido con los campos exigidos"
  (define prefs-json
    (archivo-temporal
     "{\"creditos_minimos\": 10, \"creditos_maximos\": 14, \"cursos_interes\": [
        {\"curso\": \"CE1103\", \"grupos\": [1,2,3]},
        {\"curso\": \"CE1105\", \"grupos\": [1,2]},
        {\"curso\": \"MA1103\", \"grupos\": [1,2,3]}]}"))
  (define salida (make-temporary-file "salida-~a.json"))
  (define total (ejecutar ruta-catalogo-real prefs-json salida))
  (check-equal? total 13)

  (define json (call-with-input-file salida read-json))
  (check-equal? (hash-ref json 'creditos_minimos) 10)
  (check-equal? (hash-ref json 'creditos_maximos) 14)
  (check-equal? (hash-ref json 'total_combinaciones) 13)
  (define combos (hash-ref json 'combinaciones))
  (check-equal? (length combos) 13)
  ;; ids consecutivos 1..13
  (check-equal? (map (lambda (c) (hash-ref c 'id)) combos) (range 1 14))

  (for ([c combos])
    (define sels (hash-ref c 'selecciones))
    ;; un par curso-grupo por cada curso de interes
    (check-equal? (sort (map (lambda (s) (hash-ref s 'curso)) sels) string<?)
                  '("CE1103" "CE1105" "MA1103"))
    ;; creditos totales = suma de los creditos de cada curso, dentro del rango
    (check-equal? (hash-ref c 'creditos_totales)
                  (apply + (map (lambda (s) (hash-ref s 'creditos)) sels)))
    (check-true (<= 10 (hash-ref c 'creditos_totales) 14))
    ;; cada seleccion trae grupo y carrera(s) de origen
    (for ([s sels])
      (check-true (exact-integer? (hash-ref s 'id_grupo)))
      (check-true (string? (hash-ref s 'numero_grupo)))
      (check-true (pair? (hash-ref s 'carreras)))
      (check-true (andmap string? (hash-ref s 'carreras)))))

  (delete-file prefs-json)
  (delete-file salida))

(test-case "ejecutar con 0 combinaciones deja un JSON valido con lista vacia"
  (define prefs-json
    (archivo-temporal
     "{\"creditos_minimos\": 30, \"creditos_maximos\": 40, \"cursos_interes\": [
        {\"curso\": \"CE1103\", \"grupos\": [1,2,3]}]}"))
  (define salida (make-temporary-file "salida-~a.json"))
  (define total (ejecutar ruta-catalogo-real prefs-json salida))
  (check-equal? total 0)
  (define json (call-with-input-file salida read-json))
  (check-equal? (hash-ref json 'combinaciones) '())
  (check-equal? (hash-ref json 'total_combinaciones) 0)
  (delete-file prefs-json)
  (delete-file salida))

(test-case "ejecutar rechaza un rango de creditos invertido"
  (define prefs-json
    (archivo-temporal
     "{\"creditos_minimos\": 20, \"creditos_maximos\": 10, \"cursos_interes\": []}"))
  (define salida (make-temporary-file "salida-~a.json"))
  (check-exn exn:fail? (lambda () (ejecutar ruta-catalogo-real prefs-json salida)))
  (delete-file prefs-json)
  (delete-file salida))

(test-case "ejecutar con un catalogo que no existe falla con error"
  (define salida (make-temporary-file "salida-~a.json"))
  (check-exn exn:fail? (lambda () (ejecutar "no-existe.json" ruta-preferencias-ejemplo salida)))
  (delete-file salida))