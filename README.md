# CEmestre : Constructor de Horarios · Etapa 1 

**Curso:** CE1106 Paradigmas de Programación    
**Institución:** Instituto Tecnológico de Costa Rica  
**Integrantes:** Dylan Bonilla Barquero, Alanna Mendoza Fonseca, Miguel Valdelomar Martínez

## Descripción general

CEmestre es un sistema que ayuda a un estudiante del TEC a armar su horario de matrícula. Esta primera etapa, implementada en C bajo el paradigma imperativo, recibe el catálogo de cursos de dos carreras (Ingeniería en Computadores e Ingeniería en Electrónica) y detecta choques de horario entre todos los grupos disponibles, válida qué cursos puede matricular el estudiante según su historial académico y exporta el resultado en un archivo JSON, el cual muestra la información recopilada para cada curso del plan de estudios (primeros 4 semestres) del estudiante.

---

## Uso del ejecutable

El programa recibe exactamente dos argumentos por línea de comandos, en este orden: primero la ruta al catálogo de cursos de la carrera en formato JSON, y luego la ruta al archivo de historial del estudiante en formato TXT. Si alguno de los dos argumentos falta o la ruta es incorrecta, el programa reporta el error y termina sin generar salida. Una vez que ambos archivos se cargan correctamente, el procesamiento completo ocurre de forma automática y el archivo de salida se escribe en `etapa1_c/output/catalogo_salida.json` sin necesidad de parámetros adicionales.

```bash
./build/catalogo_run <ruta_catalogo.json> <ruta_historial.txt>
```

**Ejemplo:**
```bash
./build/catalogo_run etapa1_c/ArchivosEntrada/info_cursos/ingComputadores/catalogo_cursos_computadores.json etapa1_c/ArchivosEntrada/historial_computadores.txt
```

El archivo de salida se genera en `etapa1_c/output/catalogo_salida.json`.

---

## Arquitectura del proyecto

### Estructura de carpetas

```
CEmestre_Proyecto/
├── etapa1_c/                        # Módulo principal en C
│   ├── src/                         # Archivos de implementación (.c)
│   │   ├── main.c
│   │   ├── catalogo.c
│   │   ├── horario.c
│   │   ├── historial.c
│   │   ├── consulta.c
│   │   ├── exportar.c
│   │   └── manejoArreglos.c
│   ├── include/                     # Headers (.h)
│   │   ├── modelos.h                # Definición de structs principales
│   │   ├── constantes.h             # Constantes de tamaño
│   │   ├── catalogo.h
│   │   ├── horario.h
│   │   ├── historial.h
│   │   ├── consulta.h
│   │   ├── exportar.h
│   │   ├── manejoArreglos.h
│   │   └── listaEnlazada.h
│   ├── structures/
│   │   └── listaEnlazada.c          # Implementación de la lista enlazada
│   ├── lib/
│   │   ├── cJSON.c                  # Librería externa para manejo de JSON
│   │   └── cJSON.h
│   ├── ArchivosEntrada/             # Archivos de entrada (historial estudiante y planes de estudio)
│   │   ├── historial_computadores.txt
│   │   ├── historial_electronica.txt
│   │   └── info_cursos/
│   │       ├── ingComputadores/catalogo_cursos_computadores.json
│   │       └── ingElectronica/catalogo_cursos_electronica.json
│   └── output/
│       └── catalogo_salida.json     # Archivo generado por el programa
│  
└── Obtencion_Datos/                 # Scripts Python de recolección de datos
    ├── Computadores/
    │   ├── Obtener_Grupos_Escuelas.py   # Scraper del TEC-Digital
    │   ├── CombinarDatos.py             # Compara los datos crudos de los cursos con el plan de estudios para obtener lo necesario
    │   ├── horarios_raw.json            # Datos crudos de los cursos del TEC-Digital
    │   └── plan_estudios.json           # Plan de estudios extraído para cada carrera
    │   └── catalogo_cursos_computadores.json  # Cursos y sus horarios disponibles para el plan de estudios
    └── Electrónica/
        └── (misma estructura)
```

---

### Descripción de módulos

#### `main.c` — Punto de entrada y coordinador principal

Recibe dos argumentos por línea de comandos: la ruta del catálogo JSON y la ruta del historial del estudiante. Valida que ambos argumentos existan antes de proceder. Luego llama a cada módulo de manera que se sigue el siguiente orden: carga del historial → carga del catálogo → detección de choques → consulta de disponibilidad → exportación a archivo JSON de salida. Al finalizar, libera toda la memoria dinámica reservada durante la ejecución.

**Flujo de ejecución:**

 
```mermaid
flowchart TD
    A([ Argumentos CLI]) --> B
 
    B[" cargarHistorial()\nLee historial.txt"]
    B --> C
 
    C[" cargarCatalogo()\nParsea catalogo.json con cJSON"]
    C --> D
 
    D[" detectarChoques()\nCompara todos los pares de cursos y grupos"]
    D --> E
 
    E[" obtenerCursosDisponibles()\nValida requisitos contra historial"]
    E --> F
 
    F[" exportarCatalogo()\nEscribe catalogo_salida.json"]
    F --> G
 
    G(Liberar memoria)
 
    style A fill:#1B3F72,color:#fff,stroke:none
    style G fill:#1A2F4E,color:#cdd8e8,stroke:none
    style B fill:#1E3A5F,stroke:#4A7FB5,color:#A8C4E0
    style C fill:#1A3550,stroke:#3A6D9E,color:#A8C4E0
    style D fill:#162E47,stroke:#2E5F8A,color:#8AB4D4
    style E fill:#112640,stroke:#245278,color:#7DAAC8
    style F fill:#0D1F35,stroke:#1C4568,color:#6D9EBE
    linkStyle default stroke:#F472B6,stroke-width:2px
```

---

#### `catalogo.c / catalogo.h` — Carga del catálogo

Lee el archivo JSON de entrada (archivo con todos los cursos y grupos que tienen los primeros cuatro semestres del plan de estudios del estudiante) completo en un buffer de memoria, lo parsea usando la librería `cJSON`, y construye un arreglo dinámico de structs `Curso` en memoria. Cada curso se extrae con la función interna `parsearCurso()`, que a su vez llama a `parsearGrupo()` para cada uno de los grupos del curso.

Funciones expuestas:
- `cargarCatalogo(rutaArchivoJson, &cantidadCursos)` → Retorna el arreglo de cursos.
- `liberarCatalogo(arregloCursos, cantidadCursos)` → Libera toda la memoria del catálogo incluyendo grupos, profesores, bloques y arreglos dinámicos de requisitos.

---


#### `horario.c / horario.h` — Detección de choques

Convierte los strings de horario al formato `"Martes - 7:30:10:20"` en structs `BloqueHorario` con los campos `inicioMin` y `finMin` expresados en minutos desde la medianoche. Debido a esta conversión, La detección de choque entre dos bloques se reduce a una comparación numérica simple. Luego recorre todos los pares posibles de cursos y grupos del catálogo y cuando detecta un conflicto agrega el código del curso afectado al arreglo dinámico `chocaCon` de cada uno de los dos cursos involucrados.

Funciones expuestas:
- `parsearBloqueHorario(texto, &bloque)` → Convierte el string al struct.
- `bloquesChocan(bloqueA, bloqueB)` → Retorna 1 si los rangos se solapan.
- `gruposChocan(grupoA, grupoB)` → Compara todos los bloques de dos grupos.
- `agregarChoque(curso, codigoQueChoca)` → Inserta un código en `chocaCon` sin duplicados, usando `realloc`.
- `detectarChoques(arregloCursos, cantidadCursos)` → Recorre todos los pares y actualiza ambos cursos si hay conflicto.

---

#### `historial.c / historial.h` — Carga historial del estudiante

Lee un archivo de texto plano con la siguiente estructura: primera línea es el nombre de la carrera, y las líneas siguientes son los códigos de los cursos aprobados. Realiza dos pasadas sobre el archivo: la primera cuenta las líneas válidas para saber cuánta memoria reservar, y la segunda las lee y almacena en un struct `Historial`.

Funciones expuestas:
- `cargarHistorial(rutaArchivo)` → Retorna un `Historial*` con la carrera y el arreglo de cursos aprobados.
- `liberarHistorial(historial)` → Libera todos los strings y el struct `Historial` generado .

---

#### `consulta.c / consulta.h` — Validación de requisitos

Recorre el catálogo completo y determina, para cada curso, si el estudiante cumple todos sus requisitos según el historial cargado. Descarta automáticamente los cursos que el estudiante ya aprobó. Si un curso cumple todos los requisitos, se marca con `estudiantePuedeMatricular = true` y se inserta en una lista enlazada de resultados. 

Funciones expuestas:
- `obtenerCursosDisponibles(arregloCursos, cantidadCursos, aprobados, cantidadAprobados)` → Retorna una `ListaCursos` con punteros a los cursos disponibles.

---

#### `exportar.c / exportar.h` — Generación del archivo de salida

Serializa el arreglo de structs `Curso` (ya procesado con choques y matriculabilidad calculados) de vuelta a JSON usando la biblioteca `cJSON`. Construye el JSON objeto por objeto: primero el curso con sus campos estáticos, luego sus arreglos dinámicos (`requisitos`, `correquisitos`, `choca_con`), y finalmente el arreglo de grupos con sus bloques, profesores, horarios y demás información. El resultado se escribe al archivo de salida `catalogo_salida.json`.

Funciones expuestas:
- `exportarCatalogo(arregloCursos, cantidadCursos, rutaSalida)` → Retorna 1 si tuvo éxito, 0 si hubo error.

---

#### `manejoArreglos.c / manejoArreglos.h` — Utilidades de strings

Contiene funciones de uso general compartidas por múltiples módulos para manejar arreglos dinámicos de strings y su conversión hacia o desde JSON.

Funciones expuestas:
- `copiarStr(destino, tamDestino, origen)` → Copia segura de string con límite de tamaño para evitar desbordamientos de buffer.
- `leerArregloStrings(arregloJson, &cantidad)` → Convierte un arreglo JSON de strings en un `char**` dinámico.
- `arregloStringsAJson(arreglo, cantidad)` → Convierte un `char**` en un arreglo `cJSON`.
- `liberarArregloStrings(arreglo, cantidad)` → Libera cada elemento y el arreglo contenedor.

---

#### `listaEnlazada.c / listaEnlazada.h` — Lista enlazada de cursos

Implementa una lista enlazada simple de punteros `Curso*`. Los nodos almacenan punteros al arreglo principal (no copias), por lo que la lista es liviana en memoria y no duplica datos. Se utiliza para acumular los cursos disponibles para el estudiante.

Funciones expuestas:
- `inicializarLista(lista)` → Deja la lista en estado vacío.
- `insertarCurso(lista, curso)` → Inserta un curso al final de la lista.
- `liberarLista(lista)` → Libera los nodos pero no los cursos apuntados (esos los libera `liberarCatalogo`).

---

#### `modelos.h` — Definición de structs

Define los cuatro structs de datos principales del sistema. Es incluido por la mayoría de los módulos.

#### `constantes.h` — Constantes del sistema

Define los tamaños máximos de los campos de texto de los structs usando `#define`. Centralizar estas constantes permite modificar límites sin tocar la lógica de los módulos.

#### `lib/cJSON` — Librería externa

Librería de código abierto para parsear y generar JSON en C puro. Se incluyó directamente en el proyecto (sin dependencias externas) para garantizar que el ejecutable compile sin configuración adicional.

---

## 2.2 Decisiones de diseño

### 2.2.1 Justificación de decisiones propias y específicas del dataset

#### Recolección de datos desde TEC-Digital

Para la obtención de los horarios de cursos se utilizó la Guía de Horarios del Instituto Tecnológico de Costa Rica. En una primera instancia se intentó trabajar con la versión pública de dicha guía, sin embargo, al estar construida con una tecnología antigua (ASP.NET WebForms), no exponía una API convencional de fácil acceso.
Como alternativa, se inspeccionó el módulo de Guía de Horario dentro del Expediente Estudiantil del TEC-Digital mediante las herramientas de desarrollo del navegador (DevTools → Network). A través de este análisis se identificó que la página realiza una solicitud HTTP de tipo POST al siguiente endpoint:

https://tecdigital.tec.ac.cr/tda-expediente-estudiantil/ajax/tabla_guia_horario

Dicha solicitud requiere tres parámetros en el cuerpo de la petición:

<img width="371" height="168" alt="Screenshot from 2026-09-19 23-55-20" src="https://github.com/user-attachments/assets/9ff8dcaa-70fd-4b6d-8cb8-f73962bf600f" />

Adicionalmente, la solicitud requiere autenticación mediante la cookie de sesión ad_session_id, obtenida al iniciar sesión en el TEC-Digital.

Para la obtención del plan de estudios con requisitos y correquisitos de cada curso, se identificó un segundo endpoint utilizado por el módulo de malla curricular:

https://tecdigital.tec.ac.cr/tds-curriculum-exp/ajax/json_draw_angular?id_plan=2103

Este endpoint retorna un JSON con la estructura completa del plan de estudios, incluyendo los campos requirements, co_requirements y equivalent por curso, los cuales fueron utilizados para enriquecer el catálogo final.

Con base en ambos endpoints se desarrolló un script en Python que automatiza la descarga, limpieza y combinación de los datos, generando el archivo catalogo_cursos.json que sirve como entrada al programa en C.

De la información que podemos obtener por medio del API que usa tec digital quitamos la siguiente info de cada curso

- Aula
- Cupo
- Semestre en el que está la materia

Para cada escuela se obtuvo el payload donde se obtiene el código de la carrera que usa el api y se pone el archivo de python para que haga todos los cálculos, a partir de ese payload es que se genero el python para realizar consultas automáticas de acuerdo con el código de la escuela a la que pertenece cada curso. De manera que en el mismo python se pueden realizar múltiples consultas a todas las escuelas de interés y así obtener todos los grupos disponibles para cada curso

<img width="599" height="327" alt="Screenshot from 2026-09-19 23-59-15" src="https://github.com/user-attachments/assets/0323e365-5569-4acf-ae1a-a5b7fc7d9ce4" />
<img width="535" height="476" alt="Screenshot from 2026-09-20 00-00-21" src="https://github.com/user-attachments/assets/854ed3d1-78aa-40cb-bce8-fd2ff0b2d216" />

Mediante la obtención de los payloads nos dimos cuenta que el código del curso era el mismo o parecido que utilizaban para hacer consultas en el api de la página del tec digital

<img width="416" height="456" alt="Screenshot from 2026-09-20 00-01-13" src="https://github.com/user-attachments/assets/439275bd-955b-4ead-a71e-0a51615c2496" />

Encontramos en plan de estudios de cada carrera en json al revisar las respuestas de la pagina al seleccionar la escuela de cada carrera especifica.

Esta decisión de automatizar la recolección (en lugar de transcribir manualmente) garantiza exactitud en los datos y permite regenerar el catálogo fácilmente ante cambios de horario.

#### Cursos compartidos entre carreras

Un caso concreto del dataset es que varios cursos aparecen en ambas carreras. Por ejemplo, `MA0101 Matemática General`, `QU1102 Laboratorio de Química Básica I` y `CS1502 Introducción a la Técnica, Ciencia y Tecnología` forman parte de los primeros semestres tanto de Ingeniería en Computadores como de Ingeniería en Electrónica. El catálogo de cada carrera es independiente, pero los datos de horario se obtienen de las mismas escuelas.

#### Cursos sin grupos en el período actual

El catálogo incluye `CI0205 Prueba Avanzada de Inglés`, que no tiene grupos registrados en el período consultado (II Semestre 2026). En lugar de descartarlo, se decidió incluirlo en el catálogo con `"grupos": []`, porque el curso existe en el plan de estudios y su ausencia de grupos es una condición del período, no un error del dato. Esto significa que puede aparecer como elegible para un estudiante que cumpla sus requisitos, aunque sin grupos disponibles para matricular actualmente.

#### Representación de horarios como enteros

Los bloques de horario del TEC-Digital vienen en texto con el formato `"Martes - 7:30:10:20"`. Se decidió convertirlos a minutos desde medianoche (por ejemplo, `7:30` → `450 minutos`) al momento de comparar, en lugar de guardar strings o structs en los archivos. Esto simplifica radicalmente la detección de choque: dos bloques chocan si y solo si `A.inicioMin < B.finMin && B.inicioMin < A.finMin`, sin necesidad de parsear strings durante la comparación.

Por ejemplo, `MA0101 grupo 001` tiene bloque `Martes - 7:30:10:20` (450 a 620 min) y `CE1101 grupo 001` tiene `Martes - 7:30:9:20` (450 a 560 min). Ambos coinciden en el mismo día y sus rangos se solapan, por lo que se detecta choque correctamente.

#### Correquisitos no validados como condición de acceso

Los correquisitos del plan de estudios (por ejemplo, `FI1102 Física General II` tiene como requisito `FI1101` y como correquisito `MA1102`) se almacenan en el catálogo de salida pero **no se usan para bloquear la matriculabilidad**. Esto es correcto porque los correquisitos son cursos que se deben llevar simultáneamente, no previamente. La validación de si el estudiante lleva el correquisito al mismo tiempo es responsabilidad de la Etapa 2, que ya trabaja con combinaciones de cursos.

---

### 2.2.2 Caso límite real encontrado y cómo se resolvió

**Problema:** Cursos compartidos entre múltiples escuelas generaban entradas duplicadas en `horarios_raw.json`.

Durante la recolección, el scraper consultaba múltiples escuelas por separado (Computadores, Ciencias, Matemática, Física, etc.) para obtener todos los grupos disponibles para los cursos de cada carrera. Esto hacía que cursos que aparecen en varias escuelas (como `MA0101` o `FI1101`) se incluyeran múltiples veces en el JSON crudo, con grupos duplicados o incluso inconsistentes entre sí.

**Solución:** En `CombinarDatos.py` se construyó un índice de horarios agrupado por código de curso (`horarios_por_codigo`), donde para cada código se acumulan todos los grupos encontrados en cualquier escuela. Al momento de generar el catálogo final, se toman todos los grupos de ese índice sin importar la escuela de origen, y se conservan todos (incluyendo los de escuelas distintas a la principal), ya que representan grupos realmente disponibles para ese curso en ese período. Esto garantiza que el catálogo refleje todas las opciones de horario reales.

---

### 2.2.3 Justificación del formato de salida

**Formato elegido: JSON**

El archivo de salida usa el mismo formato que el archivo de entrada: JSON. Esta decisión está directamente ligada a la naturaleza incremental del sistema: la Etapa 2 en Racket necesita consumir fácilmente los datos producidos en esta etapa, y JSON es un formato estándar con soporte nativo en prácticamente todos los lenguajes.

**Ventajas concretas para este proyecto:**

1. **Contrato claro entre etapas.** El JSON de salida extiende el de entrada añadiendo únicamente los dos campos calculados: `choca_con` (lista de códigos de cursos que generan conflicto) y `estudiante_puede_matricular` (booleano). Racket puede leer el mismo archivo sin transformaciones adicionales.

2. **Legibilidad durante desarrollo.** El archivo generado con `cJSON_Print()` produce JSON indentado y legible. Durante el desarrollo, esto permitió verificar manualmente que los choques y la matriculabilidad se calculaban correctamente sin necesidad de herramientas adicionales.

3. **Arreglos vacíos explícitos.** Se decidió exportar siempre `[]` en lugar de omitir las claves cuando un curso no tiene requisitos, correquisitos o choques. Esto hace el formato predecible: la Etapa 2 siempre puede asumir que las claves existen.

4. **Extensibilidad.** Si en etapas futuras se necesita agregar información adicional por curso (por ejemplo, una puntuación de prioridad o notas del estudiante), se puede añadir como un campo más al objeto JSON sin romper la compatibilidad con los campos existentes.

**Ejemplo del objeto de salida para un curso:**

```json
{
  "codigo": "FI1102",
  "nombre": "física general ii",
  "creditos": 3,
  "requisitos": ["FI1101"],
  "correquisitos": ["MA1102"],
  "choca_con": ["CE1103", "MA1403"],
  "estudiante_puede_matricular": false,
  "grupos": [
    {
      "numero_grupo": "001",
      "nombre": "Física general II",
      "tipo_grupo": "Regular",
      "profesores": ["Pérez Mora Carlos"],
      "bloques_horario": ["Martes - 7:30:9:20", "Jueves - 7:30:9:20"]
    }
  ]
}
```

---

## 2.3 Estructuras de datos desarrolladas

### `BloqueHorario`
```c
typedef struct {
    char dia[4];     // Abreviatura: "LUN", "MAR", "MIE", "JUE", "VIE"
    int inicioMin;   // Hora de inicio en minutos desde medianoche (ej: 7:30 → 450)
    int finMin;      // Hora de fin en minutos desde medianoche   (ej: 10:20 → 620)
} BloqueHorario;
```
Representa un bloque de tiempo ya parseado. Usar enteros en lugar de strings permite comparar rangos de horario con aritmética simple sin llamadas a funciones de string.

---

### `Grupo`
```c
typedef struct {
    char nombre[128];
    char numeroGrupo[16];
    char tipoGrupo[32];       // "Regular", "Asistida", etc.
    char **bloquesHorario;    // Arreglo dinámico de strings de horario
    int numBloquesHorario;
    char **profesores;        // Arreglo dinámico de nombres de profesores
    int numProfesores;
} Grupo;
```
Representa un grupo específico de un curso. Los bloques de horario se almacenan como strings originales para exportación y se parsean a `BloqueHorario` solo durante la comparación, evitando almacenar información redundante.

---

### `Curso`
```c
typedef struct {
    char codigo[16];
    char nombre[160];
    int creditos;
    char **requisitos;                // Arreglo dinámico de códigos
    int numRequisitos;
    char **correquisitos;             // Arreglo dinámico de códigos
    int numCorrequisitos;
    Grupo *grupos;                    // Arreglo dinámico de structs Grupo
    int numGrupos;
    char **chocaCon;                  // Calculado por detectarChoques()
    int numChocaCon;
    bool estudiantePuedeMatricular;   // Calculado por obtenerCursosDisponibles()
} Curso;
```
Estructura central del sistema. Todos los arreglos son dinámicos (reservados con `malloc`/`realloc`) porque la cantidad de grupos, requisitos y choques varía por curso y no se conoce en tiempo de compilación. Los campos `chocaCon` y `estudiantePuedeMatricular` se inicializan vacíos/falsos y se llenan durante el procesamiento.

---

### `Historial`
```c
typedef struct {
    char carrera[32];
    char **aprobados;   // Arreglo dinámico de códigos de cursos aprobados
    int cantidadAprobados;
} Historial;
```
Representa el estado académico del estudiante. El arreglo `aprobados` es dinámico porque su tamaño depende del contenido del archivo de historial. Se reserva en dos pasadas: primero se cuenta el número de líneas y luego se reserva la memoria exacta necesaria.

---

### `NodoCurso` y `ListaCursos`
```c
typedef struct NodoCurso {
    Curso *curso;               // Puntero al curso en el arreglo principal
    struct NodoCurso *siguiente;
} NodoCurso;

typedef struct ListaCursos {
    NodoCurso *cabeza;
    int cant;
} ListaCursos;
```
Lista enlazada simple usada para acumular los cursos disponibles para el estudiante. Los nodos almacenan punteros (no copias) al arreglo principal de cursos, por lo que la lista no duplica datos en memoria. Al liberar la lista, solo se liberan los nodos, no los cursos apuntados.

---

## Compilación

```bash
gcc -o build/catalogo_run \
    src/main.c src/catalogo.c src/horario.c src/historial.c \
    src/consulta.c src/exportar.c src/manejoArreglos.c \
    structures/listaEnlazada.c lib/cJSON.c \
    -I include -I lib \
    -Wall -Wextra -std=c11
```

---

# CEmestre : Constructor de Horarios · Etapa 2

## Descripción general

CEmestre es un sistema que ayuda a un estudiante del TEC a armar su horario de matrícula. Esta segunda etapa, implementada en Racket bajo el paradigma funcional (Typed Racket), recibe el catálogo de cursos que produjo la Etapa 1 en C (con los choques de horario entre grupos y la elegibilidad de cada curso ya calculados) y un archivo de preferencias del estudiante con los cursos y grupos que le interesan y el rango de créditos deseado. A partir de eso genera, de forma perezosa mediante streams, las combinaciones de horario válidas (un grupo por cada curso elegido y sin choques entre ellos), las filtra por el rango de créditos y por los correquisitos, y las exporta a un archivo JSON que la Etapa 3 (Prolog) va a consumir.

---

## Uso del ejecutable

El programa se puede ejecutar de tres formas. En todas recibe tres rutas: el catálogo de la Etapa 1 (JSON), el archivo de preferencias del estudiante (JSON) y el archivo de salida (JSON).

### 1. Botón Run en DrRacket

Se abre `main.rkt` y se presiona Run. Como no hay argumentos de línea de comandos, se usan las rutas por defecto definidas al inicio de `main.rkt`:

| Constante | Valor por defecto |
|---|---|
| `catalogo-por-defecto` | `../ArchivosEntrada/catalogo_salida.json` |
| `preferencias-por-defecto` | `../ArchivosEntrada/preferencias_ejemplo.json` |
| `salida-por-defecto` | `../ArchivosSalida/combinaciones_salida.json` |


### 2. Línea de comandos

Desde la carpeta del proyecto:

```bash
racket main.rkt <catalogo.json> <preferencias.json> <salida.json>
```

**Ejemplo:**
```bash
racket main.rkt ../ArchivosEntrada/catalogo_salida.json ../ArchivosEntrada/preferencias_ejemplo.json ../ArchivosSalida/combinaciones_salida.json
```

### 3. Llamando `ejecutar` desde la ventana de Interactions de DrRacket

Después de presionar Run, se puede probar con otros archivos sin modificar el código:

```racket
(ejecutar "../ArchivosEntrada/catalogo_salida.json"
          "../ArchivosEntrada/preferencias_ejemplo.json"
          "../ArchivosSalida/combinaciones_salida.json")
```

### Mensajes del programa

Al terminar, `ejecutar` imprime cuántas combinaciones escribió y en qué archivo.

### Archivos de entrada

**1. Catálogo (salida de la Etapa 1).** Arreglo JSON de cursos. Esta etapa usa los siguientes campos, y si alguno falta o tiene un tipo distinto el programa se detiene con un mensaje que indica cuál:

```json
{
  "codigo": "CE1101",
  "nombre": "introducción a la programación",
  "creditos": 3,
  "carreras": ["Ingeniería en Computadores"],
  "requisitos": [],
  "correquisitos": [],
  "ya_aprobado": false,
  "estudiante_puede_matricular": true,
  "grupos": [
    {
      "id_grupo": 1,
      "numero_grupo": "001",
      "tipo_grupo": "Regular",
      "profesores": ["Schmidt Peralta Jeff"],
      "bloques_horario": ["Martes - 7:30:9:20", "Jueves - 7:30:9:20"],
      "choca_con": [{"curso": "MA1403", "id_grupo": 1}]
    }
  ]
}
```

**2. Preferencias del estudiante.** Objeto JSON con el rango de créditos y la lista de cursos de interés, cada uno con los grupos que el estudiante está dispuesto a matricular (más de un grupo por curso cuando aplica):

```json
{
  "creditos_minimos": 7,
  "creditos_maximos": 10,
  "cursos_interes": [
    {"curso": "MA1403", "grupos": [1, 2]},
    {"curso": "CE1101", "grupos": [1, 2]}
  ]
}
```

Los números de `grupos` corresponden al `id_grupo` del catálogo (el identificador único del grupo dentro de su curso), no al `numero_grupo` visible (`"001"`).

---

## 2.1 Arquitectura del proyecto

### Estructura de carpetas

```
etapa2_racket/
├── ArchivosEntrada/
│   ├── catalogo_salida.json         # Catálogo producido por la Etapa 1 (C)
│   └── preferencias_ejemplo.json    # Preferencias del estudiante
├── ArchivosSalida/
│   └── combinaciones_salida.json    # Archivo generado por el programa
└── src/
    ├── main.rkt                     # Punto de entrada y coordinador
    ├── modelos.rkt                  # Definición de structs
    ├── lectura.rkt                  # JSON -> structs, con validación
    ├── choques.rkt                  # Consulta de choques entre grupos
    ├── streams.rkt                  # Núcleo perezoso (sin tipos)
    ├── combinaciones.rkt            # Genera combinaciones con streams
    ├── filtros.rkt                  # Opciones y reglas de filtrado
    └── exportar.rkt                 # Combinaciones -> JSON de salida
```

---

### Flujo de ejecución

```mermaid
flowchart TD
    A([Argumentos CLI]) --> B

    B["leer catalogo y leer preferencias\nJSON para pasarlos a structs validados"]
    B --> C

    C["Armar opciones con los \nCursos de interés habilitados con sus grupos"]
    C --> D

    D["subconjuntos (stream)\nPoda por créditos máximos"]
    D --> E

    E["Generar-combinaciones (stream)\nUn grupo por curso, poda por choques"]
    E --> F

    F["filtrar-con-todos\nCréditos en rango y correquisitos cubiertos"]
    F --> G

    G["exportar-combinaciones\nEscribe el JSON mientras el stream produce"]
    G --> H(Archivo de salida)

    style A fill:#1B3F72,color:#fff,stroke:none
    style H fill:#1A2F4E,color:#cdd8e8,stroke:none
    style B fill:#1E3A5F,stroke:#4A7FB5,color:#A8C4E0
    style C fill:#1A3550,stroke:#3A6D9E,color:#A8C4E0
    style D fill:#162E47,stroke:#2E5F8A,color:#8AB4D4
    style E fill:#112640,stroke:#245278,color:#7DAAC8
    style F fill:#0D1F35,stroke:#1C4568,color:#6D9EBE
    style G fill:#0A1828,stroke:#183A58,color:#5D8EAE
    linkStyle default stroke:#F472B6,stroke-width:2px
```
---

### Descripción de módulos

#### `main.rkt` — Punto de entrada y coordinador principal

Une todas las piezas. Lee los argumentos, carga los archivos, valida que el mínimo de créditos no supere el máximo, construye el stream de combinaciones válidas y se lo pasa al exportador. Todo el cálculo está en `construir-combinaciones`, que no hace trabajo hasta que alguien consume el stream.

Funciones principales:
- `construir-combinaciones(catalogo, prefs)` → Retorna el stream de combinaciones válidas.
- `subconjuntos(opciones, disponible)` → Stream de los subconjuntos de cursos cuyos créditos no superan lo disponible.
- `creditos-de-opciones(opciones)` y `sumar-creditos(selecciones)` → Suman créditos recursivamente.
- `selecciones->combinacion(selecciones)` → Convierte una lista de pares curso-grupo en un struct `combinacion` con su total de créditos.
- `ejecutar(ruta-catalogo, ruta-preferencias, ruta-salida)` → Corre todo el proceso y retorna cuántas combinaciones se exportaron.

---

#### `modelos.rkt` — Definición de structs

Define los ocho structs del sistema (ver sección 2.3). Todos son inmutables y transparentes, el archivo los exporta con `all-defined-out` para que el resto de módulos los use.

---

#### `lectura.rkt` — Carga y validación de los archivos de entrada

Convierte el JSON del catálogo y el de preferencias en structs. Cada valor pasa por una función de validación que lanza un error con un mensaje claro si el campo no existe o no tiene el tipo esperado.

Funciones principales:
- `leer-catalogo(ruta)` → Retorna una lista de structs `curso`.
- `leer-preferencias(ruta)` → Retorna un struct `preferencias`.

---

#### `choques.rkt` — Consulta de choques entre grupos

Consulta el campo `choca_con` que la Etapa 1 ya calculó para cada grupo. Un par curso-grupo se identifica por `(código del curso, id_grupo)`.

Funciones principales:
- `selecciones-chocan?` → Indica si dos pares curso-grupo chocan.
- `choca-con-elegidos?` → Indica si una opcion candidata choca con alguna de las ya elegidas. Es la regla que usa el generador para podar.
---

#### `streams.rkt` — Núcleo perezoso

Contiene lo que necesita `stream-cons`.

Funciones principales :
- `append-perezoso(primero, resto-thunk)` → Concatena dos streams sin calcular el segundo hasta que el primero se acabe. El segundo viene envuelto en una función sin parámetros (thunk).
- `append-map-perezoso(f, lista)` → Aplica `f` (que devuelve un stream) a cada elemento de la lista y junta los resultados en un solo stream, de forma perezosa.
---

#### `combinaciones.rkt` — Generación de combinaciones

Genera, con streams, las combinaciones de un grupo por curso. Recibe una lista donde cada elemento son las alternativas de un curso y va eligiendo un grupo de cada uno, descartando una rama apenas la candidata choca con lo ya elegido.

Funciones principales:
- `selecciones-de-opcion(opcion)` → Convierte un curso de interés en la lista de pares curso-grupo que se pueden probar.
- `opciones->listas(opciones)` → Hace lo anterior para todas las opciones.
- `generar-combinaciones(cursos, elegidos)` → Retorna el stream de combinaciones. Internamente usa `probar-candidata` (decide si una candidata se agrega o se descarta) y `combinacion-final` (cierra una combinación completa).

---

#### `filtros.rkt` — Opciones y reglas de filtrado

Contiene la función con polimorfismo paramétrico del proyecto y las reglas con las que se filtran las combinaciones.

Funciones principales:
- `filtrar-con-todos(criterios, secuencia)` → Función polimórfica `(All (A) ...)`: deja pasar solo los elementos de un stream que cumplen todas las reglas de la lista.
- `creditos-en-rango?(minimo, maximo)` → Devuelve una regla que indica si una combinación tiene los créditos dentro del rango.
- `correquisitos-cubiertos?(catalogo)` → Devuelve una regla que indica si todos los correquisitos de una combinación están cubiertos.
- `armar-opciones(catalogo, intereses)` → Busca cada curso de interés en el catálogo y se queda con los habilitados que tienen al menos uno de los grupos pedidos.

---

#### `exportar.rkt` — Generación del archivo de salida

Serializa las combinaciones a JSON y las escribe a medida que el stream las produce, sin convertirlo antes a una lista. Al final escribe cuántas combinaciones fueron.

Funciones principales:
- `exportar-combinaciones(ruta, minimo, maximo, secuencia)` → Crea el archivo de salida y retorna el total de combinaciones escritas.
- `seleccion->json(par)` y `combinacion->json(comb, id)` → Convierten los structs a objetos JSON.

---

## 2.2 Decisiones de diseño

### 2.2.1 Justificación de decisiones propias y específicas de la implementación

#### Se reutiliza `choca_con` de la Etapa 1 en lugar de recalcular los horarios

El enunciado indica que la lógica de choques ya construida no debe modificarse, y la Etapa 1 deja calculado, para cada grupo, con cuáles grupos de otros cursos choca. Racket solo consulta esa información. Por ejemplo, `CE1101` grupo 001 se imparte martes y jueves de 7:30 a 9:20; cualquier grupo que coincida con ese horario aparece en su lista `choca_con`, y `choca-con-elegidos?` lo detecta sin interpretar ningún horario.

Como el número de grupo se repite entre cursos, cada choque se identifica con el par `(código del curso, id_grupo)`, donde `id_grupo` es el identificador único dentro del curso que asigna la Etapa 1. La Etapa 1 registra cada choque en ambos grupos, por lo que basta con revisar uno de los dos lados.

#### `streams.rkt` sin tipos, conectado con `require/typed`

`stream-cons` no se puede usar directamente dentro de `#lang typed/racket`: el verificador de tipos falla con un error de identificador sin tipo (`missing type for identifier`). Por eso las funciones que construyen streams con `stream-cons` viven en `streams.rkt`, escrito en `#lang racket`, y los módulos tipados las importan declarando su tipo con `require/typed`:

```racket
(require/typed "streams.rkt"
  [append-map-perezoso (All (A B) (-> (-> A (Sequenceof B)) (Listof A) (Sequenceof B)))])
```

Esa declaración también funciona como contrato en la frontera entre ambos archivos, de modo que los datos que cruzan se verifican en tiempo de ejecución.

#### Poda de choques durante la generación

`generar-combinaciones` no arma la combinación completa para descartarla después: en cuanto un grupo candidato choca con alguno de los ya elegidos (`choca-con-elegidos?`), devuelve un stream vacío y esa rama no se explora más. Por ejemplo, si el grupo 001 de `MA1403` choca con el grupo 001 de `CE1101`, ninguna combinación que contenga ambos se llega a generar.

#### Los cursos de interés son opciones: cada combinación usa un subconjunto

Cada combinación incluye un subconjunto de los cursos de interés, con un grupo por cada curso elegido, cuyo total de créditos cae en el rango pedido. Esta interpretación hace que el rango de créditos tenga sentido: si cada combinación incluyera siempre todos los cursos de interés, el total de créditos sería el mismo en todas y el filtro no discriminaría nada. Si todos los cursos juntos se pasan del máximo, igual se generan combinaciones con los subconjuntos que sí caben.

Los créditos dependen del curso y no del grupo, así que `subconjuntos` poda antes de generar grupos: apenas un curso ya no cabe en los créditos disponibles, deja de explorar los subconjuntos que lo incluyen. Después, `construir-combinaciones` se queda solo con los subconjuntos no vacíos que alcanzan el mínimo y genera las combinaciones de grupos de cada uno.

#### Filtros como reglas reutilizables

`filtrar-con-todos` es la función con polimorfismo paramétrico: recibe una lista de reglas `(-> A Boolean)` y un stream de elementos de cualquier tipo `A`, y devuelve el stream con los elementos que cumplen todas las reglas. Funciona igual con combinaciones, cursos o números, porque no usa nada específico del tipo. Las reglas se construyen con funciones que devuelven funciones: `creditos-en-rango?` recibe el mínimo y el máximo y devuelve la regla que revisa los créditos de una combinación, y `correquisitos-cubiertos?` recibe el catálogo y devuelve la regla de correquisitos. Agregar un criterio nuevo es agregar una regla a la lista, sin modificar `filtrar-con-todos`.

Un correquisito se considera cubierto si el curso ya está aprobado en el catálogo o si está dentro de la misma combinación.

---

### 2.2.2 Caso límite real encontrado y cómo se resolvió

**Problema:** Cuando ningún curso de interés era válido, el programa exportaba una combinación vacía con 0 créditos en lugar de ninguna.

Mientras probábamos el generador con una lista de cursos de interés vacía (por ejemplo, un código que no existe en el catálogo, o cursos que el estudiante no puede matricular), `armar-opciones` devolvía una lista vacía. Pero `generar-combinaciones` interpreta "no quedan cursos por elegir" como que ya hay una combinación completa, así que producía **una** combinación sin ningún curso y con 0 créditos. Si además el mínimo de créditos era 0, esa "combinación" pasaba el filtro y se habría exportado como si fuera un horario válido.

**Solución:** En `construir-combinaciones` se excluyen los subconjuntos vacíos antes de generar las combinaciones, con un filtro perezoso que exige que el subconjunto no sea vacío y alcance el mínimo de créditos:

```racket
(lambda ([sub : (Listof opcion)])
  (and (not (null? sub)) (>= (creditos-de-opciones sub) minimo)))
```

Con esto, una lista de intereses sin cursos válidos produce un archivo de salida con `"combinaciones": []` y `"total_combinaciones": 0`, y el programa avisa por consola qué cursos se omitieron.

---

### 2.2.3 Justificación del formato de salida

**Formato elegido: JSON**

Se mantuvo JSON porque es el formato de la entrada de esta etapa, y porque los datos son jerárquicos: una combinación contiene varios pares curso-grupo, y cada par lleva los datos de su curso, incluida una lista de carreras. Eso no se representa de forma limpia en CSV. Además, la Etapa 3 (Prolog) y la Etapa 4 (Java) tienen bibliotecas para leer JSON.

**Estructura del archivo:**

```json
{
  "creditos_minimos": 7,
  "creditos_maximos": 10,
  "combinaciones": [
    {
      "id": 1,
      "creditos_totales": 7,
      "selecciones": [
        {
          "curso": "MA1403",
          "nombre": "matemática discreta",
          "creditos": 4,
          "id_grupo": 1,
          "numero_grupo": "001",
          "carreras": ["Ingeniería en Computadores"]
        },
        {
          "curso": "CE1101",
          "nombre": "introducción a la programación",
          "creditos": 3,
          "id_grupo": 2,
          "numero_grupo": "002",
          "carreras": ["Ingeniería en Computadores"]
        }
      ]
    }
  ],
  "total_combinaciones": 1
}
```

**Decisiones de diseño ligadas al formato:**

1. **Escritura perezosa:** `escribir-combinaciones` escribe cada combinación apenas el stream la genera, sin armar antes una lista completa. Así la generación con streams no pierde su sentido.
2. **`total_combinaciones` al final:** solo se conoce después de consumir el stream, por lo que lo lleva el acumulador del `for/fold` y se escribe al cerrar.
3. **`id_grupo` y `numero_grupo`:** el primero es el que usa `choca_con`. El segundo es el número visible que mostrará en etapas posteriores.
4. **`carreras` como lista:** hay cursos compartidos entre carreras (ej. `MA0101`), y una lista cubre ambos casos.
5. **Datos listos para Prolog:** cada combinación lleva un `id` y sus `creditos_totales`, y el encabezado repite el rango de créditos, así que Prolog no recalcula nada.

---

## 2.3 Estructuras de datos desarrolladas

Todas están definidas en `modelos.rkt` con `struct`, tipadas campo por campo y con `#:transparent`.

### `choque`
```racket
(struct choque ([codigo-curso : String]
                [id-grupo : Integer])
  #:transparent)
```
Un choque de horario: identifica el grupo de otro curso con el que choca un grupo. Equivale a un elemento de `choca_con` en el JSON de la Etapa 1.

---

### `grupo`
```racket
(struct grupo ([id : Integer]
               [numero : String]
               [tipo : String]
               [profesores : (Listof String)]
               [bloques : (Listof String)]
               [choques : (Listof choque)])
  #:transparent)
```
Un grupo de un curso. `id` lo distingue de los demás grupos del mismo curso (el número visible se repite entre cursos, y hasta puede repetirse dentro de uno), `bloques` guarda los horarios como texto y `choques` trae los choques ya calculados por C.

---

### `curso`
```racket
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
```
Refleja el curso tal como lo exporta la Etapa 1. `carreras` es una lista porque un curso puede pertenecer a más de una carrera, `ya-aprobado` permite saber si un correquisito ya se cumplió, y `puede-matricular` indica si el estudiante cumple los requisitos.

---

### `interes`
```racket
(struct interes ([codigo-curso : String]
                 [ids-grupos : (Listof Integer)])
  #:transparent)
```
Un curso que el estudiante quiere matricular, junto con los grupos que está dispuesto a tomar.

---

### `preferencias`
```racket
(struct preferencias ([creditos-minimos : Integer]
                      [creditos-maximos : Integer]
                      [intereses : (Listof interes)])
  #:transparent)
```
El archivo de preferencias completo: el rango de créditos y la lista de intereses.

---

### `opcion`
```racket
(struct opcion ([curso : curso]
                [grupos : (Listof grupo)])
  #:transparent)
```
Un curso de interés ya validado (existe en el catálogo y se puede matricular), junto con solo los grupos pedidos que realmente existen en él. Es la entrada del generador.

---

### `seleccion`
```racket
(struct seleccion ([curso : curso]
                   [grupo : grupo])
  #:transparent)
```
Un par curso-grupo escogido dentro de una combinación. Es la unidad con la que trabaja el generador y la que se exporta.

---

### `combinacion`
```racket
(struct combinacion ([selecciones : (Listof seleccion)]
                     [creditos : Integer])
  #:transparent)
```
Una combinación de horario: varios pares curso-grupo sin choques y la suma de sus créditos. Es el elemento de los streams que se filtran y se exportan.

---

### Streams (secuencias perezosas)

Además de los structs, la estructura central del programa es el stream: una secuencia cuyos elementos se calculan solo cuando alguien los pide, por lo que en memoria nunca existe la lista completa de combinaciones. Se construye con `stream-cons` en `streams.rkt` y se usa en todo el flujo:

| Stream | Tipo de sus elementos | Quién lo produce |
|---|---|---|
| Subconjuntos de cursos | `(Listof opcion)` | `subconjuntos` (`main.rkt`) |
| Combinaciones de un subconjunto | `(Listof seleccion)` | `generar-combinaciones` (`combinaciones.rkt`) |
| Combinaciones con créditos | `combinacion` | `stream-map` en `construir-combinaciones` |
| Combinaciones filtradas | `combinacion` | `filtrar-con-todos` (`filtros.rkt`) |

Operaciones desarrolladas:
- `append-perezoso(primero, resto-thunk)`: concatena dos streams. El segundo llega envuelto en un thunk, así que solo se calcula cuando el primero se acaba.
- `append-map-perezoso(f, lista)`: aplica `f` a cada elemento y junta los streams resultantes en uno solo.

---

## Ejecución

Requiere Racket instalado (el proyecto se probó con DrRacket 9.3). Las formas de ejecutarlo, con sus rutas de entrada y salida, están en la sección [Uso del ejecutable](#uso-del-ejecutable). No hay un paso de compilación aparte.

---
