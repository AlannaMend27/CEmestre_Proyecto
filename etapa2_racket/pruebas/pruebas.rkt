#lang typed/racket

;NOTA ESTO ES PARA PROBAR MIENTRAS

(require "../src/modelos.rkt"
         "../src/lectura.rkt"
         "../src/filtros.rkt")

;; Parte A: de los cursos que pidio el estudiante, cuales son validos y cuantos grupos se prueban
(define catalogo (leer-catalogo "../ArchivosEntrada/catalogo_salida.json"))
(define prefs (leer-preferencias "../ArchivosEntrada/preferencias_ejemplo.json"))
(define opciones (armar-opciones catalogo (preferencias-intereses prefs)))

(displayln "Parte A: cursos de interes validos")
(for ([opcion-actual opciones])
  (display (curso-codigo (opcion-curso opcion-actual)))
  (display " -> grupos a probar: ")
  (displayln (length (opcion-grupos opcion-actual))))

;; Parte B: la MISMA funcion polimorfica filtrando tres tipos distintos de elementos
(displayln "Parte B: filtrar-con-todos con enteros, textos y cursos")
(displayln (for/list : (Listof Integer)
             ([elemento (filtrar-con-todos
                         (list even? (lambda ([numero : Integer]) (> numero 2)))
                         '(1 2 3 4 5 6))])
             elemento))
(displayln (for/list : (Listof String)
             ([elemento (filtrar-con-todos
                         (list (lambda ([texto : String]) (> (string-length texto) 0)))
                         '("a" "" "b"))])
             elemento))
(displayln (for/list : (Listof String)
             ([elemento (filtrar-con-todos
                         (list (lambda ([curso-actual : curso]) (>= (curso-creditos curso-actual) 4)))
                         catalogo)])
             (curso-codigo elemento)))

;; Parte C: criterio de creditos con combinaciones de mentira (vacias, solo con creditos)
(displayln "Parte C: creditos-en-rango? 10 a 14")
(define en-rango (creditos-en-rango? 10 14))
(displayln (list (en-rango (combinacion '() 9))
                 (en-rango (combinacion '() 12))
                 (en-rango (combinacion '() 15))))

;; Parte D: correquisitos, con el catalogo de Computadores
(displayln "Parte D: correquisitos-cubiertos?")
(define catalogo-computadores (leer-catalogo "../ArchivosEntrada/catalogo_salida.json"))

(: curso-por-codigo (-> String (Listof curso) curso))
(define (curso-por-codigo codigo lista)
  (cond ((null? lista) (error "no existe el curso" codigo))
        ((equal? (curso-codigo (car lista)) codigo) (car lista))
        (else (curso-por-codigo codigo (cdr lista)))))

;; Un par curso-grupo de mentira: el curso con su primer grupo
(: par (-> String seleccion))
(define (par codigo)
  (define curso-actual (curso-por-codigo codigo catalogo-computadores))
  (seleccion curso-actual (car (curso-grupos curso-actual))))

(define cubiertos? (correquisitos-cubiertos? catalogo-computadores))
(displayln (list (cubiertos? (combinacion (list (par "FI1201")) 1))                    ; sin FI1101
                 (cubiertos? (combinacion (list (par "FI1201") (par "FI1101")) 4))      ; con FI1101
                 (cubiertos? (combinacion (list (par "QU1102")) 1))                    ; sin QU1106
                 (cubiertos? (combinacion (list (par "QU1102") (par "QU1106")) 4))     ; los dos
                 (cubiertos? (combinacion (list (par "CE1103")) 4))))                  ; sin correquisitos