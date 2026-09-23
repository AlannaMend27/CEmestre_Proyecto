#ifndef HISTORIAL_H
#define HISTORIAL_H
#include "modelos.h"
#include "constantes.h"

//Este archivo es el encargado de leer el historial del estudiante

//Devuelve un puntero a Historial con todos los datos cargados
Historial *cargarHistorial(const char *rutaArchivo);

//Libera toda la memoria del historial cargado con cargarHistorial
void liberarHistorial(Historial *historial);

#endif