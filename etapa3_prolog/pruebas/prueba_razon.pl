:- encoding(utf8).
% pruebas_razon.pl
% Pruebas de razon.pl. Se ejecuta desde la carpeta src/:
%   swipl -g correr_pruebas -t halt pruebas_razon.pl

:- use_module('../src/razon').

:- prolog_load_context(directory, Dir),
   atom_concat(Dir, '/../ArchivosEntrada/politica.pl', Ruta),
   consult(Ruta).

% Combinaciones de prueba: curso_grupo(Codigo, IdGrupo, NumGrupo, Creditos, Carreras, Area)

valida([curso_grupo('CE1101', 1, '001', 4, ['Computadores'], 'Programación y software'),
        curso_grupo('MA1102', 2, '001', 4, ['Computadores','Electronica'], 'Matemática'),
        curso_grupo('CI1106', 3, '001', 2, ['Computadores','Electronica'], 'Comunicación e idiomas')]).

solo_computadores([curso_grupo('CE1101', 1, '001', 4, ['Computadores'], 'Programación y software')]).

limite_80([curso_grupo('CE1101', 1, '001', 4, ['Computadores'], 'Programación y software'),
           curso_grupo('MA1102', 2, '001', 4, ['Computadores'], 'Matemática'),
           curso_grupo('FI1101', 3, '001', 2, ['Electronica'], 'Física')]).

pasa_80([curso_grupo('CE1101', 1, '001', 4, ['Computadores'], 'Programación y software'),
         curso_grupo('MA1102', 2, '001', 4, ['Computadores'], 'Matemática'),
         curso_grupo('FI1101', 3, '001', 1, ['Computadores'], 'Física'),
         curso_grupo('QU1102', 4, '001', 1, ['Electronica'], 'Química')]).

tres_area([curso_grupo('CE1101', 1, '001', 4, ['Computadores','Electronica'], 'Programación y software'),
           curso_grupo('CE1103', 2, '001', 4, ['Computadores','Electronica'], 'Programación y software'),
           curso_grupo('CE1106', 3, '001', 4, ['Computadores','Electronica'], 'Programación y software')]).

dos_area([curso_grupo('CE1101', 1, '001', 4, ['Computadores','Electronica'], 'Programación y software'),
          curso_grupo('CE1103', 2, '001', 4, ['Computadores','Electronica'], 'Programación y software')]).

incompat([curso_grupo('SE1100', 1, '001', 2, ['Computadores','Electronica'], 'Humanidades'),
          curso_grupo('SE1400', 2, '001', 2, ['Computadores','Electronica'], 'Humanidades')]).

incompat_inv([curso_grupo('SE1400', 2, '001', 2, ['Computadores','Electronica'], 'Humanidades'),
              curso_grupo('SE1100', 1, '001', 2, ['Computadores','Electronica'], 'Humanidades')]).

incompat_ce([curso_grupo('CE1107', 1, '001', 4, ['Computadores','Electronica'], 'Arquitectura y hardware'),
             curso_grupo('EL3310', 2, '001', 4, ['Computadores','Electronica'], 'Arquitectura y hardware')]).

% balance y area se violan a la vez: debe ganar balance
balance_y_area([curso_grupo('CE1101', 1, '001', 4, ['Computadores'], 'Programación y software'),
                curso_grupo('CE1103', 2, '001', 4, ['Computadores'], 'Programación y software'),
                curso_grupo('CE1106', 3, '001', 4, ['Computadores'], 'Programación y software')]).

% area e incompatibilidad se violan a la vez: debe ganar area
area_e_incompat([curso_grupo('SE1100', 1, '001', 2, ['Computadores','Electronica'], 'Humanidades'),
                 curso_grupo('SE1400', 2, '001', 2, ['Computadores','Electronica'], 'Humanidades'),
                 curso_grupo('CS1101', 3, '001', 2, ['Computadores','Electronica'], 'Humanidades')]).

% prueba(+Nombre, :Meta): OK si Meta tiene exito
prueba(Nombre, Meta) :-
    (   call(Meta)
    ->  format("OK     ~w~n", [Nombre])
    ;   format("FALLO  ~w~n", [Nombre])
    ).

% prueba_falla(+Nombre, :Meta): OK si Meta falla
prueba_falla(Nombre, Meta) :-
    (   call(Meta)
    ->  format("FALLO  ~w~n", [Nombre])
    ;   format("OK     ~w~n", [Nombre])
    ).

correr_pruebas :-
    % combinaciones validas
    valida(V),
    prueba('valida cumple', veredicto(V, true, ninguna)),
    prueba_falla('valida no tiene razon', primera_violacion(V, _)),
    prueba('cumple_todo en valida', cumple_todo(V)),
    prueba('combinacion vacia cumple', veredicto([], true, ninguna)),
    limite_80(L80),
    prueba('exactamente 80% cumple (limite)', veredicto(L80, true, ninguna)),
    dos_area(D),
    prueba('2 cursos de un area cumple (limite)', veredicto(D, true, ninguna)),

    % regla 1
    solo_computadores(SC),
    prueba('solo Computadores viola balance',
           primera_violacion(SC, balance('Computadores'))),
    pasa_80(P80),
    prueba('mas de 80% viola balance',
           primera_violacion(P80, balance('Computadores'))),

    % regla 2
    tres_area(TA),
    prueba('3 cursos de un area viola area',
           primera_violacion(TA, area('Programación y software'))),

    % regla 3
    incompat(I),
    prueba('SE1100 y SE1400 incompatibles',
           primera_violacion(I, incompatibilidad('SE1100', 'SE1400'))),
    incompat_inv(II),
    prueba('incompatibilidad simetrica (orden invertido en la lista)',
           primera_violacion(II, incompatibilidad('SE1100', 'SE1400'))),
    incompat_ce(IC),
    prueba('CE1107 y EL3310 incompatibles',
           primera_violacion(IC, incompatibilidad('CE1107', 'EL3310'))),

    % orden de las reglas
    balance_y_area(BA),
    prueba('balance gana sobre area',
           primera_violacion(BA, balance('Computadores'))),
    prueba_falla('Razon ya ligada a area no se salta el balance',
                 primera_violacion(BA, area(_))),
    area_e_incompat(AI),
    prueba('area gana sobre incompatibilidad',
           primera_violacion(AI, area('Humanidades'))),

    % veredicto, cumple_todo, razon_rechazo
    prueba('veredicto de rechazada',
           veredicto(TA, false, area('Programación y software'))),
    prueba_falla('cumple_todo falla en rechazada', cumple_todo(TA)),
    prueba('razon_rechazo en rechazada', razon_rechazo(I, incompatibilidad(_, _))),
    prueba_falla('razon_rechazo falla en valida', razon_rechazo(V, _)),
    prueba('solo una solucion de primera_violacion',
           ( findall(R, primera_violacion(BA, R), Rs), length(Rs, 1) )),

    % textos
    prueba('texto ninguna', razon_texto(ninguna, 'Cumple todas las reglas')),
    prueba('texto balance', ( razon_texto(balance('Computadores'), T1), sub_atom(T1, _, _, _, 'Computadores') )),
    prueba('texto area', ( razon_texto(area('Matemática'), T2), sub_atom(T2, _, _, _, 'Matemática') )),
    prueba('texto incompatibilidad', ( razon_texto(incompatibilidad('SE1100','SE1400'), T3), sub_atom(T3, _, _, _, 'SE1400') )).