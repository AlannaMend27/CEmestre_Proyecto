#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "manejoArreglos.h"

//Aqui van funciones de manejo de arreglos de strings

//Copia un string hacia un buffer de destino evitando desbordamientos
void copiarStr(char *destino, size_t tamDestino, const char *origen) {
    //Si el origen viene nulo dejamos el destino vacio y salimos
    if (origen == NULL) {
        destino[0] = '\0';
        return;
    }
    
    strncpy(destino, origen, tamDestino - 1);
    destino[tamDestino - 1] = '\0';
}

//Convierte un arreglo JSON de strings en un arreglo de strings dinamico en memoria
char **leerArregloStrings(const cJSON *arregloJson, int *cantidadSalida) {
    *cantidadSalida = 0;
    
    //Verificamos que el elemento recibido sea realmente un arreglo en el json
    if (!cJSON_IsArray(arregloJson)) {
        return NULL;
    }

    int totalElementos = cJSON_GetArraySize(arregloJson);
    
    //Si el arreglo esta vacio retornamos nulo
    if (totalElementos == 0) {
        return NULL;
    }

    char **arregloDinamico = malloc(sizeof(char *) * totalElementos);
    
    //Si falla la reserva de memoria reportamos el error y detenemos el programa
    if (!arregloDinamico) {
        fprintf(stderr, "Error de memoria en leerArregloStrings\n");
        exit(1);
    }

    //Declaramos la variable para el indice y el puntero temporal del item json
    int indiceElemento = 0;
    const cJSON *itemJson;
    
    //Recorremos cada elemento del arreglo json para extraer sus valores de texto
    cJSON_ArrayForEach(itemJson, arregloJson) {
        if (cJSON_IsString(itemJson) && itemJson->valuestring != NULL) {
            arregloDinamico[indiceElemento] = malloc(strlen(itemJson->valuestring) + 1);
            
            //Validamos la memoria para el string individual
            if (!arregloDinamico[indiceElemento]) {
                fprintf(stderr, "Error de memoria en leerArregloStrings\n");
                exit(1);
            }
            
            //Copiamos el texto del item al arreglo dinamico y avanzamos el indice
            strcpy(arregloDinamico[indiceElemento], itemJson->valuestring);
            indiceElemento++;
        }
    }

    //Guardamos la cantidad total de elementos procesados y retornamos el arreglo dinamico
    *cantidadSalida = indiceElemento;
    return arregloDinamico;
}

//Libera por completo el arreglo dinamico de strings y cada uno de sus elementos internos
void liberarArregloStrings(char **arregloStrings, int cantidadElementos) {
    //Si el arreglo es nulo salimos de inmediato
    if (!arregloStrings) {
        return;
    }
    
    //Recorremos cada posicion del arreglo para liberar la memoria del string contenido
    for (int indiceElemento = 0; indiceElemento < cantidadElementos; indiceElemento++) {
        free(arregloStrings[indiceElemento]);
    }
    
    free(arregloStrings);
}