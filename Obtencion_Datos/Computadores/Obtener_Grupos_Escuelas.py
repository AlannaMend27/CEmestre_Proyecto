
# scraper_tec.py
# Este archivo descarga la guía de horarios de una escuela desde el TEC-Digital y la exporta a JSON.

import requests
from bs4 import BeautifulSoup
import json
import sys

# CONFIGURACIÓN 

# Creedenciales para entrar al tec digital, se copia de DevTools > Application > Cookies
AD_SESSION_ID = "10949566054%2c202083176%2c1%2c1788239105%20{985%201788253505%204AA3793A4A40D13D0996DDE7323AE4362CC39477}" 

# Escuelas a la que se le va a realizar la consulta
CONSULTAS = [
    {"sede": "CA", "carrera": "CES", "periodo": "2026_S_2"},
    {"sede": "CA", "carrera": "CI", "periodo": "2026_S_2"},
    {"sede": "CA", "carrera": "QU", "periodo": "2026_S_2"},
    {"sede": "CA", "carrera": "MA", "periodo": "2026_S_2"},
    {"sede": "CA", "carrera": "CS", "periodo": "2026_S_2"},
    {"sede": "CA", "carrera": "FI", "periodo": "2026_S_2"},
    {"sede": "CA", "carrera": "E", "periodo": "2026_S_2"},
    {"sede": "CA", "carrera": "FH", "periodo": "2025_V_1"},
    {"sede": "CA", "carrera": "PI", "periodo": "2026_S_2"},
    {"sede": "CA", "carrera": "CD", "periodo": "2026_S_2"},
    {"sede": "CA", "carrera": "SHO", "periodo": "2026_S_2"},
]

# Archivo de salida
ARCHIVO_SALIDA = "horarios_raw.json"

#URL de la guia de horario del tec
URL_ENDPOINT = "https://tecdigital.tec.ac.cr/tda-expediente-estudiantil/ajax/tabla_guia_horario"



def parsear_tabla(html: str, sede: str, carrera: str, periodo: str) -> list[dict]:
    """Convierte el HTML de la tabla en una lista de dicts con los campos del curso."""

    # crear arbol facil de navegar
    soup = BeautifulSoup(html, "html.parser")
    tabla = soup.find("table", {"id": "tguiaHorario"})

    if not tabla:
        print("No se encontró la tabla. Revisá las credenciales o los parámetros.")
        return []

    # encontrar filas
    filas = tabla.find("tbody").find_all("tr")
    registros = []

    # guardar cada fila en el .json ya parseada
    for fila in filas:
        celdas = [td.get_text(strip=True) for td in fila.find_all("td")]

        if len(celdas) < 9:
            continue

        # La tabla tiene una fila por cada bloque horario del mismo curso/grupo
        # así que acá guardamos cada fila como un registro independiente
        registros.append({
            "sede":        sede,
            "carrera":     carrera,
            "periodo":     periodo,
            "codigo":      celdas[0],
            "nombre":      celdas[1],
            "grupo":       celdas[2],
            "creditos":    int(celdas[3]) if celdas[3].isdigit() else celdas[3],
            "horario_raw": celdas[4],   # ej: "Lunes - 8:0:9:50"
            "aula":        celdas[5],
            "profesor":    celdas[6],
            "cupo":        int(celdas[7]) if celdas[7].isdigit() else celdas[7],
            "tipo_materia":celdas[8],
            "tipo_grupo":  celdas[9]  if len(celdas) > 9  else "",
            "reservados":  celdas[10] if len(celdas) > 10 else "",
        })

    return registros


def descargar(sesion_id: str, consultas: list[dict]) -> list[dict]:
    session = requests.Session()

    # Seteamos la cookie de sesión para ingresar al tec digital
    session.cookies.set("ad_session_id", sesion_id, domain="tecdigital.tec.ac.cr")

    # Headers mínimos para que el servidor no rechace el request
    session.headers.update({
        "User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) Chrome/151.0.0.0",
        "Referer":    "https://tecdigital.tec.ac.cr/tda-expediente-estudiantil/",
        "Origin":     "https://tecdigital.tec.ac.cr",
        "X-Requested-With": "XMLHttpRequest",
    })

    todos = []
    for q in consultas:

        sede, carrera, periodo = q["sede"], q["carrera"], q["periodo"]

        print(f"  Descargando: sede={sede}  carrera={carrera}  periodo={periodo}")

        resp = session.post(URL_ENDPOINT, data={
            "sede":    sede,
            "carrera": carrera,
            "periodo": periodo,
        })

        if resp.status_code != 200:
            print(f"Error HTTP {resp.status_code}")
            continue

        registros = parsear_tabla(resp.text, sede, carrera, periodo)
        todos.extend(registros)

    return todos


def agrupar_por_curso(registros: list[dict]) -> list[dict]:
    """ Agrupa las filas por (codigo, grupo) y coloca solo los datos necesarios para el proyecto (limpieza datos) """

    agrupado = {}

    for r in registros:
        key = (r["codigo"], r["grupo"], r["carrera"])
        if key not in agrupado:
            agrupado[key] = {
                "sede":        r["sede"],
                "carrera":     r["carrera"],
                "periodo":     r["periodo"],
                "codigo":      r["codigo"],
                "nombre":      r["nombre"],
                "grupo":       r["grupo"],
                "creditos":    r["creditos"],
                "bloques_horario": [],          # lista de strings con cada bloque
                "profesores":  [],              # lista de srings para profes disponibles
                "tipo_grupo":  r["tipo_grupo"],
            }
        entrada = agrupado[key]

        if r["horario_raw"] not in entrada["bloques_horario"]:

            entrada["bloques_horario"].append(r["horario_raw"])

        if r["profesor"] not in entrada["profesores"]:
            entrada["profesores"].append(r["profesor"])

    return list(agrupado.values())


if __name__ == "__main__":
    if AD_SESSION_ID == "PEGA_AQUI_TU_SESSION_ID":
        print("❌  Falta pegar el session ID. Leé las instrucciones al inicio del script.")
        sys.exit(1)

    print("=== Scraper Guía de Horarios TEC-Digital ===")
    registros_raw = descargar(AD_SESSION_ID, CONSULTAS)

    if not registros_raw:
        print("No se obtuvieron datos. Verificá el session ID y los parámetros.")
        sys.exit(1)

    cursos = agrupar_por_curso(registros_raw)
    print(f"\nTotal cursos únicos (código+grupo): {len(cursos)}")

    with open(ARCHIVO_SALIDA, "w", encoding="utf-8") as f:
        json.dump(cursos, f, ensure_ascii=False, indent=2)

    print(f" Guardado en '{ARCHIVO_SALIDA}'")
    print("\nEjemplo del primer curso:")
    print(json.dumps(cursos[0], ensure_ascii=False, indent=2))