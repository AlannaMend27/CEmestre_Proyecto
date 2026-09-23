#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "historial.h"

//Este archivo se encarga de leer el archivo de entrada del estudiante

//Elimina el salto de linea al final de un string por si existe
static void limpiarLineaFinal(char *lineaActual) {
    int longitudLinea = strlen(lineaActual);

    //Revisamos si el ultimo caracter es un salto de linea 
    if (longitudLinea > 0 && lineaActual[longitudLinea - 1] == '\n') {
        lineaActual[longitudLinea - 1] = '\0';
        longitudLinea--;
    }

    //Revisamos tambien si existe un retorno de carro
    if (longitudLinea > 0 && lineaActual[longitudLinea - 1] == '\r') {
        lineaActual[longitudLinea - 1] = '\0';
    }
}

//Lee el archivo de historial y devuelve un struct Historial con carrera y codigos aprobados
Historial *cargarHistorial(const char *rutaArchivo) {

    //Abrimos el archivo en modo lectura 
    FILE *archivoHistorial = fopen(rutaArchivo, "r");
    
    //Verificamos si hubo un problema al intentar abrir el archivo de texto
    if (!archivoHistorial) {
        fprintf(stderr, "No se pudo abrir el historial: %s\n", rutaArchivo);
        return NULL;
    }

    Historial *historial = malloc(sizeof(Historial));

    //Verificamos si fallo la asignacion de memoria para la estructura principal
    if (!historial) {
        fprintf(stderr, "Error de memoria en cargarHistorial\n");
        fclose(archivoHistorial);
        return NULL;
    }

    //Inicializamos los valores en cero y declaramos el buffer temporal 
    historial->cantidadAprobados = 0;
    char lineaActual[TAM_LINEA_HISTORIAL];

    //Leemos la primera linea que corresponde al nombre de la carrera del estudiante
    if (fgets(lineaActual, sizeof(lineaActual), archivoHistorial) == NULL) {
        fprintf(stderr, "El archivo de historial esta vacio: %s\n", rutaArchivo);
        free(historial);
        fclose(archivoHistorial);
        return NULL;
    }

    //Limpiamos el texto y lo copiamos a la variable de carrera
    limpiarLineaFinal(lineaActual);
    strncpy(historial->carrera, lineaActual, sizeof(historial->carrera) - 1);
    //Forzamos poner un caracter nulo al final
    historial->carrera[sizeof(historial->carrera) - 1] = '\0';

    int cantidadLineas = 0;
    
    //Realizamos una primera pasada para contar cuantos cursos hay registrados
    while (fgets(lineaActual, sizeof(lineaActual), archivoHistorial) != NULL) {
        limpiarLineaFinal(lineaActual);
        
        //Solo contamos las lineas que realmente tengan texto y no esten vacias
        if (strlen(lineaActual) > 0) {
            cantidadLineas++;
        }
    }

    //Si no hay cursos aprobados retornamos el historial solo con la carrera cargada
    if (cantidadLineas == 0) {
        fclose(archivoHistorial);
        return historial;
    }

    //Reservamos memoria para donde estaran los cursos aprobados del historial
    historial->aprobados = malloc(sizeof(char *) * cantidadLineas);
    
    //Validamos que se haya podido reservar la memoria para el arreglo de strings
    if (!historial->aprobados) {
        fprintf(stderr, "Error de memoria en cargarHistorial\n");
        free(historial);
        fclose(archivoHistorial);
        return NULL;
    }

    //Devolvemos el puntero al inicio del archivo y saltamos la linea de la carrera
    rewind(archivoHistorial);
    fgets(lineaActual, sizeof(lineaActual), archivoHistorial);

    int indiceAprobado = 0;
    
    //Leemos nuevamente el archivo linea por linea para guardar cada codigo aprobado
    while (fgets(lineaActual, sizeof(lineaActual), archivoHistorial) != NULL) {
        limpiarLineaFinal(lineaActual);

        //Si la linea tiene contenido reservamos la memoria exacta y copiamos el texto
        if (strlen(lineaActual) > 0) {
            historial->aprobados[indiceAprobado] = malloc(strlen(lineaActual) + 1);
            
            //Si falla la reserva para un codigo especifico detenemos el proceso
            if (!historial->aprobados[indiceAprobado]) {
                fprintf(stderr, "Error de memoria en cargarHistorial\n");
                fclose(archivoHistorial);
                return NULL;
            }
            
            //Copiamos el codigo del curso al arreglo
            strcpy(historial->aprobados[indiceAprobado], lineaActual);
            indiceAprobado++;
        }
    }

    //Cerramos el archivo y guardamos el total de cursos aprobados
    historial->cantidadAprobados = indiceAprobado;
    return historial;
}

//Libera toda la memoria del historial
void liberarHistorial(Historial *historial) {
    //Si el puntero es nulo salimos para evitar errores de memoria
    if (!historial) {
        return;
    }

    //Recorremos todos los elementos del arreglo para liberar cada string
    for (int indiceAprobado = 0; indiceAprobado < historial->cantidadAprobados; indiceAprobado++) {
        free(historial->aprobados[indiceAprobado]);
    }

    free(historial->aprobados);
    free(historial);
}