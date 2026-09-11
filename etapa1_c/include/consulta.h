#ifndef CONSULTA_H
#define CONSULTA_H

#include "modelos.h"

/*aqui va la logica de preguntarle cosas al catalogo 
ya cargado, tipo preguntarle que cuales puede matricular y asi
*/

//Imprime todos los datos de un curso en consola 
void imprimirCurso(const Curso *c);

// Busca un curso por codigo e imprime sus datos si lo encuentra 
void buscar(const char *codigoCurso, Curso *cursos, int numCursos);

/* Devuelve un arreglo de punteros a los cursos que el estudiante
   puede matricular segun el historial
   Escribe la cantidad en *cantidadOut */
Curso **obtenerCursosDisponibles(Curso *cursos, int numCursos,char **cursosAprobados, int numAprobados,int *cantidadOut);

#endif