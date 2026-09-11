#ifndef MANEJO_ARREGLOS_H
#define MANEJO_ARREGLOS_H
#include <stddef.h>
#include "cJSON.h"

//En este archivo van funciones de manejo de arreglos de strings

//Esta funcion copia un string a un buffer de tamano fijo de forma segura
void copiarStr(char *destino, size_t tamDestino, const char *origen);

//Convierte un arreglo JSON de strings en un arreglo dinamico de strings
char **leerArregloStrings(const cJSON *arregloJson, int *cantidadSalida);

//Libera toda la memoria reservada para el arreglo dinamico de strings
void liberarArregloStrings(char **arregloStrings, int cantidadElementos);

#endif