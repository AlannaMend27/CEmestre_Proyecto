#include <stdio.h>
#include <stdlib.h>
#include "catalogo.h"
#include "consulta.h"
#include "horario.h"
#include "historial.h"
#include "exportar.h"

int main(int cantidadArgumentos, char *arregloArgumentos[]) {

    printf("Proyecto CEmestre \n");   

    //Verificamos que se haya pasado la ruta del catalogo como argumento por linea de comandos
    if (cantidadArgumentos < 2) {
        fprintf(stderr, "Uso: catalogo_run <ruta_al_catalogo.json>\n");
        return 1;
    }
    
    const char *rutaArchivoJson = arregloArgumentos[1];

    //Verificamos que se haya pasado la ruta del historial como segundo argumento
    if (cantidadArgumentos < 3) {
        fprintf(stderr, "Uso: catalogo_run <ruta_catalogo.json> <ruta_historial.txt>\n");
        return 1;
    }
    
    const char *rutaHistorial = arregloArgumentos[2];

    //Cargamos el historial del estudiante desde el archivo de texto
    Historial *historial = cargarHistorial(rutaHistorial);
    
    //Validamos que el historial se haya cargado correctamente en memoria
    if (!historial) {
        fprintf(stderr, "No se pudo cargar el historial.\n");
        return 1;
    }

    printf("Datos del estudiante cargados exitosamente \n");   
    printf("Carrera: %s | Cursos aprobados: %d\n\n", historial->carrera, historial->cantidadAprobados);
    int cantidadCursos = 0;
    
    //Cargamos el catalogo completo desde el archivo JSON proporcionado
    Curso *arregloCursos = cargarCatalogo(rutaArchivoJson, &cantidadCursos);
    
    //Validamos si ocurrio un error al intentar cargar el catalogo
    if (!arregloCursos) {
        fprintf(stderr, "No se pudo cargar el catalogo.\n");
        return 1;
    }

    //Detectamos los choques de horario entre todos los cursos cargados en el catalogo
    detectarChoques(arregloCursos, cantidadCursos);
    
    //Obtenemos el arreglo de punteros a los cursos que el estudiante puede matricular segun su historial
    Curso **cursosDisponibles = obtenerCursosDisponibles(arregloCursos, cantidadCursos,historial->aprobados, historial->cantidadAprobados);

    // generar el archivo de salida json con los cursos disponibles y sus datos para el estudiante
    const char *rutaSalida = "etapa1_c/output/catalogo_salida.json";
    if (!exportarCatalogo(arregloCursos, cantidadCursos, rutaSalida)) {
        fprintf(stderr, "Error al exportar el catalogo.\n");
    }

    printf("El archivo ha sido generado con exito, revisa la carpeta ubicada en: %s \n", rutaSalida);


    //Liberamos toda la memoria dinamica utilizada antes de finalizar el programa
    free(cursosDisponibles);
    liberarCatalogo(arregloCursos, cantidadCursos);
    liberarHistorial(historial);
    
    return 0;
}