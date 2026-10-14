#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "catalogo.h"
#include "exportar.h"
#include "manejoArreglos.h"

// Serializar el catalogo ya procesado a un archivo JSON de salida.

// Convierte un struct Grupo a un objeto JSON de cJSON
static cJSON *grupoAJson(const Grupo *grupo) {

    // crear objeto para almacenar la informacion del grupo
    cJSON *grupoJson = cJSON_CreateObject();

    cJSON_AddStringToObject(grupoJson, "numero_grupo", grupo->numeroGrupo);
    cJSON_AddStringToObject(grupoJson, "nombre",       grupo->nombre);
    cJSON_AddStringToObject(grupoJson, "tipo_grupo",   grupo->tipoGrupo);

    cJSON_AddItemToObject(grupoJson, "profesores",
        arregloStringsAJson(grupo->profesores, grupo->numProfesores));

    cJSON_AddItemToObject(grupoJson, "bloques_horario",
        arregloStringsAJson(grupo->bloquesHorario, grupo->numBloquesHorario));

    //Cada choque se exporta con el curso y el grupo con el que choca
    cJSON *chocaConJson = cJSON_CreateArray();
    for (int indiceChoque = 0; indiceChoque < grupo->numChocaCon; indiceChoque++) {
        cJSON *choqueJson = cJSON_CreateObject();
        cJSON_AddStringToObject(choqueJson, "curso", grupo->chocaCon[indiceChoque].codigoCurso);
        cJSON_AddNumberToObject(choqueJson, "id_grupo", grupo->chocaCon[indiceChoque].idGrupo);
        cJSON_AddItemToArray(chocaConJson, choqueJson);
    }
    cJSON_AddItemToObject(grupoJson, "choca_con", chocaConJson);

    cJSON_AddNumberToObject(grupoJson, "id_grupo", grupo->idGrupo);
    return grupoJson;
}

// Convierte un struct Curso completo a JSON
static cJSON *cursoAJson(const Curso *curso) {

    // crear cuerpo principal del Json
    cJSON *cursoJson = cJSON_CreateObject();

    // crear propiedades del jasony asignarle el valor de los datos del struct curso recibido
    cJSON_AddStringToObject(cursoJson, "codigo", curso->codigo);
    cJSON_AddStringToObject(cursoJson, "nombre", curso->nombre);
    cJSON_AddNumberToObject(cursoJson, "creditos",curso->creditos);

    //en este caso es un arreglo
    cJSON_AddItemToObject(cursoJson, "requisitos", arregloStringsAJson(curso->requisitos, curso->numRequisitos));

    cJSON_AddItemToObject(cursoJson, "correquisitos", arregloStringsAJson(curso->correquisitos, curso->numCorrequisitos));

    // arreglo de las carreras
    cJSON *arregloCarreras = cJSON_CreateArray();
    for (int i = 0; i < curso->numCarreras; i++) {
        cJSON_AddItemToArray(arregloCarreras, cJSON_CreateString(curso->carreras[i]));
    }
    cJSON_AddItemToObject(cursoJson, "carreras", arregloCarreras);

    // true/false segun si el estudiante cumple los requisitos
    cJSON_AddBoolToObject(cursoJson, "estudiante_puede_matricular", curso->estudiantePuedeMatricular);

    // Arreglo de grupos con sus horarios y profesores
    cJSON *gruposJson = cJSON_CreateArray();

    // se recorre cada grupo
    for (int i = 0; i < curso->numGrupos; i++) {
        // procesar la informaci[on del grupo]
        cJSON_AddItemToArray(gruposJson, grupoAJson(&curso->grupos[i]));
    }

    // se almacena como objeto del curso en el json
    cJSON_AddItemToObject(cursoJson, "grupos", gruposJson);

    return cursoJson;
}

// Recorre el catalogo completo, construye el JSON y lo escribe en el archivo de salida
int exportarCatalogo(const Curso *arregloCursos, int cantidadCursos, const char *rutaArchivoSalida, const char *carreraEstudiante) {

    // Creamos el arreglo JSON raiz que va a contener todos los cursos
    cJSON *raiz = cJSON_CreateArray();

    // verificar que se haya creado el arreglo
    if (!raiz) {
        fprintf(stderr, "Error al crear el arreglo JSON raiz en exportarCatalogo\n");
        return 0;
    }

    // almacenar cada curso en el archivo JSON
    for (int i = 0; i < cantidadCursos; i++) {
        int esValido = 0;
            for (int j = 0; j < arregloCursos[i].numCarreras; j++) {

                if (strcmp(arregloCursos[i].carreras[j], carreraEstudiante) == 0) {
                    
                    esValido = 1;
                    break;
                }
            }

            // si el curso es del plan de estudios del estudiante, 
            if (esValido) {
                cJSON_AddItemToArray(raiz, cursoAJson(&arregloCursos[i]));
            }
    }

    // serealizar el objeto JSON a texto con formato legible 
    char *textoJson = cJSON_Print(raiz);
    cJSON_Delete(raiz);

    if (!textoJson) {
        fprintf(stderr, "Error al serializar el JSON en exportarCatalogo\n");
        return 0;
    }

    // Abrimos el archivo de salida para escribir
    FILE *archivoSalida = fopen(rutaArchivoSalida, "w");

    // verificar que es posible abrir el archivo
    if (!archivoSalida) {
        fprintf(stderr, "No se pudo abrir el archivo de salida: %s\n", rutaArchivoSalida);
        free(textoJson);
        return 0;
    }

    // Escribimos el texto
    fputs(textoJson, archivoSalida);

    // cerrar archivo y liberar memoria
    fclose(archivoSalida);
    free(textoJson);

    printf("Catalogo exportado exitosamente\n");
    return 1;
}