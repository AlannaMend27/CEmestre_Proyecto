:- encoding(utf8).
% exportar.pl
% Evalua todas las combinaciones por backtracking y escribe resultados.json

:- module(exportar, [evaluar_combinacion/4,
                     evaluar_todas/1,
                     exportar_resultados/2]).
:- use_module(library(http/json)).
:- use_module(library(filesex)).
:- use_module(library(apply)).
:- use_module(carga).
:- use_module(razon).

% evaluar_combinacion(+Id, +Creditos, +Cursos, -Resultado)
% Resultado es un diccionario listo para escribir como JSON.
evaluar_combinacion(Id, Creditos, Cursos, Resultado) :-
    veredicto(Cursos, Cumple, Razon),
    selecciones_json(Cursos, Selecciones),
    razon_json(Razon, RazonJson),
    Resultado = _{id: Id,
                  creditos_totales: Creditos,
                  selecciones: Selecciones,
                  cumple: Cumple,
                  razon: RazonJson}.

% pares curso-grupo de la combinacion
selecciones_json([], []).
selecciones_json([curso_grupo(Codigo, IdGrupo, NumeroGrupo, _, _, _)|Resto],
                 [_{curso: Codigo, id_grupo: IdGrupo, numero_grupo: NumeroGrupo}|RestoJson]) :-
    selecciones_json(Resto, RestoJson).

% razon_json(+Razon, -Json)
% Si cumple, la razon es null. Si no, regla violada, detalle y texto.
% detalle es SIEMPRE una lista: [Carrera], [Area] o [CursoA, CursoB].
razon_json(ninguna, null).
razon_json(balance(Carrera), _{regla: balance, detalle: [Carrera], texto: Texto}) :-
    razon_texto(balance(Carrera), Texto).
razon_json(area(Area), _{regla: area, detalle: [Area], texto: Texto}) :-
    razon_texto(area(Area), Texto).
razon_json(incompatibilidad(A, B), _{regla: incompatibilidad, detalle: [A, B], texto: Texto}) :-
    razon_texto(incompatibilidad(A, B), Texto).

% evaluar_todas(-Resultados)
% Recorre todas las combinaciones cargadas por backtracking.
evaluar_todas(Resultados) :-
    findall(Resultado,
            ( combinacion(Id, Creditos, Cursos),
              evaluar_combinacion(Id, Creditos, Cursos, Resultado) ),
            Resultados).

cumple_resultado(Resultado) :-
    get_dict(cumple, Resultado, true).

% resumen(+Resultados, -Resumen)
resumen(Resultados, _{total: Total, cumplen: Cumplen, rechazadas: Rechazadas}) :-
    length(Resultados, Total),
    include(cumple_resultado, Resultados, Validas),
    length(Validas, Cumplen),
    Rechazadas is Total - Cumplen.

% exportar_resultados(+Ruta, -Resumen)
% Escribe el JSON de salida y devuelve el resumen para mostrarlo.
exportar_resultados(Ruta, Resumen) :-
    evaluar_todas(Resultados),
    resumen(Resultados, Resumen),
    file_directory_name(Ruta, Directorio),
    make_directory_path(Directorio),
    setup_call_cleanup(
        open(Ruta, write, Flujo, [encoding(utf8)]),
        json_write_dict(Flujo, _{resumen: Resumen, combinaciones: Resultados}),
        close(Flujo)).