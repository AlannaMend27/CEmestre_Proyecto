#ifndef HORARIO_H
#define HORARIO_H
#include "modelos.h"

//Aqui van las comparaciones de bloques de horario entre grupos

//Convierte un string tipo MAR[07:30-09:20] en un BloqueHorario
int parsearBloqueHorario(const char *textoHorario, BloqueHorario *bloqueParseado);

//Compara dos bloques ya parseados para ver si chocan
int bloquesChocan(const BloqueHorario *primerBloque, const BloqueHorario *segundoBloque);

//Compara todos los bloques de dos grupos para revisar si hay conflictos
int gruposChocan(const Grupo *primerGrupo, const Grupo *segundoGrupo);

//Agrega un par curso-grupo a la lista chocaCon de un grupo especifico
void agregarChoque(Grupo *grupoActual, const char *codigoCursoQueChoca, int idGrupoQueChoca);

//Recorre todos los pares de grupos de cursos distintos y llena el chocaCon de cada grupo
void detectarChoques(Curso *arregloCursos, int cantidadCursos, const char *carreraEstudiante);

#endif