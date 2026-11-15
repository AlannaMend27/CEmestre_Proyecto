:- encoding(utf8).
% razon.pl
% Primera regla de politica que viola una combinacion. El orden de las
% reglas es: balance, area, incompatibilidad.

:- module(razon, [primera_violacion/2,
                  razon_rechazo/2,
                  cumple_todo/1,
                  veredicto/3,
                  razon_texto/2]).
:- use_module(reglas).

% primera_violacion(+Cursos, -Razon)
% El corte evita reportar mas de una violacion y que una regla posterior
% pise a una anterior. Razon se unifica despues del corte para que
% funcione aunque llegue instanciada.
primera_violacion(Cursos, Razon) :-
    viola_balance(Cursos, Carrera), !,
    Razon = balance(Carrera).
primera_violacion(Cursos, Razon) :-
    viola_area(Cursos, Area), !,
    Razon = area(Area).
primera_violacion(Cursos, Razon) :-
    viola_incompatibilidad(Cursos, A, B), !,
    Razon = incompatibilidad(A, B).

% exito solo si la combinacion es rechazada.
razon_rechazo(Cursos, Razon) :-
    primera_violacion(Cursos, Razon).

% cumple si no existe ninguna violacion (negacion por falla).
cumple_todo(Cursos) :-
    \+ primera_violacion(Cursos, _).

% veredicto(+Cursos, -Cumple, -Razon)
veredicto(Cursos, false, Razon) :-
    primera_violacion(Cursos, Razon), !.
veredicto(_, true, ninguna).

% texto legible de la razon para el JSON.
razon_texto(ninguna, 'Cumple todas las reglas').
razon_texto(balance(Carrera), Texto) :-
    format(atom(Texto), 'Balance entre carreras: ~w supera su maximo', [Carrera]).
razon_texto(area(Area), Texto) :-
    format(atom(Texto), 'Limite por area: demasiados cursos de ~w', [Area]).
razon_texto(incompatibilidad(A, B), Texto) :-
    format(atom(Texto), 'Incompatibilidad: ~w y ~w', [A, B]).