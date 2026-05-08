#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// =========================
// CONFIGURACAO DO WIFI
// =========================
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

// =========================
// URL DA API
// =========================
// Temperatura atual de Sao Paulo
const char* WEATHER_URL =
  "https://api.open-meteo.com/v1/forecast?latitude=-23.55&longitude=-46.63&current=temperature_2m";


// =========================
// Variaveis
// ========================
unsigned long lastRequestTime = 0;
const unsigned long REQUEST_INTERVAL = 10000;


// =========================
// CONECTA NO WIFI
// =========================
void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);

  Serial.print("Conectando ao WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void ensureWiFiConnected() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado. Tentando reconectar...");
    connectWiFi();
  }
}

// =========================
// FAZ A REQUISICAO GET
// =========================
void makeGetRequest() {
  HTTPClient http;

  Serial.println("\n--- FAZENDO GET NA API OPEN-METEO ---");
  http.begin(WEATHER_URL);

  int httpCode = http.GET();

  Serial.print("Status HTTP: ");
  Serial.println(httpCode);

  if (httpCode <= 0) {
    Serial.println("Erro na requisicao");
    http.end();
    return;
  }

  String payload = http.getString();
  http.end();

  Serial.println("\nJSON recebido:");
  Serial.println(payload);

  // Exemplo esperado:
  // {
  //   "current": {
  //     "temperature_2m": 24.3
  //   }
  // }

  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print("Erro ao interpretar JSON: ");
    Serial.println(error.c_str());
    return;
  }

  JsonVariant temperatureNode = doc["current"]["temperature_2m"];

  if (temperatureNode.isNull()) {
    Serial.println("Campo current.temperature_2m nao encontrado");
    return;
  }

  float temperatureC = temperatureNode;

  Serial.println("\nTemperatura extraida do JSON:");
  Serial.print(temperatureC, 1);
  Serial.println(" C");
}

void setup() {
  Serial.begin(115200);

  connectWiFi();
  makeGetRequest();
}

void loop() {
  unsigned long now = millis();

  if (now - lastRequestTime >= REQUEST_INTERVAL) {
    lastRequestTime = now;

    ensureWiFiConnected();
    makeGetRequest();
  }
}