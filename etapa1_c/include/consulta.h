#ifndef CONSULTA_H
#define CONSULTA_H

#include "modelos.h"

//Aqui va la logica para consultar datos del catalogo ya cargado
//Por ejemplo para saber cuales cursos puede matricular el estudiante y asi

//Imprime todos los datos de un curso en la consola
void imprimirCurso(const Curso *cursoActual);

//Busca un curso por su codigo e imprime sus datos si lo encuentra
void buscar(const char *codigoCurso, Curso *arregloCursos, int cantidadCursos);

//Devuelve un arreglo de punteros a los cursos que el estudiante puede llevar segun sus aprobados
Curso **obtenerCursosDisponibles(Curso *arregloCursos, int cantidadCursos, char **cursosAprobados, int cantidadAprobados);

#endif