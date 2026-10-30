#lang typed/racket
(require "modelos.rkt")
(require "choques.rkt")

;; Este archivo genera de forma perezosa (con streams) las combinaciones de horario, eligiendo un grupo por curso y descartando apenas se detecta un choque

(require/typed "streams.rkt" [append-map-perezoso (All (A B) (-> (-> A (Sequenceof B)) (Listof A) (Sequenceof B)))])

(provide selecciones-de-opcion opciones->listas generar-combinaciones)

;; Manejo inicial de las opciones
;; Recibe una opcion (un curso con los grupos que el estudiante quiere) y devuelve
;; una lista con una seleccion (curso + grupo) por cada grupo de la opcion.
(: selecciones-de-opcion (-> opcion (Listof seleccion)))
(define (selecciones-de-opcion op)
  (map (lambda ([g : grupo]) (seleccion (opcion-curso op) g))
       (opcion-grupos op)))

;; Generamos pares de grupo y curso (seleccion) para cada curso en la lista de opciones a matricular
(: opciones->listas (-> (Listof opcion) (Listof (Listof seleccion))))
(define (opciones->listas opciones)
  (map selecciones-de-opcion opciones))

;; Cuando ya se escogio un grupo de cada curso, se le da vuelta a los elegidos ya que quedaaron al revés.
(: combinacion-final (-> (Listof seleccion) (Sequenceof (Listof seleccion))))
(define (combinacion-final elegidos)
  (in-list (list (reverse elegidos))))

;; Revisa si una candidata se puede agregar a lo ya elegido.
(: probar-candidata (-> seleccion (Listof (Listof seleccion)) (Listof seleccion) (Sequenceof (Listof seleccion))))
(define (probar-candidata candidata cursos-restantes elegidos)
  (cond
    ; Si choca con algo ya elegido, se descarta y no se sigue por este camino
    ((choca-con-elegidos? candidata elegidos) (in-list '()))
    ; Si no choca, se agrega a lo elegido y se sigue con los cursos que faltan
    (else (generar-combinaciones cursos-restantes (cons candidata elegidos)))))

;; Genera las combinaciones una por una (stream), un grupo por cada curso.
(: generar-combinaciones (-> (Listof (Listof seleccion)) (Listof seleccion) (Sequenceof (Listof seleccion))))
(define (generar-combinaciones cursos elegidos)
  (cond
    ((null? cursos) (combinacion-final elegidos))
    ; Si quedan cursos, se prueba cada grupo candidato del primer curso
    (else
     (append-map-perezoso
      (lambda ([candidata : seleccion])
        ; se prueba el primer elemento de cursos
        (probar-candidata candidata (cdr cursos) elegidos))
      (car cursos)))))

