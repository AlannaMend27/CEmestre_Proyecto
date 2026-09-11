#ifndef CATALOGO_H
#define CATALOGO_H

#include "modelos.h"

// aqui van los metodos que tienen que ver con leer el json y convertirlo en structs de c

//Lee el archivo JSON, devuelve un arreglo dinamico de Curso y escribe la cantidad de numcursos
Curso *cargarCatalogo(const char *rutaArchivo, int *numCursos);

// Libera toda la memoria del catalogo cargado 
void liberarCatalogo(Curso *cursos, int numCursos);

#endif