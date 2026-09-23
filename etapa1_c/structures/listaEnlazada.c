#include <stdio.h>
#include <stdlib.h>
#include "listaEnlazada.h"

// Inicializa la lista dejandola vacia y lista para usar
void inicializarLista(ListaCursos* lista){
    lista->cabeza = NULL;
    lista->cant = 0;
}
 
// Inserta un puntero a curso al final de la lista
void insertarCurso(ListaCursos* lista, Curso* curso){

    // reservar memoria para el nuevo nodo
    NodoCurso* NodoNuevo = malloc(sizeof(NodoCurso));

    if(!NodoNuevo){
        fprintf(stderr, "Error de memoria en insertarCurso\n");
        exit(1);
    }

    NodoNuevo->curso = curso;
    NodoNuevo->siguiente = NULL;

    // si no hay elementos en la lista el nodo es el head
    if (lista->cabeza == NULL){
        lista->cabeza = NodoNuevo;
        lista->cant++;
        return;
    }

    // si ya hay elementos, recorrer hasta el final
    NodoCurso* actual = lista->cabeza;
    while(actual->siguiente != NULL){
        actual = actual->siguiente;
    }

    actual->siguiente = NodoNuevo;
    lista->cant++;

}

// Libera todos los nodos de la lista pero no los cursos
void liberarLista(ListaCursos* lista) {
 
    // Si la lista es nula salimos de inmediato
    if (!lista) {
        return;
    }
 
    NodoCurso* nodoActual = lista->cabeza;
 
    // Recorremos cada nodo guardando el siguiente antes de liberar el actual
    while (nodoActual != NULL) {
        NodoCurso* nodoSiguiente = nodoActual->siguiente;
        free(nodoActual);
        nodoActual = nodoSiguiente;
    }
 
    // Dejamos la lista en estado limpio por si se reutiliza
    lista->cabeza = NULL;
    lista->cant = 0;
}

