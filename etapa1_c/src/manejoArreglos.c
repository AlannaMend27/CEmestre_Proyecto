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
    
    //copiamos el texto de origen en el destino
    strncpy(destino, origen, tamDestino - 1);
    //dejando un espacio para el caracter final
    destino[tamDestino - 1] = '\0';
}

//Convierte un arreglo JSON de strings en un arreglo de strings dinamico en memoria
char **leerArregloStrings(const cJSON *arregloJson, int *cantidadSalida) {
    
    *cantidadSalida = 0;
    
    //Verificamos que el elemento recibido sea realmente un arreglo en el json
    if (!cJSON_IsArray(arregloJson)) {
        return NULL;
    }

    //obtenemos el total de elementos del arreglo que esta en el json
    int totalElementos = cJSON_GetArraySize(arregloJson);
    
    //Si el arreglo esta vacio retornamos nulo
    if (totalElementos == 0) {
        return NULL;
    }

    //reservamos memoria justa para esa cantidad de elementos
    char **arregloDinamico = malloc(sizeof(char *) * totalElementos);
    
    //Si falla la reserva de memoria reportamos el error y detenemos el programa
    if (!arregloDinamico) {
        fprintf(stderr, "Error de memoria en leerArregloStrings\n");
        exit(1);
    }

    //Un contador para saber donde escribir y un puntero de reccrrido
    int indiceElemento = 0;
    const cJSON *itemJson;
    
    //Para cada elemento el arreglo json
    cJSON_ArrayForEach(itemJson, arregloJson) {
        //Verificamos que esto sea un string valido
        if (cJSON_IsString(itemJson) && itemJson->valuestring != NULL) {

            //Reservamos memoria del tamanio del texto
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

// Convierte un arreglo de strings de C en un arreglo JSON de cJSON
cJSON *arregloStringsAJson(char **arreglo, int cantidad) {
    cJSON *arregloJson = cJSON_CreateArray();

    // Si el arreglo viene nulo o vacio devolvemos un arreglo JSON vacio de todas formas
    if (!arreglo || cantidad == 0) {
        return arregloJson;
    }

    // agregar elemento si no es un string nulo
    for (int i = 0; i < cantidad; i++) {
        if (arreglo[i] != NULL) {
            cJSON_AddItemToArray(arregloJson, cJSON_CreateString(arreglo[i]));
        }
    }

    return arregloJson;
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