#lang typed/racket

;; lectura.rkt
;; este archivo convierte los JSON en los structs 

(require typed/json "modelos.rkt")

(provide leer-catalogo leer-preferencias) ; esto hace publicas esas 2 funciones

(: como-objeto (-> JSExpr (HashTable Symbol JSExpr)))

(define (como-objeto valor)
  (if (hash? valor) valor(error 'lectura "se esperaba un objeto JSON")))

(: campo (-> JSExpr Symbol JSExpr))
(define (campo objeto nombre)
  (hash-ref (como-objeto objeto)
            nombre
            (lambda () (error 'lectura "falta el campo ~a" nombre))))

(: como-texto (-> JSExpr String))
(define (como-texto valor)
  (if (string? valor)
      valor
      (error 'lectura "se esperaba texto")))

(: como-entero (-> JSExpr Integer))
(define (como-entero valor)
  (if (exact-integer? valor)
      valor
      (error 'lectura "se esperaba un numero entero")))

(: como-booleano (-> JSExpr Boolean))
(define (como-booleano valor)
  (if (boolean? valor)
      valor
      (error 'lectura "se esperaba true o false")))

;; Funcion polimorfica: aplica "convertir" a cada elemento de un arreglo JSON
;; Sirve igual para listas de textos, de enteros, de grupos, etc.
(: como-lista (All (A) (-> (-> JSExpr A) JSExpr (Listof A))))
(define (como-lista convertir valor)
  (if (list? valor)
      (map convertir valor)
      (error 'lectura "se esperaba un arreglo JSON")))

;; ---------- Del JSON de C a structs ----------

(: leer-choque (-> JSExpr choque))
(define (leer-choque json)
  (choque (como-texto (campo json 'curso))
          (como-entero (campo json 'id_grupo))))

(: leer-grupo (-> JSExpr grupo))
(define (leer-grupo json)
  (grupo (como-entero (campo json 'id_grupo))
         (como-texto (campo json 'numero_grupo))
         (como-texto (campo json 'tipo_grupo))
         (como-lista como-texto (campo json 'profesores))
         (como-lista como-texto (campo json 'bloques_horario))
         (como-lista leer-choque (campo json 'choca_con))))

(: leer-curso (-> JSExpr curso))
(define (leer-curso json)
  (curso (como-texto (campo json 'codigo))
         (como-texto (campo json 'nombre))
         (como-entero (campo json 'creditos))
         (como-lista como-texto (campo json 'requisitos))
         (como-lista como-texto (campo json 'correquisitos))
         (como-lista como-texto (campo json 'carreras))
         (como-booleano (campo json 'ya_aprobado))
         (como-booleano (campo json 'estudiante_puede_matricular))
         (como-lista leer-grupo (campo json 'grupos))))

;; ---------- Del JSON de preferencias a structs ----------

(: leer-interes (-> JSExpr interes))
(define (leer-interes json)
  (interes (como-texto (campo json 'curso))
           (como-lista como-entero (campo json 'grupos))))

;; ---------- Lectura de archivos ----------

(: leer-json-archivo (-> Path-String JSExpr))
(define (leer-json-archivo ruta)
  (define contenido (call-with-input-file ruta read-json))
  (if (eof-object? contenido)
      (error 'lectura "el archivo ~a esta vacio" ruta)
      contenido))

(: leer-catalogo (-> Path-String (Listof curso)))
(define (leer-catalogo ruta)
  (como-lista leer-curso (leer-json-archivo ruta)))

(: leer-preferencias (-> Path-String preferencias))
(define (leer-preferencias ruta)
  (define json (leer-json-archivo ruta))
  (preferencias (como-entero (campo json 'creditos_minimos))
                (como-entero (campo json 'creditos_maximos))
                (como-lista leer-interes (campo json 'cursos_interes))))