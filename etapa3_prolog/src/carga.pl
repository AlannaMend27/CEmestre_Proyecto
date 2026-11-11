:- encoding(utf8).
% carga.pl
% Lee el JSON generado por Racket y lo convierte en hechos


:- module(carga, [cargar_combinaciones/1, cargar_politica/1, combinacion/3]).
:- use_module(library(http/json)).

:- dynamic combinacion/3.

% carga politica.pl como un archivo de hechos normal.
cargar_politica(Ruta) :-
    consult(user:Ruta).

% lee el JSON y guarda un hecho por combinacion.
cargar_combinaciones(Ruta) :-
    retractall(combinacion(_, _, _)),
    open(Ruta, read, Flujo, [encoding(utf8)]),
    json_read_dict(Flujo, Datos, [value_string_as(atom)]),
    close(Flujo),
    get_dict(combinaciones, Datos, Lista),
    registrar_combinaciones(Lista).

%  recorre la lista y hace un hecho por combinacion.
registrar_combinaciones([]).
registrar_combinaciones([C|Resto]) :-
    get_dict(id, C, Id),
    get_dict(creditos_totales, C, Creditos),
    get_dict(selecciones, C, CursosJson),
    convertir_cursos(CursosJson, Cursos),
    assertz(combinacion(Id, Creditos, Cursos)),
    registrar_combinaciones(Resto).

% cada curso del JSON pasa a curso_grupo 
convertir_cursos([], []).
convertir_cursos([CursoJson|Resto], [Curso|RestoCursos]) :-
    crear_curso_grupo(CursoJson, Curso),
    convertir_cursos(Resto, RestoCursos).

% saca los campos del diccionario
crear_curso_grupo(D, curso_grupo(Codigo, IdGrupo, NumeroGrupo, Creditos, Carreras, Area)) :-
    get_dict(curso, D, Codigo),
    get_dict(id_grupo, D, IdGrupo),
    get_dict(numero_grupo, D, NumeroGrupo),
    get_dict(creditos, D, Creditos),
    get_dict(carreras, D, Carreras),
    get_dict(area, D, Area).