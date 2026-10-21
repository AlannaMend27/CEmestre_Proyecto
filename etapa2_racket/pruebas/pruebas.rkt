#lang typed/racket
(require "../src/modelos.rkt" "../src/lectura.rkt")
(define catalogo (leer-catalogo "../ArchivosEntrada/catalogo_salida.json"))
(define prefs (leer-preferencias "../ArchivosEntrada/preferencias_ejemplo.json"))
(display "cursos leidos: ") (displayln (length catalogo))
(display "intereses: ") (displayln (length (preferencias-intereses prefs)))
(displayln (curso-codigo (car catalogo)))