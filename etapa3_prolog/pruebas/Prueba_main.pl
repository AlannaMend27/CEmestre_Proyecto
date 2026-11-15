:- encoding(utf8).
% pruebas_main.pl
% Pruebas de main.pl, ejecutandolo como programa aparte (como lo haria el
% usuario por linea de comandos). Se ejecuta desde src/:
%   swipl -g correr_pruebas -t halt pruebas_main.pl

:- use_module(library(process)).
:- use_module(library(http/json)).

ruta_politica('../ArchivosPrueba/politica_prueba.pl').
ruta_combinaciones('../ArchivosPrueba/combinaciones_prueba.json').

prueba(Nombre, Meta) :-
    (   catch(Meta, _, fail)
    ->  format("OK     ~w~n", [Nombre])
    ;   format("FALLO  ~w~n", [Nombre])
    ).

% ejecutar_main(+Args, -Estado, -Salida): corre swipl main.pl Args
ejecutar_main(Args, Estado, Salida) :-
    current_prolog_flag(executable, Swipl),
    process_create(Swipl, ['main.pl'|Args],
                   [stdout(pipe(Out)), stderr(null), process(Pid)]),
    read_string(Out, _, Salida),
    close(Out),
    process_wait(Pid, exit(Estado)).

correr_pruebas :-
    ruta_politica(Pol), ruta_combinaciones(Comb),
    tmp_file(main, Base),
    atom_concat(Base, '.json', Sal),

    prueba('ejecucion normal termina con estado 0',
           ejecutar_main([Comb, Pol, Sal], 0, _)),
    prueba('se genera el archivo de salida', exists_file(Sal)),
    prueba('muestra el resumen por consola',
           ( ejecutar_main([Comb, Pol, Sal], _, Out),
             sub_string(Out, _, _, _, "Total: 6, cumplen: 2, rechazadas: 4") )),
    prueba('el JSON generado tiene 6 combinaciones',
           ( setup_call_cleanup(open(Sal, read, F, [encoding(utf8)]),
                                json_read_dict(F, D), close(F)),
             get_dict(combinaciones, D, L), length(L, 6) )),

    prueba('con 2 argumentos termina con estado 1',
           ejecutar_main([Comb, Pol], 1, _)),
    prueba('con 4 argumentos termina con estado 1',
           ejecutar_main([Comb, Pol, Sal, extra], 1, _)),
    prueba('archivo de combinaciones inexistente termina con estado 1',
           ejecutar_main(['no_existe.json', Pol, Sal], 1, _)),
    prueba('archivo de politica inexistente termina con estado 1',
           ejecutar_main([Comb, 'no_existe.pl', Sal], 1, _)),

    % la politica se puede cambiar sin tocar el codigo
    tmp_file(pol, PBase),
    atom_concat(PBase, '.pl', PolDistinta),
    setup_call_cleanup(open(PolDistinta, write, PF),
        format(PF, ":- dynamic max_carrera/2, max_cursos_area/1, incompatibles/2.~nmax_carrera('Computadores', 100).~nmax_carrera('Electronica', 100).~nmax_cursos_area(5).~n", []),
        close(PF)),
    atom_concat(Base, '_pol.json', Sal2),
    prueba('con otra politica (100%, 5 por area, sin pares) todas cumplen',
           ( ejecutar_main([Comb, PolDistinta, Sal2], 0, Out2),
             sub_string(Out2, _, _, _, "Total: 6, cumplen: 6, rechazadas: 0") )).