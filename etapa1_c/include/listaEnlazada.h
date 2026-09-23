#ifndef LISTAENLAZADA_H
#define LISTAENLAZADA_H
#include "modelos.h"

// structs de la lista enlazada

typedef struct NodoCurso {
    Curso *curso;
    struct NodoCurso *siguiente;
} NodoCurso;

typedef struct ListaCursos{
    NodoCurso* cabeza;
    int cant;
} ListaCursos;

// metodos de la lista enlazada

// Inicializa la lista dejandola vacia y lista para usar
void inicializarLista(ListaCursos *lista);
 
// Inserta un puntero a curso al final de la lista
void insertarCurso(ListaCursos *lista, Curso *curso);
 
// Libera todos los nodos de la lista (sin liberar los cursos)
void liberarLista(ListaCursos *lista);



#endif