#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "cJSON.h"

/* ---------- Definición de structs ---------- */

typedef struct {
    char  nombre[128];
    char  numero_grupo[16];

    char  **bloques_horario;   /* arreglo dinámico de strings */
    int   num_bloques_horario;

    char  **profesores;        /* arreglo dinámico de strings */
    int   num_profesores;

    char  tipo_grupo[32];
} Grupo;

typedef struct {
    char  codigo[16];
    char  nombre[160];
    int   creditos;

    char  **requisitos;
    int   num_requisitos;

    char  **correquisitos;
    int   num_correquisitos;

    Grupo *grupos;
    int   num_grupos;

    char  **choca_con;
    int   num_choca_con;

    bool  estudiante_puede_matricular;
} Curso;

/* ---------- Utilidades ---------- */

/* Copia un string de forma segura hacia un buffer de tamaño fijo */
static void copiar_str(char *destino, size_t tam_destino, const char *origen) {
    if (origen == NULL) {
        destino[0] = '\0';
        return;
    }
    strncpy(destino, origen, tam_destino - 1);
    destino[tam_destino - 1] = '\0';
}

/* Convierte un arreglo JSON de strings en un char** dinámico.
   Devuelve el arreglo y escribe la cantidad de elementos en *n */
static char **leer_arreglo_strings(const cJSON *arreglo_json, int *n) {
    *n = 0;
    if (!cJSON_IsArray(arreglo_json)) return NULL;

    int total = cJSON_GetArraySize(arreglo_json);
    if (total == 0) return NULL;

    char **arreglo = malloc(sizeof(char *) * total);
    if (!arreglo) { fprintf(stderr, "Error de memoria\n"); exit(1); }

    int idx = 0;
    const cJSON *item;
    cJSON_ArrayForEach(item, arreglo_json) {
        if (cJSON_IsString(item) && item->valuestring != NULL) {
            arreglo[idx] = malloc(strlen(item->valuestring) + 1);
            if (!arreglo[idx]) { fprintf(stderr, "Error de memoria\n"); exit(1); }
            strcpy(arreglo[idx], item->valuestring);
            idx++;
        }
    }
    *n = idx;
    return arreglo;
}

/* Libera un char** creado con leer_arreglo_strings */
static void liberar_arreglo_strings(char **arreglo, int n) {
    if (!arreglo) return;
    for (int i = 0; i < n; i++) free(arreglo[i]);
    free(arreglo);
}

/* ---------- Parseo de un grupo ---------- */

static void parsear_grupo(const cJSON *grupo_json, Grupo *g) {
    copiar_str(g->nombre, sizeof(g->nombre),
               cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(grupo_json, "nombre")));

    copiar_str(g->numero_grupo, sizeof(g->numero_grupo),
               cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(grupo_json, "numero_grupo")));

    copiar_str(g->tipo_grupo, sizeof(g->tipo_grupo),
               cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(grupo_json, "tipo_grupo")));

    const cJSON *bloques = cJSON_GetObjectItemCaseSensitive(grupo_json, "bloques_horario");
    g->bloques_horario = leer_arreglo_strings(bloques, &g->num_bloques_horario);

    const cJSON *profes = cJSON_GetObjectItemCaseSensitive(grupo_json, "profesores");
    g->profesores = leer_arreglo_strings(profes, &g->num_profesores);
}

/* ---------- Parseo de un curso ---------- */

static void parsear_curso(const cJSON *curso_json, Curso *c) {
    copiar_str(c->codigo, sizeof(c->codigo),
               cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(curso_json, "codigo")));

    copiar_str(c->nombre, sizeof(c->nombre),
               cJSON_GetStringValue(cJSON_GetObjectItemCaseSensitive(curso_json, "nombre")));

    const cJSON *creditos = cJSON_GetObjectItemCaseSensitive(curso_json, "creditos");
    c->creditos = cJSON_IsNumber(creditos) ? creditos->valueint : 0;

    const cJSON *req = cJSON_GetObjectItemCaseSensitive(curso_json, "requisitos");
    c->requisitos = leer_arreglo_strings(req, &c->num_requisitos);

    const cJSON *correq = cJSON_GetObjectItemCaseSensitive(curso_json, "correquisitos");
    c->correquisitos = leer_arreglo_strings(correq, &c->num_correquisitos);

    const cJSON *choca = cJSON_GetObjectItemCaseSensitive(curso_json, "choca_con");
    c->choca_con = leer_arreglo_strings(choca, &c->num_choca_con);

    const cJSON *puede_matricular = cJSON_GetObjectItemCaseSensitive(curso_json, "estudiante_puede_matricular");
    c->estudiante_puede_matricular = cJSON_IsTrue(puede_matricular);

    /* Grupos */
    const cJSON *grupos_json = cJSON_GetObjectItemCaseSensitive(curso_json, "grupos");
    if (cJSON_IsArray(grupos_json)) {
        c->num_grupos = cJSON_GetArraySize(grupos_json);
        if (c->num_grupos > 0) {
            c->grupos = malloc(sizeof(Grupo) * c->num_grupos);
            if (!c->grupos) { fprintf(stderr, "Error de memoria\n"); exit(1); }

            int idx = 0;
            const cJSON *grupo_item;
            cJSON_ArrayForEach(grupo_item, grupos_json) {
                parsear_grupo(grupo_item, &c->grupos[idx]);
                idx++;
            }
        } else {
            c->grupos = NULL;
        }
    } else {
        c->grupos = NULL;
        c->num_grupos = 0;
    }
}

/* ---------- Carga del catálogo completo ---------- */

/* Lee el archivo JSON y devuelve un arreglo dinámico de Curso.
   Escribe la cantidad de cursos en *num_cursos.
   Devuelve NULL si algo falla. */
Curso *cargar_catalogo(const char *ruta_archivo, int *num_cursos) {
    *num_cursos = 0;

    FILE *f = fopen(ruta_archivo, "rb");
    if (!f) {
        fprintf(stderr, "No se pudo abrir el archivo: %s\n", ruta_archivo);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long tam = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buffer = malloc(tam + 1);
    if (!buffer) { fclose(f); fprintf(stderr, "Error de memoria\n"); return NULL; }

    fread(buffer, 1, tam, f);
    buffer[tam] = '\0';
    fclose(f);

    cJSON *raiz = cJSON_Parse(buffer);
    free(buffer);

    if (!raiz) {
        const char *error_ptr = cJSON_GetErrorPtr();
        fprintf(stderr, "Error de parseo JSON antes de: %s\n", error_ptr ? error_ptr : "desconocido");
        return NULL;
    }

    if (!cJSON_IsArray(raiz)) {
        fprintf(stderr, "El JSON raíz no es un arreglo\n");
        cJSON_Delete(raiz);
        return NULL;
    }

    int total = cJSON_GetArraySize(raiz);
    Curso *cursos = malloc(sizeof(Curso) * total);
    if (!cursos) {
        fprintf(stderr, "Error de memoria\n");
        cJSON_Delete(raiz);
        return NULL;
    }

    int idx = 0;
    const cJSON *curso_item;
    cJSON_ArrayForEach(curso_item, raiz) {
        parsear_curso(curso_item, &cursos[idx]);
        idx++;
    }

    cJSON_Delete(raiz);
    *num_cursos = total;
    return cursos;
}

/* ---------- Liberación de memoria ---------- */

void liberar_catalogo(Curso *cursos, int num_cursos) {
    if (!cursos) return;

    for (int i = 0; i < num_cursos; i++) {
        Curso *c = &cursos[i];

        liberar_arreglo_strings(c->requisitos, c->num_requisitos);
        liberar_arreglo_strings(c->correquisitos, c->num_correquisitos);
        liberar_arreglo_strings(c->choca_con, c->num_choca_con);

        for (int j = 0; j < c->num_grupos; j++) {
            Grupo *g = &c->grupos[j];
            liberar_arreglo_strings(g->bloques_horario, g->num_bloques_horario);
            liberar_arreglo_strings(g->profesores, g->num_profesores);
        }
        free(c->grupos);
    }
    free(cursos);
}

/* ---------- Ejemplo de uso / impresión ---------- */

static void imprimir_curso(const Curso *c) {
    printf("Codigo: %s\n", c->codigo);
    printf("Nombre: %s\n", c->nombre);
    printf("Creditos: %d\n", c->creditos);
    printf("Puede matricular: %s\n", c->estudiante_puede_matricular ? "si" : "no");

    printf("Requisitos (%d): ", c->num_requisitos);
    for (int i = 0; i < c->num_requisitos; i++) printf("%s ", c->requisitos[i]);
    printf("\n");

    printf("Correquisitos (%d): ", c->num_correquisitos);
    for (int i = 0; i < c->num_correquisitos; i++) printf("%s ", c->correquisitos[i]);
    printf("\n");

    printf("Grupos (%d):\n", c->num_grupos);
    for (int i = 0; i < c->num_grupos; i++) {
        const Grupo *g = &c->grupos[i];
        printf("  - Grupo %s (%s) [%s]\n", g->numero_grupo, g->nombre, g->tipo_grupo);
        printf("    Horarios: ");
        for (int k = 0; k < g->num_bloques_horario; k++) printf("%s | ", g->bloques_horario[k]);
        printf("\n    Profesores: ");
        for (int k = 0; k < g->num_profesores; k++) printf("%s | ", g->profesores[k]);
        printf("\n");
    }
    printf("--------------------------------------------------\n");
}
/*-----Busca Curso por codigo------*/
char buscar(char *codigo_curso, Curso *cursos, int num_cursos) {
    for (int i = 0; i < num_cursos; i++) {
        if (strcmp(cursos[i].codigo, codigo_curso) == 0) {
            printf("\nCurso encontrado (%s):\n", codigo_curso);
            imprimir_curso(&cursos[i]);
            break;
        } 
    
}
}

Curso **obtener_cursos_disponibles(Curso *cursos, int num_cursos,
                                    char **cursos_aprobados, int num_aprobados,
                                    int *cantidad_out) {
 
    Curso **cursos_disponibles = malloc(sizeof(Curso *) * num_cursos);
    int cantidad = 0;
 
    for (int i = 0; i < num_cursos; i++) {
 
        /* Si el curso ya fue aprobado, no lo ofrecemos de nuevo */
        bool ya_aprobado = false;
        for (int j = 0; j < num_aprobados; j++) {
            if (strcmp(cursos[i].codigo, cursos_aprobados[j]) == 0) {
                ya_aprobado = true;
                break;
            }
        }
        if (ya_aprobado) {
            continue;
        }
 
        /* Revisa si TODOS los requisitos del curso están aprobados */
        bool requisitos_cumplidos = true;
 
        for (int k = 0; k < cursos[i].num_requisitos; k++) {
            bool requisito_aprobado = false;
 
            for (int l = 0; l < num_aprobados; l++) {
                if (strcmp(cursos[i].requisitos[k], cursos_aprobados[l]) == 0) {
                    requisito_aprobado = true;
                    break;
                }
            }
 
            if (!requisito_aprobado) {
                requisitos_cumplidos = false;
                break;
            }
        }
 
        if (requisitos_cumplidos) {
            cursos_disponibles[cantidad] = &cursos[i];
            cantidad++;
        }
    }
 
    *cantidad_out = cantidad;
    return cursos_disponibles;
}


int main(int argc, char *argv[]) {
    const char *ruta = (argc > 1) ? argv[1] : "ArchivosEntrada/info_cursos/ingComputadores/catalogo_cursos_computadores.json";
 
    int num_cursos = 0;
    char *cursos_aprobados[] = {"MA0101", "CE1101", "CE1104", "MA1102", "MA1403"};
    int num_aprobados = 5;
 
    Curso *cursos = cargar_catalogo(ruta, &num_cursos);
 
    if (!cursos) {
        fprintf(stderr, "No se pudo cargar el catálogo.\n");
        return 1;
    }
 
    printf("Se cargaron %d cursos.\n\n", num_cursos);
 
    int cantidad_cursos_disponibles = 0;
    Curso **cursos_disponibles = obtener_cursos_disponibles(
        cursos, num_cursos,
        cursos_aprobados, num_aprobados,
        &cantidad_cursos_disponibles
    );
 
    printf("Cursos disponibles: %d\n", cantidad_cursos_disponibles);
    for (int i = 0; i < cantidad_cursos_disponibles; i++) {
        printf("%s - %s\n", cursos_disponibles[i]->codigo, cursos_disponibles[i]->nombre);
    }
 
    free(cursos_disponibles);
    liberar_catalogo(cursos, num_cursos);
    return 0;
}
