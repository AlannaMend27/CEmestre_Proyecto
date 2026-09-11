/*aqui va la logica de preguntarle cosas al catalogo 
ya cargado, tipo preguntarle que cuales puede matricular y asi
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "consulta.h"

/* Imprime todos los datos de un curso en consola */
void imprimirCurso(const Curso *c) {
    printf("Codigo: %s\n", c->codigo);
    printf("Nombre: %s\n", c->nombre);
    printf("Creditos: %d\n", c->creditos);
    printf("Puede matricular: %s\n", c->estudiantePuedeMatricular ? "si" : "no");

    printf("Requisitos (%d): ", c->numRequisitos);
    for (int i = 0; i < c->numRequisitos; i++) {
        printf("%s ", c->requisitos[i]);
    }
    printf("\n");

    printf("Correquisitos (%d): ", c->numCorrequisitos);
    for (int i = 0; i < c->numCorrequisitos; i++) {
        printf("%s ", c->correquisitos[i]);
    }
    printf("\n");

    printf("Grupos (%d):\n", c->numGrupos);
    for (int i = 0; i < c->numGrupos; i++) {
        const Grupo *g = &c->grupos[i];
        printf("  - Grupo %s (%s) [%s]\n",
            g->numeroGrupo, g->nombre, g->tipoGrupo);

        printf("    Horarios: ");
        for (int k = 0; k < g->numBloquesHorario; k++) {
            printf("%s | ", g->bloquesHorario[k]);
        }

        printf("\n    Profesores: ");
        for (int k = 0; k < g->numProfesores; k++) {
            printf("%s | ", g->profesores[k]);
        }
        printf("\n");
    }
    printf("--------------------------------------------------\n");
}

/* Busca un curso por codigo e imprime sus datos si lo encuentra */
void buscar(const char *codigoCurso, Curso *cursos, int numCursos) {
    for (int i = 0; i < numCursos; i++) {
        if (strcmp(cursos[i].codigo, codigoCurso) == 0) {
            printf("\nCurso encontrado (%s):\n", codigoCurso);
            imprimirCurso(&cursos[i]);
            return;
        }
    }
    printf("Curso con codigo %s no encontrado.\n", codigoCurso);
}

/* Devuelve un arreglo de punteros a los cursos que el estudiante
   puede matricular segun su historial de aprobados */
Curso **obtenerCursosDisponibles(Curso *cursos, int numCursos, char **cursosAprobados, int numAprobados, int *cantidadOut) {
    Curso **cursosDisponibles = malloc(sizeof(Curso *) * numCursos);
    int cantidad = 0;

    for (int i = 0; i < numCursos; i++) {

        //Si el curso ya fue aprobado, se omite 
        bool yaAprobado = false;
        for (int j = 0; j < numAprobados; j++) {
            if (strcmp(cursos[i].codigo, cursosAprobados[j]) == 0) {
                yaAprobado = true;
                break;
            }
        }
        if (yaAprobado) {
            continue;
        }

        // Verifica que TODOS los requisitos del curso esten aprobados 
        bool requisitosCumplidos = true;
        for (int k = 0; k < cursos[i].numRequisitos; k++) {
            bool requisitoCumplido = false;
            for (int l = 0; l < numAprobados; l++) {
                if (strcmp(cursos[i].requisitos[k], cursosAprobados[l]) == 0) {
                    requisitoCumplido = true;
                    break;
                }
            }
            if (!requisitoCumplido) {
                requisitosCumplidos = false;
                break;
            }
        }

        if (requisitosCumplidos) {
            cursosDisponibles[cantidad] = &cursos[i];
            cantidad++;
        }
    }

    *cantidadOut = cantidad;
    return cursosDisponibles;
}