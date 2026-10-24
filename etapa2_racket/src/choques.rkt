#lang typed/racket
(require "modelos.rkt")

;Este archivo se encarga de verifcicar si un grupo
;de un curso choca con otro grupo de otro curso

(provide selecciones-chocan? choca-con-elegidos? sin-choques?)
 
;; verificar si la lista de choques de un grupo menciona al grupo a verificar
(: choque-registrado? (-> (Listof choque) String Integer Boolean))
(define (choque-registrado? choques codigo id)
  (cond ((null? choques) #f)
        ; si hay mismo código de curso y mismo id de grupo, ahí hay un choque
        ((and (equal? (choque-codigo-curso (car choques)) codigo)
              (= (choque-id-grupo (car choques)) id))
         #t)
        ; Si no, se revisa el resto de la lista con recursion
        (else (choque-registrado? (cdr choques) codigo id))))
 
; Chocan dos selecciones curso-grupo
; C registra cada choque en ambos grupos, así que basta mirar un lado
(: selecciones-chocan? (-> seleccion seleccion Boolean))
(define (selecciones-chocan? primera segunda)
  ; verificar si la segunda seleccion esta dentro de la lista de choques de la primera seleccion
  (choque-registrado? (grupo-choques (seleccion-grupo primera)) 
                      (curso-codigo (seleccion-curso segunda)) 
                      (grupo-id (seleccion-grupo segunda)))) 
 
;; Verifica si la selección candidata choca con alguna de las ya elegidas
(: choca-con-elegidos? (-> seleccion (Listof seleccion) Boolean))
(define (choca-con-elegidos? candidata elegidas)
  (cond ((null? elegidas) #f)
        ; verificar con la funcion seleeciones-chocan?
        ((selecciones-chocan? candidata (car elegidas)) #t)
        (else (choca-con-elegidos? candidata (cdr elegidas)))))
 
;; Verifica que ninguna pareja de una lista de selecciones choque entre si
(: sin-choques? (-> (Listof seleccion) Boolean))
(define (sin-choques? selecciones)
  (cond ((null? selecciones) #t)
        ; se va verificando seleccion por seleccion con el resto de la lista de selecciones
        ((choca-con-elegidos? (car selecciones) (cdr selecciones)) #f)
        (else (sin-choques? (cdr selecciones)))))
 