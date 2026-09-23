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

//Agrega un codigo de curso a la lista chocaCon de otro curso especifico
void agregarChoque(Curso *cursoActual, const char *codigoQueChoca);

//Recorre todos los pares de cursos del catalogo y llena el arreglo chocaCon de cada uno
void detectarChoques(Curso *arregloCursos, int cantidadCursos);

#endif