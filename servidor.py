from flask import Flask, request, jsonify, send_from_directory
from datetime import datetime

app = Flask(__name__)

ultimo_valor = None
historico = []  # lista de {"timestamp": ms, "valor": gramas}

@app.route('/dado', methods=['POST'])
def receber_dado():
    global ultimo_valor
    dados = request.get_json()
    ultimo_valor = dados.get('valor')
    historico.append({
        "timestamp": datetime.now().timestamp() * 1000,  # ms para o JS
        "valor": ultimo_valor
    })
    print(f"Recebido: {ultimo_valor}g | Total: {len(historico)} pontos")
    return jsonify({"status": "ok"})

@app.route('/dado', methods=['GET'])
def enviar_dado():
    return jsonify({"valor": ultimo_valor})

@app.route('/historico', methods=['GET'])
def enviar_historico():
    return jsonify(historico)

@app.route('/')
def dashboard():
    return send_from_directory('.', 'dashboard.html')

if __name__ == '__main__':
    app.run(host='192.168.0.5', port=5000)