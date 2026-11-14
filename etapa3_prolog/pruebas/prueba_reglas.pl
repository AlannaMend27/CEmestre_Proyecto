:- encoding(utf8).
% pruebas/prueba_reglas.pl
% Se carga desde cualquier carpeta (File > Consult) y luego:  ?- probar_reglas.

:- use_module('../src/carga').
:- use_module('../src/reglas').

% ejemplo(Nombre, Cursos)
% curso_grupo(Codigo, IdGrupo, NumGrupo, Creditos, Carreras, Area)
ejemplo(mixta,
    [curso_grupo('MA1403',1,'001',4,['Computadores','Electronica'],'Matemática'),
     curso_grupo('CE1101',2,'002',3,['Computadores'],'Programación y software')]).
ejemplo(solo_computadores,
    [curso_grupo('CE1101',1,'001',3,['Computadores'],'Programación y software'),
     curso_grupo('CE1103',1,'001',4,['Computadores'],'Programación y software')]).
ejemplo(tres_de_matematica,
    [curso_grupo('MA1102',1,'001',4,['Computadores','Electronica'],'Matemática'),
     curso_grupo('MA1103',1,'001',4,['Computadores','Electronica'],'Matemática'),
     curso_grupo('MA1403',1,'001',4,['Computadores','Electronica'],'Matemática')]).
ejemplo(par_incompatible,
    [curso_grupo('SE1400',1,'001',0,['Computadores'],'Humanidades'),
     curso_grupo('MA1102',1,'001',4,['Computadores','Electronica'],'Matemática'),
     curso_grupo('SE1100',1,'001',0,['Computadores'],'Humanidades')]).

% ruta_politica(-Ruta): ArchivosEntrada/politica.pl, calculada desde este archivo
ruta_politica(Ruta) :-
    predicate_property(probar_reglas, file(Archivo)),
    file_directory_name(Archivo, Pruebas),
    file_directory_name(Pruebas, Raiz),
    atom_concat(Raiz, '/ArchivosEntrada/politica.pl', Ruta).

si_no(Meta, si) :- call(Meta), !.
si_no(_, no).

probar_reglas :-
    ruta_politica(Ruta),
    cargar_politica(Ruta),
    forall(ejemplo(Nombre, Cursos),
           ( format("~w:~n", [Nombre]),
             si_no(cumple_balance(Cursos), R1),
             si_no(cumple_area(Cursos), R2),
             si_no(cumple_incompatibilidad(Cursos), R3),
             format("  balance=~w  area=~w  incompatibilidad=~w~n", [R1, R2, R3]),
             forall(viola_balance(Cursos, C),
                    format("  excede balance: ~w~n", [C])),
             forall(viola_area(Cursos, A),
                    format("  area excedida: ~w~n", [A])),
             forall(viola_incompatibilidad(Cursos, X, Y),
                    format("  incompatibles: ~w y ~w~n", [X, Y])) )),
    findall(X, incompatible(X, 'SE1400'), L1),
    format("incompatible(X,'SE1400'): ~w~n", [L1]),
    ( incompatible('SE1400', 'SE1100') -> writeln("simetrico: si")
    ; writeln("simetrico: no") ).
