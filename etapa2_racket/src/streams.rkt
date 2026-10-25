#lang racket
(require racket/stream)

;; Este archivo se encarga de generar los streams
;; utilizados para calcular una combinacion de horario a la vez cuando esta se requiera

(provide append-perezoso append-map-perezoso tomar)

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

;NOTITA: De momento esta funcion se ocupa solo para el archivo PruebaCombinaciones
;La podemos dejar ya que sirve para probar que el stream es perezoso o ya quitarlo
;pq en el proyecto como tal o es algo que se necdesite

;; Toma los primeros elementos de un stream y los devuelve como lista
(define (tomar cantidad secuencia)
  ; Si ya tomamos todos los elementos pedidos o el stream está vacío, devolvemos una lista vacía
  (cond ((or (= cantidad 0) (stream-empty? secuencia))
     '())
    ; De lo contrario, tomamos el primer elemento del stream y avanzamos recursivamente
    (else (cons (stream-first secuencia)
                (tomar (- cantidad 1) (stream-rest secuencia))))))