#lang typed/racket
(require "modelos.rkt")

; En este archivo se filtran que horarios "sirven"

(require/typed racket/stream [stream-filter (All (A) (-> (-> A Boolean) (Sequenceof A) (Sequenceof A)))])

(provide filtrar-con-todos creditos-en-rango? correquisitos-cubiertos? armar-opciones)

;; Esta funcion sirve para filtrar combinaciones de cualquier tipo (cursos, numeros y textos)
;; Recibe una lista con combinaciones de horario y una lista de criterios y filtra las combinaciones posibles y correctas bajo esos criterios (reglas)
(: filtrar-con-todos (All (A) (-> (Listof (-> A Boolean)) (Sequenceof A) (Sequenceof A))))
(define (filtrar-con-todos criterios secuencia)
  (if (null? criterios) secuencia
      (filtrar-con-todos (cdr criterios) (stream-filter (car criterios) secuencia))))


;;Devuelve una funcion que dice si una combinacion tiene creditos entre el rango (regla)
(: creditos-en-rango? (-> Integer Integer (-> combinacion Boolean)))
(define (creditos-en-rango? minimo maximo)
  (lambda ([elemento : combinacion])
    (and (>= (combinacion-creditos elemento) minimo)
         (<= (combinacion-creditos elemento) maximo))))

; Recibe el catalogo y devuelve una funcion que verifica si un correquisito esta cubierto
(: correquisitos-cubiertos? (-> (Listof curso) (-> combinacion Boolean)))
(define (correquisitos-cubiertos? catalogo)
  (lambda ([elemento : combinacion]) (selecciones-con-correquisitos-listos? (combinacion-selecciones elemento)
     (map (lambda ([par : seleccion]) (curso-codigo (seleccion-curso par))) (combinacion-selecciones elemento)) catalogo)))


; Devuelve true si todos los cursos que de una combinacion de horarios posible
; tienen todos los correquisitos false si no
(: selecciones-con-correquisitos-listos? (-> (Listof seleccion) (Listof String) (Listof curso) Boolean))
(define (selecciones-con-correquisitos-listos? selecciones codigos-elegidos catalogo)

  (cond ((null? selecciones) #t)
        ;Si los correquisitos del primer curso ya estan, se revisan los otros
        ((correquisitos-listos? (curso-correquisitos (seleccion-curso (car selecciones)))codigos-elegidos catalogo)
         (selecciones-con-correquisitos-listos? (cdr selecciones) codigos-elegidos catalogo))
 
 (else #f)))

;Hace lo mismo que la funcion anterior, nada mas que con un curso solicitado
(: correquisitos-listos? (-> (Listof String) (Listof String) (Listof curso) Boolean))
(define (correquisitos-listos? correquisitos codigos-elegidos catalogo)
  (cond ((null? correquisitos) #t)
         ; Si el primer correquisito esta aprobado, se revisan los demas
        ((or (aprobado? (car correquisitos) catalogo)
             (texto-en-lista? (car correquisitos) codigos-elegidos))
         (correquisitos-listos? (cdr correquisitos) codigos-elegidos catalogo))
  (else #f)))

; Arma las combinaciones validas con el archivo y el catalogo
(: armar-opciones (-> (Listof curso) (Listof interes) (Listof opcion)))
(define (armar-opciones catalogo intereses)
  (cond ((null? intereses) '())
        (else
         ;Busca el primer curso de interes en el catalogo
         (define curso-encontrado (buscar-curso (interes-codigo-curso (car intereses)) catalogo))
         (define resto (armar-opciones catalogo (cdr intereses)))
         ; Si no se encuentra se salta
         (cond ((not curso-encontrado) resto) ((not (curso-puede-matricular curso-encontrado)) resto)
         ; Si se encuentra deja solo los grupos pedidos que existen en el curso
         (else (define candidatos (grupos-con-ids (curso-grupos curso-encontrado) (interes-ids-grupos (car intereses))))
                ; Si ningun grupo pedido exite se salta
                (if (null? candidatos) resto
                    ;si existe se pone al frente
                    (cons (opcion curso-encontrado candidatos) resto)))))))



; Retorna si un texto aparece en una lista
(: texto-en-lista? (-> String (Listof String) Boolean))
(define (texto-en-lista? texto lista)
  (cond ((null? lista) #f)
        ; Si el primer elemento es igual al texto buscado es true
        ((equal? texto (car lista)) #t)
        ;si no se busca en el resto de la lista 
        (else (texto-en-lista? texto (cdr lista)))))

;; ¿Esta este numero en la lista de numeros?
(: entero-en-lista? (-> Integer (Listof Integer) Boolean))
(define (entero-en-lista? numero lista)
  (cond ((null? lista) #f)
        ((= numero (car lista)) #t)
        (else (entero-en-lista? numero (cdr lista)))))

; indica si un numero solicitado esta en una lista de enteros
(: buscar-curso (-> String (Listof curso) (U curso False)))
(define (buscar-curso codigo-buscado catalogo)
  (cond ((null? catalogo) #f)
        ;si el primer elemento es igual al buscado
        ((equal? (curso-codigo (car catalogo)) codigo-buscado) (car catalogo))
        ; si no es se busca en el resto de la lista
        (else (buscar-curso codigo-buscado (cdr catalogo)))))

; indica si un estudiante ya aprobo el curso
(: aprobado? (-> String (Listof curso) Boolean))
(define (aprobado? codigo catalogo)
  ;busca el curso en el catalogo
  (define curso-encontrado (buscar-curso codigo catalogo))
  (if curso-encontrado (curso-ya-aprobado curso-encontrado) #f))

;; Filtra solo los grupos que el estudiante solicita de un curso
(: grupos-con-ids (-> (Listof grupo) (Listof Integer) (Listof grupo)))
(define (grupos-con-ids grupos ids)
  ;Nos quedamos con los cursos para los que la funcion devuelve verdader
  (filter (lambda ([grupo-actual : grupo]) (entero-en-lista? (grupo-id grupo-actual) ids)) grupos))