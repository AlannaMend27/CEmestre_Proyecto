#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "consulta.h"

//Revisa si un codigo de curso esta en la lista de cursos aprobados
static bool estaAprobado(const char *codigoCurso, char **cursosAprobados, int cantidadAprobados) {

    for (int indiceAprobado = 0; indiceAprobado < cantidadAprobados; indiceAprobado++) {
        if (strcmp(codigoCurso, cursosAprobados[indiceAprobado]) == 0) {
            return true;
        }
    }

    return false;
}

//Revisa si el curso pertenece a la carrera del estudiante
static bool perteneceACarrera(const Curso *curso, const char *carreraEstudiante) {

    for (int indiceCarrera = 0; indiceCarrera < curso->numCarreras; indiceCarrera++) {
        if (strcmp(curso->carreras[indiceCarrera], carreraEstudiante) == 0) {
            return true;
        }
    }

    return false;
}

//Busca un curso por su codigo en el catalogo, devuelve NULL si no existe
static const Curso *buscarCursoPorCodigo(const Curso *arregloCursos, int cantidadCursos, const char *codigoCurso) {

    for (int indiceCurso = 0; indiceCurso < cantidadCursos; indiceCurso++) {
        if (strcmp(arregloCursos[indiceCurso].codigo, codigoCurso) == 0) {
            return &arregloCursos[indiceCurso];
        }
    }

    return NULL;
}

//Revisa que el curso tenga al menos un grupo y que todos sus requisitos esten aprobados
static bool cumpleRequisitosYTieneGrupos(const Curso *curso, char **cursosAprobados, int cantidadAprobados) {

    //Un curso sin grupos ofrecidos no se puede matricular
    if (curso->numGrupos == 0) {
        return false;
    }

    //Si falta un solo requisito el curso no se puede matricular
    for (int indiceRequisito = 0; indiceRequisito < curso->numRequisitos; indiceRequisito++) {
        if (!estaAprobado(curso->requisitos[indiceRequisito], cursosAprobados, cantidadAprobados)) {
            return false;
        }
    }

    return true;
}

//Devuelve una lista con los cursos que el estudiante puede matricular segun su historial
ListaCursos obtenerCursosDisponibles(Curso *arregloCursos, int cantidadCursos, char **cursosAprobados, int cantidadAprobados, const char *carreraEstudiante) {

    ListaCursos lista;
    inicializarLista(&lista);

    //Evaluamos cada curso del catalogo para ver si el estudiante es elegible
    for (int indiceCurso = 0; indiceCurso < cantidadCursos; indiceCurso++) {

        Curso *cursoActual = &arregloCursos[indiceCurso];

        //Saltamos los cursos que no pertenecen a la carrera del estudiante
        if (!perteneceACarrera(cursoActual, carreraEstudiante)) {
            continue;
        }

        //Guardamos si ya lo aprobo y lo saltamos, un curso aprobado no se vuelve a matricular
        cursoActual->yaAprobado = estaAprobado(cursoActual->codigo, cursosAprobados, cantidadAprobados);
        if (cursoActual->yaAprobado) {
            continue;
        }

        //Debe tener grupos y cumplir todos sus requisitos
        if (!cumpleRequisitosYTieneGrupos(cursoActual, cursosAprobados, cantidadAprobados)) {
            continue;
        }

        //Cada correquisito debe estar aprobado o poder matricularse junto con este curso
        bool correquisitosCumplidos = true;
        for (int indiceCorrequisito = 0; indiceCorrequisito < cursoActual->numCorrequisitos; indiceCorrequisito++) {

            const char *codigoCorrequisito = cursoActual->correquisitos[indiceCorrequisito];

            if (estaAprobado(codigoCorrequisito, cursosAprobados, cantidadAprobados)) {
                continue;
            }

            const Curso *cursoCorrequisito = buscarCursoPorCodigo(arregloCursos, cantidadCursos, codigoCorrequisito);

            //Si el correquisito no existe o el estudiante tampoco lo puede llevar, este curso queda fuera
            if (cursoCorrequisito == NULL || !cumpleRequisitosYTieneGrupos(cursoCorrequisito, cursosAprobados, cantidadAprobados)) {
                correquisitosCumplidos = false;
                break;
            }
        }

        //Si cumplio todo lo agregamos a la lista de disponibles
        if (correquisitosCumplidos) {
            cursoActual->estudiantePuedeMatricular = true;
            insertarCurso(&lista, cursoActual);
        }
    }

    return lista;
}