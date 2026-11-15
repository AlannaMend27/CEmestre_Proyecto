:- encoding(utf8).
% pruebas_exportar.pl
% Pruebas de exportar.pl y consultas.pl. Se ejecuta desde src/:
%   swipl -g correr_pruebas -t halt pruebas_exportar.pl

:- use_module(library(http/json)).
:- use_module(carga).
:- use_module(consultas).
:- use_module(exportar).

% usa datos propios, asi las pruebas no dependen de la politica real
ruta_politica('../ArchivosPrueba/politica_prueba.pl').
ruta_combinaciones('../ArchivosPrueba/combinaciones_prueba.json').

prueba(Nombre, Meta) :-
    (   catch(Meta, _, fail)
    ->  format("OK     ~w~n", [Nombre])
    ;   format("FALLO  ~w~n", [Nombre])
    ).

prueba_falla(Nombre, Meta) :-
    (   catch(Meta, _, fail)
    ->  format("FALLO  ~w~n", [Nombre])
    ;   format("OK     ~w~n", [Nombre])
    ).

% busca el resultado de una combinacion por su id
resultado(Resultados, Id, R) :-
    member(R, Resultados),
    get_dict(id, R, Id), !.

campo(R, Campo, Valor) :- get_dict(Campo, R, Valor).

correr_pruebas :-
    ruta_politica(Pol), cargar_politica(Pol),
    ruta_combinaciones(Comb), cargar_combinaciones(Comb),

    % carga
    prueba('se cargan 6 combinaciones',
           ( findall(I, combinacion(I, _, _), Is), Is == [1,2,3,4,5,6] )),
    prueba('creditos totales de la combinacion 3',
           combinacion(3, 12, _)),

    % evaluar_todas
    evaluar_todas(Rs),
    prueba('evaluar_todas da un resultado por combinacion', length(Rs, 6)),
    resultado(Rs, 1, R1), resultado(Rs, 2, R2), resultado(Rs, 3, R3),
    resultado(Rs, 4, R4), resultado(Rs, 5, R5), resultado(Rs, 6, R6),

    prueba('valida: cumple true y razon null',
           ( campo(R1, cumple, true), campo(R1, razon, null) )),
    prueba('balance: regla y detalle',
           ( campo(R2, cumple, false), campo(R2, razon, Z2),
             campo(Z2, regla, balance), campo(Z2, detalle, ['Computadores']) )),
    prueba('area: regla y detalle',
           ( campo(R3, razon, Z3),
             campo(Z3, regla, area), campo(Z3, detalle, ['Programación y software']) )),
    prueba('incompatibilidad: regla y par',
           ( campo(R4, razon, Z4),
             campo(Z4, regla, incompatibilidad), campo(Z4, detalle, ['SE1100','SE1400']) )),
    prueba('orden: balance y area a la vez reporta balance',
           ( campo(R5, razon, Z5), campo(Z5, regla, balance) )),
    prueba('limite: 80% exacto cumple',
           ( campo(R6, cumple, true), campo(R6, razon, null) )),
    prueba('selecciones son pares curso-grupo',
           ( campo(R1, selecciones, [S1|_]),
             campo(S1, curso, 'CE1101'), campo(S1, id_grupo, 1),
             campo(S1, numero_grupo, '001') )),
    prueba('el resultado trae creditos_totales',
           campo(R3, creditos_totales, 12)),
    prueba('detalle es siempre una lista',
           ( forall(( member(R, [R2, R3, R4, R5]), campo(R, razon, Zd) ),
                    ( campo(Zd, detalle, Det), is_list(Det) )) )),
    prueba('razon trae texto legible',
           ( campo(R2, razon, ZZ), campo(ZZ, texto, T), sub_atom(T, _, _, _, 'Computadores') )),

    % exportar
    tmp_file(resultados, Base),
    atom_concat(Base, '.json', Salida),
    prueba('exportar_resultados da el resumen',
           ( exportar_resultados(Salida, Res),
             campo(Res, total, 6), campo(Res, cumplen, 2), campo(Res, rechazadas, 4) )),
    prueba('el archivo de salida existe', exists_file(Salida)),
    prueba('el JSON se puede leer de nuevo',
           ( leer_json(Salida, D), campo(D, combinaciones, L), length(L, 6) )),
    prueba('el JSON guarda el resumen',
           ( leer_json(Salida, D2), campo(D2, resumen, Rm), campo(Rm, total, 6) )),
    prueba('el JSON guarda cumple como booleano y razon null',
           ( leer_json(Salida, D3), campo(D3, combinaciones, [C1|_]),
             campo(C1, cumple, true), campo(C1, razon, null) )),
    prueba('el JSON conserva las tildes (utf8)',
           ( leer_json(Salida, D4), campo(D4, combinaciones, L4),
             resultado(L4, 3, C3), campo(C3, razon, Zj),
             campo(Zj, detalle, ['Programación y software']) )),
    tmp_file(dir, Dir),
    atomic_list_concat([Dir, '/sub/anidada/r.json'], Anidada),
    prueba('crea el directorio de salida si no existe',
           ( exportar_resultados(Anidada, _), exists_file(Anidada) )),

    % consultas: curso_en_combinacion en varios modos
    prueba('consulta (+,+): verifica un curso en una combinacion',
           curso_en_combinacion('SE1100', 4)),
    prueba_falla('consulta (+,+): curso que no esta',
                 curso_en_combinacion('SE1100', 1)),
    prueba('consulta (+,-): combinaciones que tienen un curso',
           ( findall(I, curso_en_combinacion('CE1101', I), Ids),
             Ids == [1,2,3,5,6] )),
    prueba('consulta (-,+): cursos de una combinacion',
           ( findall(C, curso_en_combinacion(C, 4), Cs), Cs == ['SE1100','SE1400'] )),
    prueba('consulta (-,-): enumera todos los pares',
           ( findall(C-I, curso_en_combinacion(C, I), Ps), length(Ps, 15) )).

leer_json(Ruta, Dict) :-
    setup_call_cleanup(
        open(Ruta, read, F, [encoding(utf8)]),
        json_read_dict(F, Dict, [value_string_as(atom)]),
        close(F)).