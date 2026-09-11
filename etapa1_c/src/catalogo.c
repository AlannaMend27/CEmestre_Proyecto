#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include "catalogo.h"
#include "manejoArreglos.h"

//Este archivo tiene la logica para leer el json de los cursos y guardarlo en los structs

//Toma el objeto json de un grupo y llena el struct correspondiente
static void parsearGrupo(const cJSON *grupoJson, Grupo *grupo) {

    //Extraemos el valor string del json y lo copiamos al struct de forma segura
    copiarStr(grupo->nombre, sizeof(grupo->nombre), 
    cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(grupoJson, "nombre")));

    copiarStr(grupo->numeroGrupo, sizeof(grupo->numeroGrupo),
    cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(grupoJson, "numero_grupo")));

    copiarStr(grupo->tipoGrupo, sizeof(grupo->tipoGrupo),
    cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(grupoJson, "tipo_grupo")));

    const cJSON *bloquesJson = cJSON_GetObjectItemCaseSensitive(grupoJson, "bloques_horario");
    grupo->bloquesHorario = leerArregloStrings(bloquesJson, &grupo->numBloquesHorario);

    const cJSON *profesoresJson = cJSON_GetObjectItemCaseSensitive(grupoJson, "profesores");
    grupo->profesores = leerArregloStrings(profesoresJson, &grupo->numProfesores);

}

//Toma el objeto json de un curso, llena el struct y parsea sus grupos internos
static void parsearCurso(const cJSON *cursoJson, Curso *curso) {

    //Igual que en los grupos extraemos los valores para copiarlos al struct
    copiarStr(curso->codigo, sizeof(curso->codigo),
    cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(cursoJson, "codigo")));

    copiarStr(curso->nombre, sizeof(curso->nombre),
    cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(cursoJson, "nombre")));

    const cJSON *creditosJson = cJSON_GetObjectItemCaseSensitive(cursoJson, "creditos");
    
    //Verificamos si los creditos vienen como un numero valido para guardarlos
    if (cJSON_IsNumber(creditosJson)) {
        curso->creditos = creditosJson->valueint;
    } else {
        curso->creditos = 0;
    }

    //seguimos extrayendo
    const cJSON *requisitosJson = cJSON_GetObjectItemCaseSensitive(cursoJson, "requisitos");
    curso->requisitos = leerArregloStrings(requisitosJson, &curso->numRequisitos);

    const cJSON *correquisitosJson = cJSON_GetObjectItemCaseSensitive(cursoJson, "correquisitos");
    curso->correquisitos = leerArregloStrings(correquisitosJson, &curso->numCorrequisitos);

    const cJSON *chocaConJson = cJSON_GetObjectItemCaseSensitive(cursoJson, "choca_con");
    curso->chocaCon = leerArregloStrings(chocaConJson, &curso->numChocaCon);

    const cJSON *puedeMatricularJson = cJSON_GetObjectItemCaseSensitive(cursoJson, "estudiante_puede_matricular");
    curso->estudiantePuedeMatricular = cJSON_IsTrue(puedeMatricularJson);

    const cJSON *gruposJson = cJSON_GetObjectItemCaseSensitive(cursoJson, "grupos");
    
    //Validamos que haya un arreglo de grupos antes de intentar parsearlos
    if (cJSON_IsArray(gruposJson)) {
        curso->numGrupos = cJSON_GetArraySize(gruposJson);
        
        //Comprobamos si el curso tiene al menos un grupo para reservarle memoria
        if (curso->numGrupos > 0) {
            curso->grupos = malloc(sizeof(Grupo) * curso->numGrupos);
            
            //Si malloc falla, error
            if (!curso->grupos) {
                fprintf(stderr, "Error de memoria al reservar grupos en parsearCurso\n");
                exit(1);
            }
            
            int indiceGrupo = 0;
            const cJSON *grupoItemJson;
            
            //Recorremos el arreglo de grupos del json uno por uno
            cJSON_ArrayForEach(grupoItemJson, gruposJson) {
                parsearGrupo(grupoItemJson, &curso->grupos[indiceGrupo]);indiceGrupo++;
            }

        } else {
            //si hay cero grupos
            curso->grupos = NULL;
        }
        
    } else {
        // si del todo no viene la etiqueta grupos
        curso->grupos = NULL;
        curso->numGrupos = 0;
    }
}

//Lee el archivo JSON completo y construye el catalogo en memoria
Curso *cargarCatalogo(const char *rutaArchivoJson, int *cantidadCursos) {
    *cantidadCursos = 0;

    FILE *archivoJson = fopen(rutaArchivoJson, "rb");
    
    //Verificamos si pudimos abrir el archivo correctamente
    if (!archivoJson) {
        fprintf(stderr, "No se pudo abrir el archivo: %s\n", rutaArchivoJson);
        return NULL;
    }

    //Movemos el puntero al final para sacar el tamano del archivo y lo devolvemos al inicio
    fseek(archivoJson, 0, SEEK_END);
    long tamanioArchivo = ftell(archivoJson);
    fseek(archivoJson, 0, SEEK_SET);

    char *bufferArchivo = malloc(tamanioArchivo + 1);
    
    //Validamos que tengamos memoria para cargar el archivo en el buffer
    if (!bufferArchivo) {
        fclose(archivoJson);
        fprintf(stderr, "Error de memoria al crear buffer en cargarCatalogo\n");
        return NULL;
    }

    //Cargamos todo el contenido al buffer, le ponemos el caracter nulo al final y cerramos el archivo
    fread(bufferArchivo, 1, tamanioArchivo, archivoJson);
    bufferArchivo[tamanioArchivo] = '\0';
    fclose(archivoJson);

    //Parseamos el texto del buffer a un objeto json y liberamos la memoria del buffer
    cJSON *jsonRaiz = cJSON_Parse(bufferArchivo);
    free(bufferArchivo);

    //Revisamos si el parser de cJSON fallo para reportar el detalle del error
    if (!jsonRaiz) {
        const char *punteroError = cJSON_GetErrorPtr();
        const char *mensajeError;
        
        //Asignamos el mensaje de error
        if (punteroError) {
            mensajeError = punteroError;
        } else {
            mensajeError = "desconocido";
        }
        
        fprintf(stderr, "Error al parsear JSON cerca de: %s\n", mensajeError);
        return NULL;
    }

    //El json debe ser una lista de cursos asi que confirmamos que sea un arreglo
    if (!cJSON_IsArray(jsonRaiz)) {
        fprintf(stderr, "El JSON raiz no es un arreglo\n");
        cJSON_Delete(jsonRaiz);
        return NULL;
    }

    int totalCursos = cJSON_GetArraySize(jsonRaiz);
    Curso *arregloCursos = malloc(sizeof(Curso) * totalCursos);
    
    //Chequeamos si hubo error al reservar la memoria para todos los cursos
    if (!arregloCursos) {
        fprintf(stderr, "Error de memoria al reservar el catalogo en cargarCatalogo\n");
        cJSON_Delete(jsonRaiz);
        return NULL;
    }

    int indiceCurso = 0;
    const cJSON *cursoItemJson;
    
    //Extraemos cada curso del json y lo pasamos al struct en su posicion respectiva
    cJSON_ArrayForEach(cursoItemJson, jsonRaiz) {
        parsearCurso(cursoItemJson, &arregloCursos[indiceCurso]);
        indiceCurso++;
    }

    //Limpiamos el objeto json de la memoria, guardamos el total de cursos y retornamos el arreglo
    cJSON_Delete(jsonRaiz);
    *cantidadCursos = totalCursos;
    return arregloCursos;
}

//Limpia toda la memoria ocupada por el catalogo y sus componentes
void liberarCatalogo(Curso *arregloCursos, int cantidadCursos) {

    //Si el arreglo viene nulo salimos de una vez para no caer en un error
    if (!arregloCursos) {
        return;
    }
    
    //Iteramos por cada curso en el catalogo para ir liberando su memoria
    for (int indiceCurso = 0; indiceCurso < cantidadCursos; indiceCurso++) {
        Curso *cursoActual = &arregloCursos[indiceCurso];
        liberarArregloStrings(cursoActual->requisitos, cursoActual->numRequisitos);
        liberarArregloStrings(cursoActual->correquisitos, cursoActual->numCorrequisitos);
        liberarArregloStrings(cursoActual->chocaCon, cursoActual->numChocaCon);
        
        //Recorremos los grupos de este curso para limpiar bloques y profesores
        for (int indiceGrupo = 0; indiceGrupo < cursoActual->numGrupos; indiceGrupo++) {
            Grupo *grupoActual = &cursoActual->grupos[indiceGrupo];
            liberarArregloStrings(grupoActual->bloquesHorario, grupoActual->numBloquesHorario);
            liberarArregloStrings(grupoActual->profesores, grupoActual->numProfesores);
        }
        
        free(cursoActual->grupos);
    }
    
    free(arregloCursos);
}