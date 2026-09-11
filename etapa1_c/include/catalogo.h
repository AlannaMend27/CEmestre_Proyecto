#ifndef CATALOGO_H
#define CATALOGO_H

#include "modelos.h"

// Aqui van las funciones para leer el JSON y convertirlo a los structs

// Lee el archivo JSON y devuelve un arreglo dinamico de tipo Curso
Curso *cargarCatalogo(const char *rutaArchivoJson, int *cantidadCursos);

// Libera toda la memoria dinamica reservada para el catalogo cuando ya no se ocupa
void liberarCatalogo(Curso *arregloCursos, int cantidadCursos);

#endif