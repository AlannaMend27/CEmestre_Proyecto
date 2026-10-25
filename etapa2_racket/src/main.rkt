#lang typed/racket

;; main.rkt
;; Une todas las piezas de la etapa 2:
;;   lectura -> opciones -> subconjuntos de cursos -> generacion (stream) -> filtros -> exportacion
;;
;; Interpretacion adoptada: los cursos de interes son OPCIONES. Cada combinacion usa
;; un subconjunto de ellos (con un grupo por curso elegido) cuyos creditos caen en el
;; rango pedido. Si todos los cursos de interes juntos se pasan del rango, se generan
;; combinaciones con los subconjuntos que si caben.
;;
;; Uso:
;;   racket main.rkt [catalogo.json] [preferencias.json] [salida.json]

(require "modelos.rkt"
         "lectura.rkt"
         "filtros.rkt"
         "combinaciones.rkt"
         "exportar.rkt")

(require/typed racket/stream
  [stream-map (All (A B) (-> (-> A B) (Sequenceof A) (Sequenceof B)))]
  [stream-filter (All (A) (-> (-> A Boolean) (Sequenceof A) (Sequenceof A)))]
  [stream-empty? (-> (Sequenceof Any) Boolean)]
  [stream-first (All (A) (-> (Sequenceof A) A))]
  [stream-rest (All (A) (-> (Sequenceof A) (Sequenceof A)))])

(require/typed "streams.rkt"
  [append-perezoso (All (A) (-> (Sequenceof A) (-> (Sequenceof A)) (Sequenceof A)))])

(provide sumar-creditos
         creditos-de-opciones
         subconjuntos
         selecciones->combinacion
         construir-combinaciones
         cursos-omitidos
         ejecutar)

;; Archivos por defecto si no se pasan argumentos
(define catalogo-por-defecto "catalogo_salida.json")
(define preferencias-por-defecto "preferencias_ejemplo.json")
(define salida-por-defecto "combinaciones_salida.json")

;; Suma los creditos de una lista de selecciones
(: sumar-creditos (-> (Listof seleccion) Integer))
(define (sumar-creditos selecciones)
  (cond ((null? selecciones) 0)
        (else (+ (curso-creditos (seleccion-curso (car selecciones)))
                 (sumar-creditos (cdr selecciones))))))

;; Convierte la lista de pares curso-grupo que produce el stream en un struct combinacion
(: selecciones->combinacion (-> (Listof seleccion) combinacion))
(define (selecciones->combinacion selecciones)
  (combinacion selecciones (sumar-creditos selecciones)))

;; Suma los creditos de una lista de opciones (un curso cuenta una sola vez, sin importar
;; cuantos grupos tenga)
(: creditos-de-opciones (-> (Listof opcion) Integer))
(define (creditos-de-opciones opciones)
  (cond ((null? opciones) 0)
        (else (+ (curso-creditos (opcion-curso (car opciones)))
                 (creditos-de-opciones (cdr opciones))))))

;; Igual que append-map-perezoso de streams.rkt, pero la entrada es un stream y no una lista:
;; aplica f al primer elemento y deja el resto para cuando haga falta.
(: aplanar-perezoso (All (A B) (-> (-> A (Sequenceof B)) (Sequenceof A) (Sequenceof B))))
(define (aplanar-perezoso f secuencia)
  (cond ((stream-empty? secuencia) '())
        (else (append-perezoso (f (stream-first secuencia))
                               (lambda () (aplanar-perezoso f (stream-rest secuencia)))))))

;; Genera de forma perezosa todos los subconjuntos de opciones cuyos creditos no
;; superan "disponible". Poda en cuanto un curso ya no cabe, antes de generar nada
;; (los creditos dependen del curso, no del grupo), asi que no se exploran subconjuntos
;; que de todas formas se pasarian del maximo.
(: subconjuntos (-> (Listof opcion) Integer (Sequenceof (Listof opcion))))
(define (subconjuntos opciones disponible)
  (cond
    ((null? opciones) (in-list (list '())))
    (else
     (define primera (car opciones))
     (define creditos (curso-creditos (opcion-curso primera)))
     (define sin-primera (lambda () (subconjuntos (cdr opciones) disponible)))
     (cond
       ;; Si el curso ya no cabe, solo queda el camino de no incluirlo
       ((> creditos disponible) (sin-primera))
       ;; Si cabe: primero los subconjuntos que lo incluyen, luego los que no
       (else
        (append-perezoso
         (stream-map (lambda ([resto : (Listof opcion)]) (cons primera resto))
                     (subconjuntos (cdr opciones) (- disponible creditos)))
         sin-primera))))))

;; Arma el stream de combinaciones validas:
;;  1. subconjuntos de cursos que no se pasan del maximo y alcanzan el minimo (y no vacios)
;;  2. para cada subconjunto, un grupo por curso sin choques (lo garantiza el generador)
;;  3. filtros finales: creditos en rango y correquisitos cubiertos
;; No calcula nada hasta que alguien consume el stream.
(: construir-combinaciones (-> (Listof curso) preferencias (Sequenceof combinacion)))
(define (construir-combinaciones catalogo prefs)
  (define minimo (preferencias-creditos-minimos prefs))
  (define maximo (preferencias-creditos-maximos prefs))
  (define opciones (armar-opciones catalogo (preferencias-intereses prefs)))
  ;; Subconjuntos no vacios que alcanzan el minimo de creditos.
  ;; (excluir el vacio evita una "combinacion" de 0 creditos cuando el minimo es 0)
  (define subconjuntos-validos
    (stream-filter
     (lambda ([sub : (Listof opcion)])
       (and (not (null? sub)) (>= (creditos-de-opciones sub) minimo)))
     (subconjuntos opciones maximo)))
  (filtrar-con-todos
   (list (creditos-en-rango? minimo maximo)
         (correquisitos-cubiertos? catalogo))
   (stream-map selecciones->combinacion
               (aplanar-perezoso
                (lambda ([sub : (Listof opcion)])
                  (generar-combinaciones (opciones->listas sub) '()))
                subconjuntos-validos))))

;; Codigos de cursos de interes que no se pudieron usar
;; (no existen en el catalogo, el estudiante no puede matricularlos o ningun grupo pedido existe)
(: cursos-omitidos (-> (Listof curso) preferencias (Listof String)))
(define (cursos-omitidos catalogo prefs)
  (define usados
    (map (lambda ([o : opcion]) (curso-codigo (opcion-curso o)))
         (armar-opciones catalogo (preferencias-intereses prefs))))
  (map interes-codigo-curso
       (filter (lambda ([i : interes]) (not (member (interes-codigo-curso i) usados)))
               (preferencias-intereses prefs))))

;; Corre todo el proceso y devuelve cuantas combinaciones se exportaron
(: ejecutar (-> Path-String Path-String Path-String Integer))
(define (ejecutar ruta-catalogo ruta-preferencias ruta-salida)
  (define catalogo (leer-catalogo ruta-catalogo))
  (define prefs (leer-preferencias ruta-preferencias))
  (when (> (preferencias-creditos-minimos prefs) (preferencias-creditos-maximos prefs))
    (error 'main "el minimo de creditos (~a) es mayor que el maximo (~a)"
           (preferencias-creditos-minimos prefs)
           (preferencias-creditos-maximos prefs)))
  (exportar-combinaciones ruta-salida
                          (preferencias-creditos-minimos prefs)
                          (preferencias-creditos-maximos prefs)
                          (construir-combinaciones catalogo prefs)))

;; Punto de entrada por linea de comandos
(module+ main
  (define args (vector->list (current-command-line-arguments)))
  (define-values (ruta-catalogo ruta-preferencias ruta-salida)
    (cond ((null? args)
           (values catalogo-por-defecto preferencias-por-defecto salida-por-defecto))
          ((= (length args) 3)
           (values (first args) (second args) (third args)))
          (else
           (eprintf "Uso: racket main.rkt [catalogo.json preferencias.json salida.json]\n")
           (exit 2))))
  (with-handlers ([exn:fail? (lambda ([e : exn])
                               (eprintf "Error: ~a\n" (exn-message e))
                               (exit 1))])
    (define omitidos (cursos-omitidos (leer-catalogo ruta-catalogo)
                                      (leer-preferencias ruta-preferencias)))
    (unless (null? omitidos)
      (printf "Aviso: cursos de interes omitidos (no existen, no se pueden matricular o sin grupos validos): ~a\n"
              omitidos))
    (define total (ejecutar ruta-catalogo ruta-preferencias ruta-salida))
    (printf "Listo: ~a combinaciones escritas en ~a\n" total ruta-salida)))