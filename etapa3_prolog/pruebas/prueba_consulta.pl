:- encoding(utf8).
% pruebas/prueba_consultas.pl
% Se carga desde cualquier carpeta (File > Consult) y luego:  ?- probar_consultas.

:- use_module('../src/carga').
:- use_module('../src/consultas').

ruta_json(Ruta) :-
    predicate_property(probar_consultas, file(Archivo)),
    file_directory_name(Archivo, Pruebas),
    file_directory_name(Pruebas, Raiz),
    atom_concat(Raiz, '/ArchivosEntrada/combinaciones_salida.json', Ruta).

% prueba(+Nombre, :Meta): OK si Meta tiene exito
prueba(Nombre, Meta) :-
    (   call(Meta)
    ->  format("OK     ~w~n", [Nombre])
    ;   format("FALLO  ~w~n", [Nombre])
    ).

probar_consultas :-
    ruta_json(Ruta),
    cargar_combinaciones(Ruta),
    combinacion(Id, _, Cursos),
    Cursos = [curso_grupo(Codigo, _, _, _, _, _)|_],
    format("Se usa la combinacion ~w y el curso ~w~n~n", [Id, Codigo]),

    % modo 1 (+,+): verificar
    prueba('modo (+,+): el curso esta en su combinacion',
           curso_en_combinacion(Codigo, Id)),
    prueba('modo (+,+): un curso inventado no esta',
           \+ curso_en_combinacion('XX9999', Id)),

    % modo 2 (+,-): en que combinaciones aparece un curso
    prueba('modo (+,-): entre las combinaciones del curso esta la combinacion usada',
           ( findall(I, curso_en_combinacion(Codigo, I), Ids), memberchk(Id, Ids) )),

    % modo 3 (-,+): que cursos tiene una combinacion
    prueba('modo (-,+): da exactamente los cursos de la combinacion',
           ( findall(C, curso_en_combinacion(C, Id), Cs),
             findall(C2, member(curso_grupo(C2,_,_,_,_,_), Cursos), Cs2),
             Cs == Cs2 )),

    % modo 4 (-,-): enumerar todo
    prueba('modo (-,-): la cantidad de pares es la suma de los cursos de todas las combinaciones',
           ( aggregate_all(count, curso_en_combinacion(_, _), Pares),
             aggregate_all(sum(L), ( combinacion(_, _, Cs3), length(Cs3, L) ), Suma),
             Pares =:= Suma )),

    format("~nCursos de la combinacion ~w: ", [Id]),
    findall(C, curso_en_combinacion(C, Id), Lista),
    format("~w~n", [Lista]).