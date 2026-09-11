#include <stdio.h>
#include <stdlib.h>
#include "catalogo.h"
#include "consulta.h"

int main(int argc, char *argv[]) {

    //Verifica que se paso la ruta del catalogo como argumento 
    if (argc < 2) {
        fprintf(stderr, "Uso: catalogo_run <ruta_al_catalogo.json>\n");
        return 1;
    }
    const char *ruta = argv[1];

    // Historial de cursos aprobados por el estudiante 
    char *cursosAprobados[] = {"MA0101", "CE1101", "CE1104", "MA1102", "MA1403"};
    int numAprobados = 5;

    //Carga el catalogo desde el archivo JSON
    int numCursos = 0;
    Curso *cursos = cargarCatalogo(ruta, &numCursos);
    if (!cursos) {
        fprintf(stderr, "No se pudo cargar el catalogo.\n");
        return 1;
    }
    printf("Se cargaron %d cursos.\n\n", numCursos);

    //Obtiene los cursos disponibles segun el historial
    int cantidadDisponibles = 0;
    Curso **cursosDisponibles = obtenerCursosDisponibles(
        cursos, numCursos,
        cursosAprobados, numAprobados,
        &cantidadDisponibles
    );

    printf("Cursos disponibles: %d\n", cantidadDisponibles);
    for (int i = 0; i < cantidadDisponibles; i++) {
        printf("%s - %s\n",
            cursosDisponibles[i]->codigo,
            cursosDisponibles[i]->nombre);
    }

    //Liberar memoria
    free(cursosDisponibles);
    liberarCatalogo(cursos, numCursos);
    return 0;
}