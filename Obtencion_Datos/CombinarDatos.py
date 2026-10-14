# Este archivo combina los horarios (horarios_raw.json) con los planes de estudio
# de Ingeniería en Computadores e Ingeniería en Electrónica para generar un único
# catálogo de cursos unificado para el proyecto CEmestre.
import json

HORARIOS_FILE        = "C:/Documentos/ParadigmasDeProgramacion/CEmestre_Proyecto/Obtencion_Datos/horarios_raw.json"
PLAN_FILE_COMPUTADORES = "C:/Documentos/ParadigmasDeProgramacion/CEmestre_Proyecto/Obtencion_Datos/plan_estudios_computadores.json"
PLAN_FILE_ELECTRONICA  = "C:/Documentos/ParadigmasDeProgramacion/CEmestre_Proyecto/Obtencion_Datos/plan_estudios_electronica.json"
SALIDA_FILE          = "C:/Documentos/ParadigmasDeProgramacion/CEmestre_Proyecto/Obtencion_Datos/catalogo_cursos.json"

SEMESTRES_MAX = 6
SEMESTRES_INCLUIDOS = {
    "Semestre 0", "Semestre 1", "Semestre 2",
    "Semestre 3", "Semestre 4", "Semestre 5", "Semestre 6"
}

NOMBRE_COMPUTADORES = "Computadores"
NOMBRE_ELECTRONICA  = "Electronica"

#  Carga de datos 

with open(HORARIOS_FILE, encoding="utf-8") as f:
    horarios = json.load(f)

with open(PLAN_FILE_COMPUTADORES, encoding="utf-8") as f:
    plan_computadores = json.load(f)

with open(PLAN_FILE_ELECTRONICA, encoding="utf-8") as f:
    plan_electronica = json.load(f)

#  Índice de horarios por código 

horarios_por_codigo: dict[str, list] = {}
for curso in horarios:
    cod = curso["codigo"]
    horarios_por_codigo.setdefault(cod, []).append(curso)

#  Funciones auxiliares 

def limpiar_lista(lista: list[dict]) -> list[str]:
    """Convierte [{"id": "CE1101"}, ...] a ["CE1101", ...] eliminando placeholders."""
    return [
        item["id"] for item in lista
        if item.get("id") and "No hay" not in item["id"]
    ]

def buscar_grupos(codigo: str, equivalentes: list[str]) -> list:
    """Busca grupos por código principal y todos sus equivalentes disponibles."""
    resultados = []
    if codigo in horarios_por_codigo:
        resultados.extend(horarios_por_codigo[codigo])
    for eq in equivalentes:
        if eq in horarios_por_codigo:
            resultados.extend(horarios_por_codigo[eq])
    return resultados

def construir_entrada(curso_plan: dict, carrera: str) -> dict:
    """Construye la entrada del catálogo para un curso dado su plan y carrera."""
    codigo      = curso_plan["id_course"]
    equivalentes = limpiar_lista(curso_plan.get("equivalent", []))
    grupos_horario = buscar_grupos(codigo, equivalentes)

    grupos = [
        {
            "nombre":          g["nombre"],
            "numero_grupo":    g["grupo"],
            "bloques_horario": g["bloques_horario"],
            "profesores":      g["profesores"],
            "tipo_grupo":      g["tipo_grupo"],
        }
        for g in grupos_horario
    ] if grupos_horario else []

    return {
        "codigo":      codigo,
        "nombre":      curso_plan["name"],
        "creditos":    curso_plan["credits"],
        "carreras":    [carrera],           # se expande si el curso aparece en ambas carreras
        "requisitos":  limpiar_lista(curso_plan.get("requirements", [])),
        "correquisitos": limpiar_lista(curso_plan.get("co_requirements", [])),
        "grupos":      grupos,
        "choca_con":   [],
        "estudiante_puede_matricular": False,
    }

#  Procesar planes de estudio 

# catalogo_index guarda codigo y posición en la lista, para detectar duplicados
catalogo: list[dict] = []
catalogo_index: dict[str, int] = {}

for plan, nombre_carrera in [
    (plan_computadores, NOMBRE_COMPUTADORES),
    (plan_electronica,  NOMBRE_ELECTRONICA),
]:
    for nivel in plan["levels"]:
        if nivel["id"] not in SEMESTRES_INCLUIDOS:
            continue

        for curso_plan in nivel["courses"]:
            codigo = curso_plan["id_course"]

            if codigo in catalogo_index:
                # El curso ya existe, solo agregar la segunda carrera si no está ya
                entrada_existente = catalogo[catalogo_index[codigo]]
                if nombre_carrera not in entrada_existente["carreras"]:
                    entrada_existente["carreras"].append(nombre_carrera)
            else:
                # Curso nuevo, construir entrada y agregar al catálogo
                entrada = construir_entrada(curso_plan, nombre_carrera)
                catalogo_index[codigo] = len(catalogo)
                catalogo.append(entrada)

#  Guardar resultado 

with open(SALIDA_FILE, "w", encoding="utf-8") as f:
    json.dump(catalogo, f, ensure_ascii=False, indent=2)

#  Resumen 

total          = len(catalogo)
compartidos    = sum(1 for c in catalogo if len(c["carreras"]) == 2)
solo_comp      = sum(1 for c in catalogo if c["carreras"] == [NOMBRE_COMPUTADORES])
solo_elec      = sum(1 for c in catalogo if c["carreras"] == [NOMBRE_ELECTRONICA])


print(f"    Total cursos únicos         : {total}")
print(f"    Solo Computadores           : {solo_comp}")
print(f"    Solo Electrónica            : {solo_elec}")
print(f"    Compartidos por ambas       : {compartidos}")
