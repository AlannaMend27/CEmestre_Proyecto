:- encoding(utf8).
% reglas.pl
% Las tres reglas de politica academica. Cada regla trabaja sobre la lista
% de cursos de UNA combinacion, y los limites los consulta en politica.pl

:- module(reglas, [cumple_balance/1,
                   cumple_area/1,
                   cumple_incompatibilidad/1,
                   viola_balance/2,
                   viola_area/2,
                   viola_incompatibilidad/3,                   incompatible/2]).
:- use_module(library(lists)).

% Regla 1: balance entre carreras

% Suma los creditos de todos los cursos de la combinacion
total_creditos(Cursos, Total) :-
    findall(Cred, member(curso_grupo(_, _, _, Cred, _, _), Cursos), Lista),
    sum_list(Lista, Total).

% calcula los creditos que aporta una carrera. Un curso compartido por N
% carreras aporta Creditos/N a cada una (4 creditos entre 2 carreras: 2
% y 2)
aporte_carrera(Carrera, Cursos, Aporte) :-
    findall(Parte,
            ( member(curso_grupo(_, _, _, Cred, Carreras, _), Cursos),
              member(Carrera, Carreras),
              length(Carreras, N),
              Parte is Cred / N ),
            Partes),
    sum_list(Partes, Aporte).

% viola_balance(+Cursos, ?Carrera)
% verifica si alguna carrera supera su porcentaje maximo por combinacion
viola_balance(Cursos, Carrera) :-
    total_creditos(Cursos, Total),
    max_carrera(Carrera, Max),
    aporte_carrera(Carrera, Cursos, Aporte),
    Aporte * 100 > Max * Total.

% verifica que ninguna carrera se pase de su maximo.
cumple_balance(Cursos) :-
    \+ viola_balance(Cursos, _).

% Regla 2: limite de cursos por area

% Cuenta cuantos cursos de la combinacion pertenecen a esa area.
cursos_en_area(Area, Cursos, Cantidad) :-
    findall(Codigo, member(curso_grupo(Codigo, _, _, _, _, Area), Cursos), Lista),
    length(Lista, Cantidad).

%  Verifica si alguna area tiene mas cursos de los permitidos.
viola_area(Cursos, Area) :-
    max_cursos_area(Maximo),
    findall(A, member(curso_grupo(_, _, _, _, _, A), Cursos), Areas),
    sort(Areas, AreasUnicas),
    member(Area, AreasUnicas),
    cursos_en_area(Area, Cursos, Cantidad),
    Cantidad > Maximo.

% verifica que ninguna area supera el maximo. permitido
cumple_area(Cursos) :-
    \+ viola_area(Cursos, _).

% Regla 3: incompatibilidad entre cursos

% verifica de forma simetrica si dos cursos son compatibles
% comparandolos con las politicas establecidas

incompatible(A, B) :- incompatibles(A, B).
incompatible(A, B) :- incompatibles(B, A).

% Verifica si la combinacion incluye los dos cursos de un par
% incompatible
viola_incompatibilidad(Cursos, A, B) :-
    member(curso_grupo(A, _, _, _, _, _), Cursos),
    member(curso_grupo(B, _, _, _, _, _), Cursos),
    A @< B,
    incompatible(A, B).

% Verifica que no haya ningun par incompatible.
cumple_incompatibilidad(Cursos) :-
    \+ viola_incompatibilidad(Cursos, _, _).
