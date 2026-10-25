#lang typed/racket
(require "../src/modelos.rkt" 
         "../src/combinaciones.rkt")
(require/typed "../src/streams.rkt" 
  [tomar (All (A) (-> Natural (Sequenceof A) (Listof A)))])

;Prueba rapida de racket

;; Mate g1 choca con Prog g1 (y viceversa)
(define g-mate-1 (grupo 1 "001" "Regular" '() '("Martes - 7:30:9:20") (list (choque "CE1101" 1))))
(define g-mate-2 (grupo 2 "002" "Regular" '() '("Lunes - 13:0:14:50") '()))
(define g-prog-1 (grupo 1 "001" "Regular" '() '("Martes - 7:30:9:20") (list (choque "MA1403" 1))))
(define g-prog-2 (grupo 2 "002" "Regular" '() '("Jueves - 7:30:9:20") '()))

(define mate (curso "MA1403" "Mate discreta" 4 '() '() '() #f #t (list g-mate-1 g-mate-2)))
(define prog (curso "CE1101" "Intro programación" 3 '() '() '() #f #t (list g-prog-1 g-prog-2)))

(define listas
  (opciones->listas (list (opcion mate (list g-mate-1 g-mate-2))
                          (opcion prog (list g-prog-1 g-prog-2)))))

(define combos (generar-combinaciones listas '()))

(: mostrar (-> (Listof seleccion) (Listof String)))
(define (mostrar sels)
  (map (lambda ([s : seleccion])
         (string-append (curso-codigo (seleccion-curso s)) "-g" (number->string (grupo-id (seleccion-grupo s)))))
       sels))


(displayln "Primera combinacion:")
(displayln (map mostrar (tomar 1 combos)))
(displayln "Todas (esperado 3, sin MA1403-g1 + CE1101-g1):")
(displayln (map mostrar (tomar 10 combos)))