from flask import Flask, request, jsonify
from flask_cors import CORS
import oracledb
from datetime import datetime

app = Flask(__name__)
CORS(app)

# =========================
# CONFIGURACAO DO BANCO
# =========================
DB_USER     = "SEU_USUARIO"
DB_PASSWORD = "SUA_SENHA"
DB_DSN      = "localhost/XEPDB1"  # ajusta se necessário

def get_connection():
    return oracledb.connect(
        user=DB_USER,
        password=DB_PASSWORD,
        dsn=DB_DSN
    )

# =========================
# ROTA DE TESTE
# =========================
@app.route("/", methods=["GET"])
def index():
    return jsonify({"status": "DoorFlow API rodando"})

# =========================
# REGISTRA EVENTO
# =========================
@app.route("/evento", methods=["POST"])
def registrar_evento():
    dados = request.get_json()

    if not dados:
        return jsonify({"erro": "JSON invalido"}), 400

    tipo      = dados.get("tipo")
    timestamp = dados.get("timestamp")
    entradas  = dados.get("entradas", 0)
    saidas    = dados.get("saidas", 0)
    saldo     = dados.get("saldo", 0)

    if not tipo or not timestamp:
        return jsonify({"erro": "Campos obrigatorios: tipo, timestamp"}), 400

    try:
        conn = get_connection()
        cursor = conn.cursor()

        cursor.execute("""
            INSERT INTO eventos (tipo, timestamp, entradas, saidas, saldo)
            VALUES (:1, :2, :3, :4, :5)
        """, (tipo, timestamp, entradas, saidas, saldo))

        conn.commit()
        cursor.close()
        conn.close()

        return jsonify({"status": "ok", "mensagem": "Evento registrado"}), 201

    except Exception as e:
        return jsonify({"erro": str(e)}), 500

# =========================
# LISTA EVENTOS
# =========================
@app.route("/eventos", methods=["GET"])
def listar_eventos():
    try:
        conn = get_connection()
        cursor = conn.cursor()

        cursor.execute("""
            SELECT id, tipo, timestamp, entradas, saidas, saldo, criado_em
            FROM eventos
            ORDER BY criado_em DESC
        """)

        rows = cursor.fetchall()
        cursor.close()
        conn.close()

        eventos = []
        for row in rows:
            eventos.append({
                "id":        row[0],
                "tipo":      row[1],
                "timestamp": row[2],
                "entradas":  row[3],
                "saidas":    row[4],
                "saldo":     row[5],
                "criado_em": str(row[6])
            })

        return jsonify(eventos), 200

    except Exception as e:
        return jsonify({"erro": str(e)}), 500

# =========================
# HORA DE PICO
# =========================
@app.route("/pico", methods=["GET"])
def hora_de_pico():
    try:
        conn = get_connection()
        cursor = conn.cursor()

        cursor.execute("""
            SELECT hora, total
            FROM contagem_por_hora
            ORDER BY total DESC
            FETCH FIRST 1 ROWS ONLY
        """)

        row = cursor.fetchone()
        cursor.close()
        conn.close()

        if not row:
            return jsonify({"mensagem": "sem dados"}), 200

        return jsonify({"hora": row[0], "total": row[1]}), 200

    except Exception as e:
        return jsonify({"erro": str(e)}), 500

if __name__ == "__main__":
    app.run(debug=True, port=5000)
