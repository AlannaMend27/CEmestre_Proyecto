#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include "catalogo.h"
#include "manejoArreglos.h"

//este archivo escribe el catalogo ya pricesado a un archivo de entrada

// Rellena un struct Grupo a partir de su objeto JSON 
static void parsearGrupo(const cJSON *grupoJson, Grupo *g) {
    copiarStr(g->nombre, sizeof(g->nombre), 
    cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(grupoJson, "nombre")));

    copiarStr(g->numeroGrupo, sizeof(g->numeroGrupo),
        cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(grupoJson, "numero_grupo")));

    copiarStr(g->tipoGrupo, sizeof(g->tipoGrupo),
        cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(grupoJson, "tipo_grupo")));

    const cJSON *bloques = cJSON_GetObjectItemCaseSensitive(grupoJson, "bloques_horario");
    g->bloquesHorario = leerArregloStrings(bloques, &g->numBloquesHorario);

    const cJSON *profes = cJSON_GetObjectItemCaseSensitive(grupoJson, "profesores");
    g->profesores = leerArregloStrings(profes, &g->numProfesores);
}

// Rellena un struct Curso a partir de su objeto JSON 
static void parsearCurso(const cJSON *cursoJson, Curso *c) {
    copiarStr(c->codigo, sizeof(c->codigo),
        cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(cursoJson, "codigo")));

    copiarStr(c->nombre, sizeof(c->nombre),
        cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(cursoJson, "nombre")));

    const cJSON *creditos = cJSON_GetObjectItemCaseSensitive(cursoJson, "creditos");
    if (cJSON_IsNumber(creditos)) {
        c->creditos = creditos->valueint;
    } else {
        c->creditos = 0;
    }

    const cJSON *req = cJSON_GetObjectItemCaseSensitive(cursoJson, "requisitos");
    c->requisitos = leerArregloStrings(req, &c->numRequisitos);

    const cJSON *correq = cJSON_GetObjectItemCaseSensitive(cursoJson, "correquisitos");
    c->correquisitos = leerArregloStrings(correq, &c->numCorrequisitos);

    const cJSON *choca = cJSON_GetObjectItemCaseSensitive(cursoJson, "choca_con");
    c->chocaCon = leerArregloStrings(choca, &c->numChocaCon);

    const cJSON *puedeMatricular = cJSON_GetObjectItemCaseSensitive(cursoJson, "estudiante_puede_matricular");
    c->estudiantePuedeMatricular = cJSON_IsTrue(puedeMatricular);

    // Parsea los grupos del curso 
    const cJSON *gruposJson = cJSON_GetObjectItemCaseSensitive(cursoJson, "grupos");
    if (cJSON_IsArray(gruposJson)) {
        c->numGrupos = cJSON_GetArraySize(gruposJson);
        if (c->numGrupos > 0) {
            c->grupos = malloc(sizeof(Grupo) * c->numGrupos);
            if (!c->grupos) {
                fprintf(stderr, "Error de memoria en parsearCurso\n");
                exit(1);
            }
            int idx = 0;
            const cJSON *grupoItem;
            cJSON_ArrayForEach(grupoItem, gruposJson) {
                parsearGrupo(grupoItem, &c->grupos[idx]);
                idx++;
            }
        } else {
            c->grupos = NULL;
        }
    } else {
        c->grupos = NULL;
        c->numGrupos = 0;
    }
}

// Lee el archivo JSON y devuelve un arreglo dinamico de Curso 
Curso *cargarCatalogo(const char *rutaArchivo, int *numCursos) {
    *numCursos = 0;

    FILE *f = fopen(rutaArchivo, "rb");
    if (!f) {
        fprintf(stderr, "No se pudo abrir el archivo: %s\n", rutaArchivo);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long tam = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buffer = malloc(tam + 1);
    if (!buffer) {
        fclose(f);
        fprintf(stderr, "Error de memoria en cargarCatalogo\n");
        return NULL;
    }

    fread(buffer, 1, tam, f);
    buffer[tam] = '\0';
    fclose(f);

    cJSON *raiz = cJSON_Parse(buffer);
    free(buffer);

    if (!raiz) {
        const char *errorPtr = cJSON_GetErrorPtr();
        fprintf(stderr, "Error al parsear JSON cerca de: %s\n",
            errorPtr ? errorPtr : "desconocido");
        return NULL;
    }

    if (!cJSON_IsArray(raiz)) {
        fprintf(stderr, "El JSON raiz no es un arreglo\n");
        cJSON_Delete(raiz);
        return NULL;
    }

    int total = cJSON_GetArraySize(raiz);
    Curso *cursos = malloc(sizeof(Curso) * total);
    if (!cursos) {
        fprintf(stderr, "Error de memoria en cargarCatalogo\n");
        cJSON_Delete(raiz);
        return NULL;
    }

    int idx = 0;
    const cJSON *cursoItem;
    cJSON_ArrayForEach(cursoItem, raiz) {
        parsearCurso(cursoItem, &cursos[idx]);
        idx++;
    }

    cJSON_Delete(raiz);
    *numCursos = total;
    return cursos;
}

// Libera toda la memoria del catalogo 
void liberarCatalogo(Curso *cursos, int numCursos) {
    if (!cursos) {
        return;
    }
    for (int i = 0; i < numCursos; i++) {
        Curso *c = &cursos[i];
        liberarArregloStrings(c->requisitos, c->numRequisitos);
        liberarArregloStrings(c->correquisitos, c->numCorrequisitos);
        liberarArregloStrings(c->chocaCon, c->numChocaCon);
        for (int j = 0; j < c->numGrupos; j++) {
            Grupo *g = &c->grupos[j];
            liberarArregloStrings(g->bloquesHorario, g->numBloquesHorario);
            liberarArregloStrings(g->profesores, g->numProfesores);
        }
        free(c->grupos);
    }
    free(cursos);
}