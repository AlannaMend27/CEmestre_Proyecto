#lang typed/racket

;; lectura.rkt
;; este archivo convierte los JSON en los structs 

(require typed/json "modelos.rkt")

(provide leer-catalogo leer-preferencias)

(: como-objeto (-> JSExpr (HashTable Symbol JSExpr)))

;;; Confirma que el JSON es un objeto
(define (como-objeto valor)
  (if (hash? valor) valor (error 'lectura "se esperaba un objeto JSON")))


;Esta funcion saca el valor de una llave del JSON y si no existe da error
(: campo (-> JSExpr Symbol JSExpr))
(define (campo objeto nombre)
  (hash-ref (como-objeto objeto) nombre (lambda () (error 'lectura "falta el campo ~a" nombre))))


;Toma un valor del JSON y confirma que es un string
(: como-texto (-> JSExpr String))
(define (como-texto valor)
  (if (string? valor) valor (error 'lectura "se esperaba texto")))


;Toma un valor del JSON y confirma que es un entero
(: como-entero (-> JSExpr Integer))
(define (como-entero valor)
  (if (exact-integer? valor) valor (error 'lectura "se esperaba un numero entero")))


;Toma un valor del JSON y confirma que es un bool
(: como-booleano (-> JSExpr Boolean))
(define (como-booleano valor)
  (if (boolean? valor) valor (error 'lectura "se esperaba true o false")))


;Toma un valor del JSON y confirma que es un arreglo 
(: como-arreglo (-> JSExpr (Listof JSExpr)))
(define (como-arreglo valor)
  (if (list? valor) valor (error 'lectura "se esperaba un arreglo JSON")))


;Confirma que el campo sea un arreglo de strings y devuelve como listra de strings
(: lista-de-textos (-> JSExpr (Listof String)))
(define (lista-de-textos valor)
  (map como-texto (como-arreglo valor)))

;Confirma que el campo sea un arreglo de enteros y devuelve como lista de enteros
(: lista-de-enteros (-> JSExpr (Listof Integer)))
(define (lista-de-enteros valor)
  (map como-entero (como-arreglo valor)))

; Estas funciones pasan de JSON de a structs

; Convierte la parte del JSON {"curso", "id_grupo"} en un struct choque
(: leer-choque (-> JSExpr choque))
(define (leer-choque json)
  (choque (como-texto (campo json 'curso)) (como-entero (campo json 'id_grupo))))


; Convierte la parte JSON de grupo en un struct grupo con sus profesores, horarios y choques
(: leer-grupo (-> JSExpr grupo))
(define (leer-grupo json)
  (grupo (como-entero (campo json 'id_grupo))
         (como-texto (campo json 'numero_grupo))
         (como-texto (campo json 'tipo_grupo))
         (lista-de-textos (campo json 'profesores))
         (lista-de-textos (campo json 'bloques_horario))
         (map leer-choque (como-arreglo (campo json 'choca_con)))))

; Igual pero con curso
(: leer-curso (-> JSExpr curso))
(define (leer-curso json)
  (curso (como-texto (campo json 'codigo))
         (como-texto (campo json 'nombre))
         (como-entero (campo json 'creditos))
         (lista-de-textos (campo json 'requisitos))
         (lista-de-textos (campo json 'correquisitos))
         (lista-de-textos (campo json 'carreras))
         (como-booleano (campo json 'ya_aprobado))
         (como-booleano (campo json 'estudiante_puede_matricular))
         (map leer-grupo (como-arreglo (campo json 'grupos)))))

; Lee del JSON de preferencias del estudiante y lo pasa a structs
(: leer-interes (-> JSExpr interes))
(define (leer-interes json)
  (interes (como-texto (campo json 'curso))
           (lista-de-enteros (campo json 'grupos))))

; Estas funciones son lectura de archivos 

; Abre el archivo .json, convierte su texto en un JSExpr y lo devuelve si está vacío, da erro
(: leer-json-archivo (-> Path-String JSExpr))
(define (leer-json-archivo ruta)
  (define contenido (call-with-input-file ruta read-json))
  (if (eof-object? contenido) (error 'lectura "el archivo ~a esta vacio" ruta) contenido))

;; Lee el catálogo JSON de C y lo devuelve como lista de structs curso
(: leer-catalogo (-> Path-String (Listof curso)))
(define (leer-catalogo ruta)
  (map leer-curso (como-arreglo (leer-json-archivo ruta))))

;; ;; Lee el archivo de preferencias completo y lo devuelve como struct preferencias
(: leer-preferencias (-> Path-String preferencias))
(define (leer-preferencias ruta)
  (define json (leer-json-archivo ruta))
  (preferencias (como-entero (campo json 'creditos_minimos))
                (como-entero (campo json 'creditos_maximos))
                (map leer-interes (como-arreglo (campo json 'cursos_interes)))))