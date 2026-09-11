#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "manejoArreglos.h"

// aqui van funciones de manejo de arreglos de strings

// Copia un string de forma segura hacia un buffer

void copiarStr(char *destino, size_t tamDestino, const char *origen) {
    if (origen == NULL) {
        destino[0] = '\0';
        return;
    }
    strncpy(destino, origen, tamDestino - 1);
    destino[tamDestino - 1] = '\0';
}

// Convierte un arreglo JSON de strings en un arreglo de strings dinamico
char **leerArregloStrings(const cJSON *arregloJson, int *n) {
    *n = 0;
    if (!cJSON_IsArray(arregloJson)) {
        return NULL;
    }

    int total = cJSON_GetArraySize(arregloJson);
    if (total == 0) {
        return NULL;
    }

    char **arreglo = malloc(sizeof(char *) * total);
    if (!arreglo) {
        fprintf(stderr, "Error de memoria en leerArregloStrings\n");
        exit(1);
    }

    int idx = 0;
    const cJSON *item;
    cJSON_ArrayForEach(item, arregloJson) {
        if (cJSON_IsString(item) && item->valuestring != NULL) {
            arreglo[idx] = malloc(strlen(item->valuestring) + 1);
            if (!arreglo[idx]) {
                fprintf(stderr, "Error de memoria en leerArregloStrings\n");
                exit(1);
            }
            strcpy(arreglo[idx], item->valuestring);
            idx++;
        }
    }

    *n = idx;
    return arreglo;
}

// libera el arreglo de strings el de (**char)
void liberarArregloStrings(char **arreglo, int n) {
    if (!arreglo) {
        return;
    }
    for (int i = 0; i < n; i++) {
        free(arreglo[i]);
    }
    free(arreglo);
}