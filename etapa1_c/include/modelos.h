#ifndef MODELOS_H
#define MODELOS_H

#include <stdbool.h>
#include "constantes.h"

//Este archivo es para representar los cursos en structs

//Representa un grupo de un curso con su horario y profes
typedef struct {
    char nombre[TAM_NOMBRE_GRUPO];
    char numeroGrupo[TAM_NUMERO_GRUPO];
    char tipoGrupo[TAM_TIPO_GRUPO];
    char  **bloquesHorario;
    int   numBloquesHorario;
    char  **profesores;
    int   numProfesores;

} Grupo; // esto es para no tener que escribir struct Grupo a cada rato

//Este struct representa un curso con todos sus datos y grupos disponibles
typedef struct {
    char codigo[TAM_CODIGO_CURSO];
    char nombre[TAM_NOMBRE_CURSO];
    int creditos;

    char **requisitos; // esto es un arreglo dinamico de strings
    int numRequisitos;

    char **correquisitos;
    int numCorrequisitos;

    Grupo *grupos;
    int numGrupos;

    char  **chocaCon;
    int numChocaCon;

    bool  estudiantePuedeMatricular;
} Curso; //igual que arriba

#endif