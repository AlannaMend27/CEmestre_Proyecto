:- encoding(utf8).
:- use_module(library(http/json)).
% probar_carga.pl
% Prueba rapida de carga.pl. Se puede cargar desde cualquier carpeta
% (por ejemplo con File > Consult en SWI-Prolog) y luego se escribe:  ?- probar.

% raiz(-Raiz): carpeta etapa3_prolog, calculada desde la ubicacion de este archivo.
raiz(Raiz) :-
    predicate_property(probar, file(Archivo)),
    file_directory_name(Archivo, Pruebas),
    file_directory_name(Pruebas, Raiz).

% resultado(+Texto, +Condicion): imprime OK o FALLA segun la condicion.
resultado(Texto, Condicion) :-
    (   call(Condicion)
    ->  format('OK     ~w~n', [Texto])
    ;   format('FALLA  ~w~n', [Texto])
    ).

probar :-
    raiz(Raiz),
    atom_concat(Raiz, '/src/carga', Modulo),
    use_module(Modulo),
    atom_concat(Raiz, '/ArchivosEntrada/combinaciones_salida.json', Ruta),

    % Total que declara el propio JSON
    open(Ruta, read, Flujo, [encoding(utf8)]),
    json_read_dict(Flujo, Datos, [value_string_as(atom)]),
    close(Flujo),
    get_dict(total_combinaciones, Datos, Esperado),

    carga:cargar_combinaciones(Ruta),
    aggregate_all(count, carga:combinacion(_, _, _), N),
    format('Combinaciones cargadas: ~w (el JSON declara ~w)~n', [N, Esperado]),
    resultado('la cantidad cargada coincide con el JSON', N =:= Esperado),

    resultado('los creditos_totales de cada combinacion coinciden con la suma de sus cursos',
              \+ ( carga:combinacion(_, Total, Cursos),
                   suma_creditos(Cursos, Suma),
                   Suma =\= Total )),

    resultado('todos los codigos de curso son atomos',
              \+ ( carga:combinacion(_, _, Cursos2),
                   member(curso_grupo(Codigo, _, _, _, _, _), Cursos2),
                   \+ atom(Codigo) )),

    carga:cargar_combinaciones(Ruta),
    aggregate_all(count, carga:combinacion(_, _, _), N2),
    resultado('cargar dos veces no duplica los hechos', N2 =:= Esperado),

    carga:combinacion(1, Cr1, Cursos1),
    format('~nCombinacion 1 (~w creditos):~n', [Cr1]),
    forall(member(Curso, Cursos1), ( print(Curso), nl )).

% suma_creditos(+Cursos, -Suma)
suma_creditos(Cursos, Suma) :-
    findall(Cr, member(curso_grupo(_, _, _, Cr, _, _), Cursos), Lista),
    sum_list(Lista, Suma).