% politica.pl
% Parametros de la politica academica, escritos como hechos

% Declaramos los predicados como dinamicos solo para que puedan ser
% modificados en tiempo de ejecucion
:- dynamic max_carrera/2, max_cursos_area/1, incompatibles/2.

% max_carrera(Carrera, Porcentaje)
% Porcentaje maximo de los creditos totales de una combinacion que puede
% aportar esa carrera.
max_carrera('Computadores', 80).
max_carrera('Electronica', 80).

% max_cursos_area(N)
% Cantidad maxima de cursos de una misma area en una combinacion.
max_cursos_area(2).

% incompatibles(CursoA, CursoB)
% Pares de cursos que no pueden estar juntos en una combinacion.

incompatibles('SE1100', 'SE1400').
incompatibles('SE1200', 'SE1400').
incompatibles('CE1107', 'EL3310').
