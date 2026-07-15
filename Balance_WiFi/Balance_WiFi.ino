#define ARCH_ESPRESSIF
#include "HX711.h"
#include <WiFi.h>
#include <HTTPClient.h>

// ─── Configurações WiFi e Servidor ───────────────────────────────────────────
const char* ssid     = "TORIGA";
const char* password = "vini2003";
const char* url      = "http://192.168.0.5:5000/dado"; // IP do seu PC

// ─── Pinos HX711 ─────────────────────────────────────────────────────────────
const int DOUT_PIN = 16;
const int SCK_PIN  = 4;

// ─── Variáveis da balança ─────────────────────────────────────────────────────
long int TR;
long int TR1;
int Nmed = 20;
int i;
long leitura;
long leitura1;
unsigned long ms1;
int TimeTR = 50000;
HX711 scale;
#define BatLen 34
float X = 1719.0 / 214150.0;
int peso;

// ─── Intervalo entre medidas ─────────────────────────────────────────────────
const unsigned long INTERVALO_MEDIDA = 5000; // 5 segundos
unsigned long ultimaMedida = 0;

void setup() {
  Serial.begin(115200);
 
  // ── Conecta WiFi ──────────────────────────────────────────────────────────
  Serial.print("Conectando WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado! IP: " + WiFi.localIP().toString());

  // ── Inicializa HX711 ──────────────────────────────────────────────────────
  scale.begin(DOUT_PIN, SCK_PIN);
  while (!scale.is_ready()) {
    Serial.println("HX711 nao encontrado.");
    delay(1000);
  }

  delay(2000);
  tara();
  ms1 = millis();
}

void loop() {
  // ── Só mede/envia a cada INTERVALO_MEDIDA (não-bloqueante) ─────────────────
  if (millis() - ultimaMedida < INTERVALO_MEDIDA) {
    return; // aguarda os 5s sem travar o núcleo/WiFi
  }
  ultimaMedida = millis();

  uint16_t adc = analogRead(BatLen);
  Serial.println(adc);

  ler();

  peso = ((float)leitura - (float)TR) * X;

  Serial.print("Leitura: ");   Serial.println(leitura - TR);
  Serial.print("Peso (g): ");  Serial.println(peso);
  Serial.print("Peso (kg): "); Serial.println((float)peso / 1000.0);

  enviarPeso(peso);
}

// ─── Envia o peso em gramas via HTTP POST ────────────────────────────────────
void enviarPeso(int valorGramas) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado, pulando envio.");
    return;
  }

  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(1000); // timeout de 1s para não travar o loop

  String payload = "{\"valor\":" + String(valorGramas) + "}";
  int httpCode = http.POST(payload);

  if (httpCode > 0) {
    Serial.printf("Enviado: %dg | HTTP: %d\n", valorGramas, httpCode);
  } else {
    Serial.printf("Erro no envio: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}

// ─── Tara ─────────────────────────────────────────────────────────────────────
void tara() {
  TR = 0;
  for (i = 0; i < Nmed; i++) {
    while (!scale.is_ready()) {}
    TR1 = scale.read();
    Serial.print("TR1: "); Serial.println(TR1);
    TR = TR + TR1;
  }
  TR = TR / Nmed;
  Serial.print("Tara: "); Serial.println(TR);
}

// ─── Leitura média ───────────────────────────────────────────────────────────
void ler() {
  leitura = 0;
  for (i = 0; i < Nmed; i++) {
    while (!scale.is_ready()) {}
    leitura1 = scale.read();
    leitura = leitura + leitura1;
  }
  leitura = leitura / Nmed;
  Serial.print("Leitura CRUA: "); Serial.println(leitura);
}