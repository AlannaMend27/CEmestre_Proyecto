#ifndef CONSULTA_H
#define CONSULTA_H
#include "modelos.h"
#include "listaEnlazada.h"

//Aqui va la logica para consultar datos del catalogo ya cargado
//Por ejemplo para saber cuales cursos puede matricular el estudiante y asi

//Devuelve un arreglo de punteros a los cursos que el estudiante puede llevar segun sus aprobados
ListaCursos obtenerCursosDisponibles(Curso *arregloCursos, int cantidadCursos, char **cursosAprobados, int cantidadAprobados);

#endif