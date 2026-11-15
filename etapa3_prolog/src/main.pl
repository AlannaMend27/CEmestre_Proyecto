:- encoding(utf8).
% main.pl
% Punto de entrada. Uso:
%   swipl main.pl <combinaciones.json> <politica.pl> <resultados.json>
% Sin argumentos usa las rutas por defecto de la estructura del proyecto.

:- use_module(library(main)).
:- use_module(carga).
:- use_module(exportar).

:- initialization(main, main).

ruta_defecto('../ArchivosEntrada/combinaciones_salida.json',
             '../ArchivosEntrada/politica.pl',
             '../ArchivosSalida/resultados.json').

main(Argv) :-
    (   argumentos(Argv, Combinaciones, Politica, Salida)
    ->  ejecutar(Combinaciones, Politica, Salida)
    ;   uso,
        halt(1)
    ).

argumentos([], Combinaciones, Politica, Salida) :-
    ruta_defecto(Combinaciones, Politica, Salida).
argumentos([Combinaciones, Politica, Salida], Combinaciones, Politica, Salida).

uso :-
    format(user_error,
           "Uso: swipl main.pl <combinaciones.json> <politica.pl> <resultados.json>~n",
           []).

ejecutar(Combinaciones, Politica, Salida) :-
    verificar_existe(Politica),
    verificar_existe(Combinaciones),
    cargar_politica(Politica),
    cargar_combinaciones(Combinaciones),
    exportar_resultados(Salida, Resumen),
    mostrar_resumen(Salida, Resumen).

verificar_existe(Ruta) :-
    exists_file(Ruta), !.
verificar_existe(Ruta) :-
    format(user_error, "No existe el archivo: ~w~n", [Ruta]),
    halt(1).

mostrar_resumen(Salida, Resumen) :-
    get_dict(total, Resumen, Total),
    get_dict(cumplen, Resumen, Cumplen),
    get_dict(rechazadas, Resumen, Rechazadas),
    format("Resultados escritos en ~w~n", [Salida]),
    format("Total: ~w, cumplen: ~w, rechazadas: ~w~n", [Total, Cumplen, Rechazadas]).