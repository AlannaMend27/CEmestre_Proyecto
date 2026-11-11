% NOTA: ESTE ARCHIVO ES DE EJEMPLO SI QUIEREN LO CAMBIAMOS TODO.
% Esto define las reglas de prolog

% Maximo porcentaje de los creditos totales de una combinacion que puede aportar una carrera
max_carrera('Computadores', 100).
max_carrera('Electronica', 20).


% Maximo de cursos de una misma area en una combinacion.
max_cursos_area(2).

% Pares incompatibles
incompatibles('CE2201', 'EL2111').
incompatibles('PI2609', 'EL4702').
incompatibles('CE3201', 'EL3313').