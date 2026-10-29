#lang racket
(require racket/stream)

;; Este archivo se encarga de generar los streams
;; utilizados para calcular una combinacion de horario a la vez cuando esta se requiera

(provide append-perezoso append-map-perezoso)

;; Concatena dos streams de forma perezosa
;; Recibe un primer stream y calcula el segundo solo cuando el primero se acaba.
(define (append-perezoso primero resto-thunk)
   ; Si el primer stream está vacío, ejecutamos el thunk
  (cond ((stream-empty? primero)
         (resto-thunk))
    ; De lo contrario construimos el stream combinando la cabeza y el resto
    (else(stream-cons (stream-first primero)
          (append-perezoso (stream-rest primero) resto-thunk)))))

;; Aplica una función generadora de streams a una lista y concatena los resultados de forma perezosa
(define (append-map-perezoso f lista)
   ; Si la lista de elementos está vacía, retornamos un stream vacío
  (cond((null? lista)
        empty-stream)
    ; De lo contrario, aplicamos la función al primer elemento y concatenamos perezosamente el resultado con el resto
    (else (append-perezoso (f (car lista))
          (lambda () (append-map-perezoso f (cdr lista)))))))
