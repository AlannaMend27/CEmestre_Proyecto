#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "horario.h"

//Aqui van las comparaciones de bloques de horario entre grupos

//Convierte horas y minutos en minutos contados desde la medianoche
static int horaAMinutos(int horas, int minutos) {
    return (horas * 60) + minutos;
}

//Convierte el nombre del dia a una abreviatura corta
static void abreviarDia(const char *diaCompleto, char *abreviatura) {
    
    if (strncmp(diaCompleto, "Lunes", 5) == 0) { 
        strcpy(abreviatura, "LUN"); 

    } else if (strncmp(diaCompleto, "Martes", 6) == 0) { 
        strcpy(abreviatura, "MAR"); 

    } else if (strncmp(diaCompleto, "Miercoles", 9) == 0) { 
        strcpy(abreviatura, "MIE"); 

    } else if (strncmp(diaCompleto, "Jueves", 6) == 0) { 
        strcpy(abreviatura, "JUE"); 

    } else if (strncmp(diaCompleto, "Viernes", 7) == 0) { 
        strcpy(abreviatura, "VIE"); 

    } else { 
        strcpy(abreviatura, "???"); 
    }
}

//Convierte un string de texto horario en una estructura BloqueHorario
int parsearBloqueHorario(const char *textoHorario, BloqueHorario *bloqueParseado) {

    //Declaramos variables temporales
    char diaCompleto[32];
    int horaInicio, minInicio, horaFin, minFin;

    //Lee el string con el formato real y asigna los valores a las variables
    int camposLeidos = sscanf(textoHorario, "%31[^ ] - %d:%d:%d:%d",diaCompleto, &horaInicio, &minInicio, &horaFin, &minFin);

    //Si no leyo exactamente los cinco campos requeridos sabemos que el formato es incorrecto
    if (camposLeidos != 5) {
        return 0;
    }

    //Convierte el nombre completo del dia a abreviatura y guarda las horas calculadas en minutos
    abreviarDia(diaCompleto, bloqueParseado->dia);
    bloqueParseado->inicioMin = horaAMinutos(horaInicio, minInicio);
    bloqueParseado->finMin = horaAMinutos(horaFin, minFin);

    return 1;
}

//Compara dos bloques ya parseados para verificar si chocan entre si
int bloquesChocan(const BloqueHorario *primerBloque, const BloqueHorario *segundoBloque) {

    //Si los dias son distintos descartamos de una vez el choque
    if (strcmp(primerBloque->dia, segundoBloque->dia) != 0) {
        return 0;
    }

    //Comparamos los minutos para ver si los rangos de hora chocan
    if (primerBloque->inicioMin < segundoBloque->finMin && segundoBloque->inicioMin < primerBloque->finMin) {
        return 1;
    }

    return 0;
}

//Compara todos los bloques de horario de dos grupos para buscar conflictos
int gruposChocan(const Grupo *primerGrupo, const Grupo *segundoGrupo) {

    //Recorremos todos los bloques de horario del primer grupo
    for (int indiceBloqueA = 0; indiceBloqueA < primerGrupo->numBloquesHorario; indiceBloqueA++) {
        BloqueHorario bloqueA;
        int parseadoA = parsearBloqueHorario(primerGrupo->bloquesHorario[indiceBloqueA], &bloqueA);

        //Si el bloque actual no tiene un formato valido pasamos al siguiente
        if (!parseadoA) {
            continue;
        }

        //Comparamos el bloque validado contra cada uno de los bloques del segundo grupo
        for (int indiceBloqueB = 0; indiceBloqueB < segundoGrupo->numBloquesHorario; indiceBloqueB++) {
            BloqueHorario bloqueB;
            int parseadoB = parsearBloqueHorario(segundoGrupo->bloquesHorario[indiceBloqueB], &bloqueB);

            if (!parseadoB) {
                continue;
            }

            //Si encontramos un par de bloques que chocan reportamos el conflicto inmediatamente
            if (bloquesChocan(&bloqueA, &bloqueB)) {
                return 1;
            }
        }
    }

    //Si termina el ciclo sin encontrar conflictos confirmamos que los grupos no chocan
    return 0;
}

//Agrega un codigo especifico a la lista de choques de un curso
void agregarChoque(Curso *cursoActual, const char *codigoQueChoca) {

    //Revisamos toda la lista actual para no insertar codigos duplicados
    for (int indiceChoque = 0; indiceChoque < cursoActual->numChocaCon; indiceChoque++) {
        if (strcmp(cursoActual->chocaCon[indiceChoque], codigoQueChoca) == 0) {
            return;
        }
    }

    //Calculamos el nuevo tamano para agrandar el arreglo y solicitamos la memoria
    int nuevaCantidad = cursoActual->numChocaCon + 1;
    char **nuevoArreglo = realloc(cursoActual->chocaCon, sizeof(char*) * nuevaCantidad);
    
    //Si falla la asignacion de memoria 
    if (!nuevoArreglo) {
        fprintf(stderr, "Error de memoria en agregarChoque\n");
        exit(1);
    }
    
    //asignamos el nuevo arreglo al struct
    cursoActual->chocaCon = nuevoArreglo;

    //Reservamos el espacio exacto para el string del codigo y lo copiamos
    cursoActual->chocaCon[cursoActual->numChocaCon] = malloc(strlen(codigoQueChoca) + 1);

    //Verificamos is no hay error
    if (!cursoActual->chocaCon[cursoActual->numChocaCon]) {
        fprintf(stderr, "Error de memoria en agregarChoque\n");
        exit(1);
    }
    
    //Copiamos el codigo nuevo en la posicion libre y actualizamos el total de choques
    strcpy(cursoActual->chocaCon[cursoActual->numChocaCon], codigoQueChoca);
    cursoActual->numChocaCon = nuevaCantidad;
}

//Recorre todos los pares de cursos del catalogo y llena el arreglo chocaCon de cada uno
void detectarChoques(Curso *arregloCursos, int cantidadCursos) {

    //Recorremos cada par de cursos distintos sin repetir combinaciones
    for (int indiceCurso1 = 0; indiceCurso1 < cantidadCursos; indiceCurso1++) {
        for (int indiceCurso2 = indiceCurso1 + 1; indiceCurso2 < cantidadCursos; indiceCurso2++) {

            //Si ambos cursos tienen exactamente el mismo codigo saltamos la comparacion
            if (strcmp(arregloCursos[indiceCurso1].codigo, arregloCursos[indiceCurso2].codigo) == 0) {
                continue;
            }

            int encontroChoque = 0;

            //Comparamos cada grupo del primer curso contra los grupos del segundo
            for (int indiceGrupo1 = 0; indiceGrupo1 < arregloCursos[indiceCurso1].numGrupos; indiceGrupo1++) {
                for (int indiceGrupo2 = 0; indiceGrupo2 < arregloCursos[indiceCurso2].numGrupos; indiceGrupo2++) {
                    
                    //Validamos si existe conflicto entre estos dos grupos especificos
                    if (gruposChocan(&arregloCursos[indiceCurso1].grupos[indiceGrupo1], &arregloCursos[indiceCurso2].grupos[indiceGrupo2])) {
                        encontroChoque = 1;
                        break;
                    }
                }
                
                //Si ya encontramos un choque rompemos el ciclo exterior tambien
                if (encontroChoque) {
                    break;
                }
            }
            
            //Si detectamos que los cursos chocan agregamos el codigo correspondiente a cada uno
            if (encontroChoque) {
                agregarChoque(&arregloCursos[indiceCurso1], arregloCursos[indiceCurso2].codigo);
                agregarChoque(&arregloCursos[indiceCurso2], arregloCursos[indiceCurso1].codigo);
            }
        }
    }
}