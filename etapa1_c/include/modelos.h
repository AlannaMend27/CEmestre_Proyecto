#ifndef MODELOS_H
#define MODELOS_H
#include <stdbool.h>
#include "constantes.h"

//Este archivo define las estructuras principales para representar los cursos

typedef struct {
//Representa un bloque de horario ya parseado
    char dia[4];  
    int inicioMin;  //Hora de inicio en minutos desde medianoche
    int finMin;

} BloqueHorario; 


typedef struct {
//Representa un grupo especifico de un curso con su horario y profesores
    char nombre[TAM_NOMBRE_GRUPO];
    char numeroGrupo[TAM_NUMERO_GRUPO];
    char tipoGrupo[TAM_TIPO_GRUPO];
    char **bloquesHorario;
    int numBloquesHorario;
    char **profesores;
    int numProfesores;
} Grupo; 


typedef struct {
//Representa un curso completo con todos sus datos, requisitos y grupos disponibles
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

} Curso; 

typedef struct {

 //Representa el historial de un estudiante y la carrera a la que pertenece
    char carrera[32];
    char **aprobados; // Arreglo dinamico de codigos de cursos aprobados
    int cantidadAprobados;  

} Historial;
 
#endif