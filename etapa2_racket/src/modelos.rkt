#lang typed/racket

;; modelos.rkt
;; aqui estan los structs, el archivo de C se moldea a estos structs

(provide (all-defined-out)) ; esta linea hace que todo lo de este archivo se pueda usar en otros

;; Un choque de horario entre grupos
(struct choque ([codigo-curso : String]
                [id-grupo : Integer])
  #:transparent)

;; Grupo de un curso
(struct grupo ([id : Integer] ;esto es para diferenciar los grupos (xq hay grupo 1 de discreta y de cdi digamos)
               [numero : String]   
               [tipo : String]
               [profesores : (Listof String)]
               [bloques : (Listof String)]
               [choques : (Listof choque)])
  #:transparent)

;; Un curso con la misma estructura de lo que tiro C
(struct curso ([codigo : String]
               [nombre : String]
               [creditos : Integer]
               [requisitos : (Listof String)]
               [correquisitos : (Listof String)]
               [carreras : (Listof String)]
               [ya-aprobado : Boolean]
               [puede-matricular : Boolean]
               [grupos : (Listof grupo)])
  #:transparent)

;; Estos son los grupos que quiere matriuclar el estudiante
(struct interes ([codigo-curso : String]
                 [ids-grupos : (Listof Integer)])
  #:transparent)

;; Archivo de preferencias completo
(struct preferencias ([creditos-minimos : Integer]
                      [creditos-maximos : Integer]
                      [intereses : (Listof interes)])
  #:transparent)

;; Un curso de interes ya validado junto con los grupos que se pueden probar
(struct opcion ([curso : curso]
                [grupos : (Listof grupo)])
  #:transparent)

;; Un par curso y grupo escogido dentro de una combinacion
(struct seleccion ([curso : curso]
                   [grupo : grupo])
  #:transparent)

;; Una combinacion de horarios varios pares curso grupo y la suma de sus creditos
(struct combinacion ([selecciones : (Listof seleccion)]
                     [creditos : Integer])
  #:transparent)