#ifndef EXPORTAR_H
#define EXPORTAR_H

#include "catalogo.h"
#include "manejoArreglos.h"

// Exporta el catalogo procesado (con choques y matriculabilidad ya calculados) a un archivo JSON en la ruta indicada.

int exportarCatalogo(const Curso *arregloCursos, int cantidadCursos, const char *rutaArchivoSalida);

#endif