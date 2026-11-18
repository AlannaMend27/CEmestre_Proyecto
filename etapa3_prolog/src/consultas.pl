:- encoding(utf8).
% Predicados de consulta que se pueden usar en mas de un modo: segun cuales

:- module(consultas, [curso_en_combinacion/2]).
:- use_module(carga).


curso_en_combinacion(Codigo, Id) :-
    combinacion(Id, _, Cursos),
    member(curso_grupo(Codigo, _, _, _, _, _), Cursos).