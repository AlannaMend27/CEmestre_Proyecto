#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "consulta.h"

//Imprime todos los datos de un curso especifico en la consola
void imprimirCurso(const Curso *cursoActual) {

    printf("Codigo: %s\n", cursoActual->codigo);
    printf("Nombre: %s\n", cursoActual->nombre);
    printf("Creditos: %d\n", cursoActual->creditos);
    
    //Revisamos si el estudiante tiene permiso de matricularlo
    if (cursoActual->estudiantePuedeMatricular) {
        printf("Puede matricular: si\n");   
    } else {
        printf("Puede matricular: no\n");
    }

    printf("Requisitos (%d): ", cursoActual->numRequisitos);

    //Recorremos e imprimimos la lista de requisitos separados por un espacio

    //NOTA: ESTO ESTA ASI FEO POR MIENTRAS LUEGO SE CAMBIA 
    for (int indiceRequisito = 0; indiceRequisito < cursoActual->numRequisitos; indiceRequisito++) {
        printf("%s ", cursoActual->requisitos[indiceRequisito]);
    }
    printf("\n");

    printf("Correquisitos (%d): ", cursoActual->numCorrequisitos);

    //Hacemos lo mismo para listar los correquisitos del curso
    for (int indiceCorrequisito = 0; indiceCorrequisito < cursoActual->numCorrequisitos; indiceCorrequisito++) {
        printf("%s ", cursoActual->correquisitos[indiceCorrequisito]);
    }
    printf("\n");

    printf("Grupos (%d):\n", cursoActual->numGrupos);
    //Iteramos sobre los grupos disponibles del curso para mostrar sus detalles
    for (int indiceGrupo = 0; indiceGrupo < cursoActual->numGrupos; indiceGrupo++) {

        const Grupo *grupoActual = &cursoActual->grupos[indiceGrupo];
        printf("  - Grupo %s (%s) [%s]\n",
        grupoActual->numeroGrupo, grupoActual->nombre, grupoActual->tipoGrupo);

        printf("    Horarios: ");
        //Mostramos cada bloque de horario en el que se imparte este grupo especifico
        for (int indiceBloque = 0; indiceBloque < grupoActual->numBloquesHorario; indiceBloque++) {
            printf("%s | ", grupoActual->bloquesHorario[indiceBloque]);
        }

        printf("\n    Profesores: ");
        //Listamos los profesores asignados a impartir las lecciones de este grupo
        for (int indiceProfesor = 0; indiceProfesor < grupoActual->numProfesores; indiceProfesor++) {
            printf("%s | ", grupoActual->profesores[indiceProfesor]);
        }
        printf("\n");
    }
    printf("--------------------------------------------------\n");
}

//Busca un curso por su codigo dentro del catalogo e imprime sus datos si lo encuentra
void buscar(const char *codigoCurso, Curso *arregloCursos, int cantidadCursos) {

    //Recorremos todo el catalogo buscando la coincidencia con el codigo solicitado
    for (int indiceCurso = 0; indiceCurso < cantidadCursos; indiceCurso++) {

        //Si el codigo del catalogo coincide con el buscado imprimimos y detenemos la busqueda de inmediato
        if (strcmp(arregloCursos[indiceCurso].codigo, codigoCurso) == 0) {
            printf("\nCurso encontrado (%s):\n", codigoCurso);
            imprimirCurso(&arregloCursos[indiceCurso]);
            return;
        }
    }

    printf("Curso con codigo %s no encontrado.\n", codigoCurso);
}

//Devuelve un arreglo de punteros a los cursos que el estudiante puede matricular segun su historial
Curso **obtenerCursosDisponibles(Curso *arregloCursos, int cantidadCursos,char **cursosAprobados, int cantidadAprobados,int *cantidadSalida) {
    
    Curso **cursosDisponibles = malloc(sizeof(Curso *) * cantidadCursos);
    int cantidad = 0;

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
            cursosDisponibles[cantidad] = &arregloCursos[indiceCurso];
            cantidad++;
        }
    }

    *cantidadSalida = cantidad;
    return cursosDisponibles;
}