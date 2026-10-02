#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "horario.h"
#include "manejoArreglos.h"

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

//Agrega un par curso-grupo a la lista de choques de un grupo
void agregarChoque(Grupo *grupoActual, const char *codigoCursoQueChoca, int idGrupoQueChoca) {

    //Revisamos la lista actual para no insertar pares duplicados
    for (int indiceChoque = 0; indiceChoque < grupoActual->numChocaCon; indiceChoque++) {
        const ChoqueGrupo *choqueExistente = &grupoActual->chocaCon[indiceChoque];
        if (strcmp(choqueExistente->codigoCurso, codigoCursoQueChoca) == 0 &&
            choqueExistente->idGrupo == idGrupoQueChoca) {
            return;
        }
    }

    //Agrandamos el arreglo en uno
    int nuevaCantidad = grupoActual->numChocaCon + 1;
    ChoqueGrupo *nuevoArreglo = realloc(grupoActual->chocaCon, sizeof(ChoqueGrupo) * nuevaCantidad);

    if (!nuevoArreglo) {
        fprintf(stderr, "Error de memoria en agregarChoque\n");
        exit(1);
    }

    grupoActual->chocaCon = nuevoArreglo;

    //Guardamos el par en la posicion libre y actualizamos el total
    ChoqueGrupo *choqueNuevo = &grupoActual->chocaCon[grupoActual->numChocaCon];
    copiarStr(choqueNuevo->codigoCurso, sizeof(choqueNuevo->codigoCurso), codigoCursoQueChoca);
    choqueNuevo->idGrupo = idGrupoQueChoca;
    grupoActual->numChocaCon = nuevaCantidad;
}

//Recorre todos los pares de grupos de cursos distintos y llena el chocaCon de cada grupo
void detectarChoques(Curso *arregloCursos, int cantidadCursos) {

    for (int indiceCurso1 = 0; indiceCurso1 < cantidadCursos; indiceCurso1++) {
        for (int indiceCurso2 = indiceCurso1 + 1; indiceCurso2 < cantidadCursos; indiceCurso2++) {

            Curso *primerCurso = &arregloCursos[indiceCurso1];
            Curso *segundoCurso = &arregloCursos[indiceCurso2];

            //Si ambos cursos tienen el mismo codigo saltamos la comparacion
            if (strcmp(primerCurso->codigo, segundoCurso->codigo) == 0) {
                continue;
            }

            //Comparamos cada grupo del primer curso contra cada grupo del segundo
            for (int indiceGrupo1 = 0; indiceGrupo1 < primerCurso->numGrupos; indiceGrupo1++) {
                for (int indiceGrupo2 = 0; indiceGrupo2 < segundoCurso->numGrupos; indiceGrupo2++) {

                    Grupo *primerGrupo = &primerCurso->grupos[indiceGrupo1];
                    Grupo *segundoGrupo = &segundoCurso->grupos[indiceGrupo2];

                    //Si estos dos grupos chocan se registra el par en ambos lados
                    if (gruposChocan(primerGrupo, segundoGrupo)) {
                        agregarChoque(primerGrupo, segundoCurso->codigo, segundoGrupo->idGrupo);
                        agregarChoque(segundoGrupo, primerCurso->codigo, primerGrupo->idGrupo);
                    }
                }
            }
        }
    }
}