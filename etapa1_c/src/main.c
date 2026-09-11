#include <stdio.h>
#include <stdlib.h>
#include "catalogo.h"
#include "consulta.h"
#include "horario.h"

int main(int cantidadArgumentos, char *arregloArgumentos[]) {

    //Verificamos que se haya pasado la ruta del catalogo como argumento por linea de comandos
    if (cantidadArgumentos < 2) {
        fprintf(stderr, "Uso: catalogo_run <ruta_al_catalogo.json>\n");
        return 1;
    }
    
    const char *rutaArchivoJson = arregloArgumentos[1];

    //ESTO ES POR MIENTRAS LUEGO VA EL ARCHIVO
    char *cursosAprobados[] = {"MA0101", "CE1101", "CE1104", "MA1102", "MA1403"};
    int cantidadAprobados = 5;

    int cantidadCursos = 0;
    
    //Cargamos el catalogo completo desde el archivo JSON proporcionado
    Curso *arregloCursos = cargarCatalogo(rutaArchivoJson, &cantidadCursos);
    
    //Validamos si ocurrio un error al intentar cargar el catalogo
    if (!arregloCursos) {
        fprintf(stderr, "No se pudo cargar el catalogo.\n");
        return 1;
    }
    
    printf("Se cargaron %d cursos.\n\n", cantidadCursos);

    //Detectamos los choques de horario entre todos los cursos cargados en el catalogo
    detectarChoques(arregloCursos, cantidadCursos);
    printf("Choques de horario calculados.\n\n");

    //Recorremos los cursos para mostrar en consola los choques encontrados y verificar el resultado
    for (int indiceCurso = 0; indiceCurso < cantidadCursos; indiceCurso++) {
        if (arregloCursos[indiceCurso].numChocaCon > 0) {
            printf("%s choca con: ", arregloCursos[indiceCurso].codigo);
            
            for (int indiceChoque = 0; indiceChoque < arregloCursos[indiceCurso].numChocaCon; indiceChoque++) {
                printf("%s ", arregloCursos[indiceCurso].chocaCon[indiceChoque]);
            }
            
            printf("\n");
        }
    }

    int cantidadDisponibles = 0;
    
    //Obtenemos el arreglo de punteros a los cursos que el estudiante puede matricular segun su historial
    Curso **cursosDisponibles = obtenerCursosDisponibles(arregloCursos, cantidadCursos,cursosAprobados, cantidadAprobados,&cantidadDisponibles
    );

    printf("\nCursos disponibles: %d\n", cantidadDisponibles);
    
    //Listamos cada uno de los cursos que el estudiante tiene permitidos llevar
    for (int indiceDisponible = 0; indiceDisponible < cantidadDisponibles; indiceDisponible++) {
        printf("%s - %s\n",cursosDisponibles[indiceDisponible]->codigo,cursosDisponibles[indiceDisponible]->nombre);
    }

    //Liberamos toda la memoria dinamica utilizada antes de finalizar el programa
    free(cursosDisponibles);
    liberarCatalogo(arregloCursos, cantidadCursos);
    
    return 0;
}