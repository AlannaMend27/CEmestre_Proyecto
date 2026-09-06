#Este archivo combina los horarios (horarios_raw.json) con el plan de estudios
#del TEC-Digital para generar el archivo de entrada final para el proyecto CEmestre

import json

HORARIOS_FILE   = "horarios_raw.json"
PLAN_FILE       = "plan_estudios.json"  
SALIDA_FILE     = "catalogo_cursos_electronica.json"

SEMESTRES_MAX   = 4   

# Carga de datos

# cargar archivo que tiene todos los horarios disponibles de la escuela
with open(HORARIOS_FILE, encoding="utf-8") as f:
    horarios = json.load(f)

# cargar archivo qu etiene el plan de la carrera (se obtiene de inspeccionar la pagina web del tec digital guia horarios)
with open(PLAN_FILE, encoding="utf-8") as f:
    plan = json.load(f)

# Índice de horarios por código
# Un curso puede tener varios grupos, así que guardamos lista de grupos.
horarios_por_codigo: dict[str, list] = {}
for curso in horarios:
    cod = curso["codigo"]
    horarios_por_codigo.setdefault(cod, []).append(curso)

#  Procesar solo semestres 0-4 del plan 
SEMESTRES_INCLUIDOS = {"Semestre 0", "Semestre 1", "Semestre 2",
                       "Semestre 3", "Semestre 4"}

def buscar_grupos(codigo: str, equivalentes: list[str]) -> list:
    """Busca grupos por código principal y todos sus equivalentes disponibles."""
    resultados = []
    
    # Primero el código principal
    if codigo in horarios_por_codigo:
        resultados.extend(horarios_por_codigo[codigo])
    
    # Luego todos los equivalentes disponibles
    for eq in equivalentes:
        if eq in horarios_por_codigo:
            resultados.extend(horarios_por_codigo[eq])
    
    return resultados


def limpiar_lista(lista: list[dict]) -> list[str]:
    """Convierte [{"id": "CE1101"}, ...] a ["CE1101", ...] eliminando el placeholder No hay materias equivalentes en el plan."""
    return [
        item["id"] for item in lista
        if item.get("id") and "No hay" not in item["id"]
    ]

catalogo = []

for nivel in plan["levels"]:
    if nivel["id"] not in SEMESTRES_INCLUIDOS:
        continue

    semestre_num = nivel["id"].split()[-1]   # "0", "1", ..., "4"

    for curso_plan in nivel["courses"]:

        # obtener codigo del curso
        codigo = curso_plan["id_course"]

        # obtener equivalentes y buscarlos todos en caso de que existan
        equivalentes = limpiar_lista(curso_plan.get("equivalent", []))
        grupos_horario = buscar_grupos(codigo, equivalentes)

        # Si no hay horario este semestre, igual lo incluimos con grupos vacíos
        if grupos_horario:
            grupos = [
                {
                    "nombre": g["nombre"],
                    "numero_grupo": g["grupo"],
                    "bloques_horario": g["bloques_horario"],
                    "profesores": g["profesores"],
                    "tipo_grupo": g["tipo_grupo"],
                }
                for g in grupos_horario
            ]
        else:
            grupos = []

        entrada = {
            # Identificación 
            "codigo":        codigo,
            "nombre":        curso_plan["name"],
            "creditos":      curso_plan["credits"],

            #  Requisitos y correquisitos 
            "requisitos":     limpiar_lista(curso_plan.get("requirements", [])),
            "correquisitos":  limpiar_lista(curso_plan.get("co_requirements", [])),

            #  Horarios por grupo 
            # Lista de grupos ofertados este semestre y sus horarios.
            "grupos":        grupos,

            # Campos vacios para el programa en C
            "choca_con":             [], 
            "estudiante_puede_matricular": False,
        }
        catalogo.append(entrada)


#  Guardar resultado 
with open(SALIDA_FILE, "w", encoding="utf-8") as f:
    json.dump(catalogo, f, ensure_ascii=False, indent=2)

print(f"  {SALIDA_FILE} generado con {len(catalogo)} cursos")
print(f"    (de {SEMESTRES_MAX} semestres del plan de estudios)")
print()

# Resumen rápido
con_horario    = sum(1 for c in catalogo if c["grupos"])
sin_horario    = sum(1 for c in catalogo if not c["grupos"])
con_requisitos = sum(1 for c in catalogo if c["requisitos"])
