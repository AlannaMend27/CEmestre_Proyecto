#ifndef MANEJO_ARREGLOS_H
#define MANEJO_ARREGLOS_H
#include <stddef.h>
#include "cJSON.h"

// en este archivp van funciones de manejo de arreglos de strings

// esta funcion copiua un string a un buffer de tamaño fijo
void copiarStr(char *destino, size_t tamDestino, const char *origen);

//Convierte un arreglo JSON de strings en un arreglo de strings dinamico
char **leerArregloStrings(const cJSON *arregloJson, int *n);

// libera el arreglo de strings
void liberarArregloStrings(char **arreglo, int n);

#endif