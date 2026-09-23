#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "consulta.h"


//Devuelve un arreglo de punteros a los cursos que el estudiante puede matricular segun su historial
ListaCursos obtenerCursosDisponibles(Curso *arregloCursos, int cantidadCursos,char **cursosAprobados, int cantidadAprobados) {
    
    // inicializamos lista enlazad apara guardar cursos
    ListaCursos lista;
    inicializarLista(&lista);

    //Evaluamos cada curso del catalogo para ver si el estudiante es elegible
    for (int indiceCurso = 0; indiceCurso < cantidadCursos; indiceCurso++) {

        bool yaAprobado = false;
        //Revisamos el historial del estudiante para verificar si ya paso este curso
        for (int indiceAprobado = 0; indiceAprobado < cantidadAprobados; indiceAprobado++) {
            //Si los codigos coinciden levantamos la bandera de aprobado y rompemos el ciclo
            if (strcmp(arregloCursos[indiceCurso].codigo, cursosAprobados[indiceAprobado]) == 0) {
                yaAprobado = true;
                break;
            }
        }
        
        //Si el curso ya fue aprobado lo saltamos y seguimos evaluando el siguiente
        if (yaAprobado) {
            continue;
        }

        bool requisitosCumplidos = true;
        //Validamos que el estudiante cumpla con todos y cada uno de los requisitos del curso
        for (int indiceRequisito = 0; indiceRequisito < arregloCursos[indiceCurso].numRequisitos; indiceRequisito++) {
            bool requisitoCumplido = false;
            
            //Buscamos el requisito actual dentro de la lista de cursos aprobados del estudiante
            for (int indiceHistorial = 0; indiceHistorial < cantidadAprobados; indiceHistorial++) {
                //Si encontramos el requisito en su historial lo marcamos como cumplido
                if (strcmp(arregloCursos[indiceCurso].requisitos[indiceRequisito], cursosAprobados[indiceHistorial]) == 0) {
                    requisitoCumplido = true;
                    break;
                }
            }
            
            //Si le falta un solo requisito marcamos fallo y detenemos la revision para ese curso
            if (!requisitoCumplido) {
                requisitosCumplidos = false;
                break;
            }
        }

        //Si cumplio absolutamente todos los requisitos agregamos el curso al arreglo de disponibles
        if (requisitosCumplidos) {
            arregloCursos[indiceCurso].estudiantePuedeMatricular = true;
            insertarCurso(&lista, &arregloCursos[indiceCurso]);
        }
    }

    return lista;
}