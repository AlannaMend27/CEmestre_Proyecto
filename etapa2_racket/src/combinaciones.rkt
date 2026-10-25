#lang typed/racket
(require "modelos.rkt")
(require "choques.rkt")

(require/typed "streams.rkt"
  [append-map-perezoso (All (A B) (-> (-> A (Sequenceof B)) (Listof A) (Sequenceof B)))]
  [tomar (All (A) (-> Natural (Sequenceof A) (Listof A)))])

;; Este archivo genera de forma perezosa (con streams) las combinaciones de horario, eligiendo un grupo por curso y descartando apenas se detecta un choque

(provide selecciones-de-opcion opciones->listas generar-combinaciones)

;; Manejo inicial de las opciones

;; Para cada grupo de la opcion, arma una seleccion con el mismo curso y grupo
(: selecciones-de-opcion (-> opcion (Listof seleccion)))
(define (selecciones-de-opcion op)
  ;recorremos los grupos de la opcion para contruir un par de grupo y curso (seleccion)
  (map (lambda ([g : grupo]) (seleccion (opcion-curso op) g))
       (opcion-grupos op)))

;; Generamos pares de grupo y curso (seleccion) para cada curso en la lista de opciones a matricular
(: opciones->listas (-> (Listof opcion) (Listof (Listof seleccion))))
(define (opciones->listas opciones)
  ; generar las selecciones por cada opcion de grupo
  (map selecciones-de-opcion opciones))

;; Manejo de las combinaciones uan vez procesadas las opciones y generadas las selecciones

;; Cuando ya se escogio un grupo de cada curso, la combinacion esta lista.
(: combinacion-final (-> (Listof seleccion) (Sequenceof (Listof seleccion))))
(define (combinacion-final elegidos)
  ; Los elegidos quedaron al reves, se les da vuelta para dejarlos en orden
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
    ; Si no quedan cursos por revisar, ya tenemos una combinacion 
    ((null? cursos) (combinacion-final elegidos))
    ; Si quedan cursos, se prueba cada grupo candidato del primer curso
    (else
     (append-map-perezoso
      ; establecemos un elemento llamado candidata de tipo seleccion
      (lambda ([candidata : seleccion])
        ; se prueba el primer elemento de cursos
        (probar-candidata candidata (cdr cursos) elegidos))
      (car cursos)))))

