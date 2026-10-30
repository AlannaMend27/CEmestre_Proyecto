#lang typed/racket

;; El archivo exportar.rkt escribe las combinaciones validas en un archivo JSON, que es el contrato con la etapa de Prolog.
;; Ejemplo del formato del archivo:
;; {
;;   "creditos_minimos": 10,
;;   "creditos_maximos": 14,
;;   "combinaciones": [
;;     {"id": 1,
;;      "creditos_totales": 13,
;;      "selecciones": [
;;        {"curso": "CE1103", "nombre": "...", "creditos": 4,
;;         "id_grupo": 1, "numero_grupo": "001",
;;         "carreras": ["Computadores"]},
;;        ...]},
;;     ...
;;   ],
;;   "total_combinaciones": 42
;; }

(require typed/json "modelos.rkt")

(provide exportar-combinaciones)

;; Convierte un par curso-grupo a un objeto JSON.
;; "id_grupo" es el mismo identificador que usa "choca_con" en el catalogo de C
;; y "numero_grupo" es el numero visible ("001").
(: seleccion->json (-> seleccion JSExpr))
(define (seleccion->json par)
  (define c (seleccion-curso par))
  (define g (seleccion-grupo par))
  (ann (hasheq 'curso (curso-codigo c)
               'nombre (curso-nombre c)
               'creditos (curso-creditos c)
               'id_grupo (grupo-id g)
               'numero_grupo (grupo-numero g)
               'carreras (curso-carreras c))
       (HashTable Symbol JSExpr)))

;; Convierte una combinacion completa a un objeto JSON.
;; "id" es un consecutivo para que las etapas siguientes puedan referirse a ella.
(: combinacion->json (-> combinacion Integer JSExpr))
(define (combinacion->json comb id)
  (ann (hasheq 'id id
               'creditos_totales (combinacion-creditos comb)
               'selecciones (map seleccion->json (combinacion-selecciones comb)))
       (HashTable Symbol JSExpr)))

;; Escribe las combinaciones una por linea, a medida que el stream las produce,
;; y devuelve cuantas escribio. El acumulador n lleva la cuenta sin mutar nada.
(: escribir-combinaciones (-> (Sequenceof combinacion) Output-Port Integer))
(define (escribir-combinaciones secuencia salida)
  (for/fold ([n : Integer 0])
            ([comb : combinacion secuencia])
    (write-string (if (= n 0) "\n" ",\n") salida)
    (write-json (combinacion->json comb (+ n 1)) salida)
    (+ n 1)))

;; Crea (o reemplaza) el archivo de salida con todas las combinaciones del stream.
;; Devuelve el total de combinaciones escritas.
(: exportar-combinaciones (-> Path-String Integer Integer (Sequenceof combinacion) Integer))
(define (exportar-combinaciones ruta minimo maximo secuencia)
  (call-with-output-file ruta #:exists 'replace
    (lambda ([salida : Output-Port])
      (write-string (format "{\"creditos_minimos\": ~a,\n\"creditos_maximos\": ~a,\n\"combinaciones\": ["
                            minimo maximo)
                    salida)
      (define total (escribir-combinaciones secuencia salida))
      (write-string (format "\n],\n\"total_combinaciones\": ~a}\n" total) salida)
      total)))