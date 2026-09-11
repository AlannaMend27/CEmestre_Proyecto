#ifndef MODELOS_H
#define MODELOS_H

#include <stdbool.h>
#include "constantes.h"

//Este archivo define las estructuras principales para representar los cursos

//Representa un bloque de horario ya parseado
typedef struct {
    char dia[4];  
    int inicioMin; //Hora de inicio en minutos desde medianoche, ej: 07:30 -> 450
    int finMin; //igual

    //NOTA: la idea de usar ints es para comparar numeros y no strings directamente clq cosa

} BloqueHorario; //Evita tener que escribir struct BloqueHorario a cada rato

//Representa un grupo especifico de un curso con su horario y profesores
typedef struct {
    char nombre[TAM_NOMBRE_GRUPO];
    char numeroGrupo[TAM_NUMERO_GRUPO];
    char tipoGrupo[TAM_TIPO_GRUPO];
    char **bloquesHorario;
    int numBloquesHorario;
    char **profesores;
    int numProfesores;
} Grupo; // typedef es para evitar tener que escribir struct Grupo a cada rato

//Representa un curso completo con todos sus datos, requisitos y grupos disponibles
typedef struct {
    char codigo[TAM_CODIGO_CURSO];
    char nombre[TAM_NOMBRE_CURSO];
    int creditos;

    char **requisitos; //Arreglo dinamico con los codigos de los requisitos
    int numRequisitos;

    char **correquisitos; //Arreglo dinamico con los codigos de los correquisitos
    int numCorrequisitos;

    Grupo *grupos;
    int numGrupos;

    char **chocaCon; //Arreglo dinamico con los codigos de los cursos con los que choca
    int numChocaCon;

    bool estudiantePuedeMatricular;
} Curso; //igual

#endif