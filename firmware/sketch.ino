// DoorFlow - Contagem de pessoas com ESP32
// CP2 - IoT

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

// =========================
// CONFIGURACAO DO WIFI
// =========================
#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""
#define WIFI_CHANNEL 6

// =========================
// URL DA API EXTERNA
// =========================
const char* TIME_URL =
  "https://timeapi.io/api/time/current/zone?timeZone=America/Sao_Paulo";

// =========================
// PINOS
// =========================
#define SENSOR_A 13
#define SENSOR_B 14
#define LED_ENTRADA 26
#define LED_SAIDA 27
#define BTN_RESET 32

// =========================
// CONSTANTES
// =========================
#define TIMEOUT_SEQUENCIA 3000
#define DEBOUNCE 300
#define INTERVALO_SYNC 600000

// =========================
// LCD
// =========================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// =========================
// VARIAVEIS
// =========================
int entradas = 0;
int saidas = 0;
int porHora[24] = {0};

enum Estado { IDLE, A_ATIVO, B_ATIVO };
Estado estadoAtual = IDLE;
unsigned long tempoUltimoSensor = 0;

unsigned long ultimoA = 0;
unsigned long ultimoB = 0;

unsigned long unixBase = 0;
unsigned long milliBase = 0;
bool horarioSincronizado = false;
unsigned long ultimoSync = 0;

unsigned long ultimoLCD = 0;
int tela = 0;

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
    Serial.println("WiFi desconectado. Reconectando...");
    connectWiFi();
  }
}

// =========================
// SINCRONIZA HORARIO
// =========================
void sincronizarHorario() {
  ensureWiFiConnected();

  HTTPClient http;
  Serial.println("\n--- SINCRONIZANDO HORARIO ---");
  http.begin(TIME_URL);

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

  DynamicJsonDocument doc(2048);
  DeserializationError erro = deserializeJson(doc, payload);

  if (erro) {
    Serial.print("Erro ao interpretar JSON: ");
    Serial.println(erro.c_str());
    return;
  }

  JsonVariant horaNode = doc["hour"];
  JsonVariant minNode  = doc["minute"];
  JsonVariant segNode  = doc["seconds"];
  JsonVariant diaNode  = doc["date"];

  if (horaNode.isNull()) {
    Serial.println("Campos de horario nao encontrados");
    return;
  }

  int hora    = horaNode.as<int>();
  int minuto  = minNode.as<int>();
  int segundo = segNode.as<int>();
  String data = diaNode.as<String>();

  char ts[20];
  snprintf(ts, sizeof(ts), "%s %02d:%02d:%02d",
    data.c_str(), hora, minuto, segundo);

  Serial.print("Horario sincronizado: ");
  Serial.println(ts);

  unixBase = hora * 3600 + minuto * 60 + segundo;
  milliBase = millis();
  horarioSincronizado = true;
  ultimoSync = millis();
}

// =========================
// UTILITARIOS DE HORARIO
// =========================
void formatarTimestamp(char* buf) {
  if (!horarioSincronizado) {
    strcpy(buf, "sem horario");
    return;
  }
  unsigned long segundosPassados = (millis() - milliBase) / 1000;
  unsigned long totalSegundos = unixBase + segundosPassados;

  int h = (totalSegundos / 3600) % 24;
  int m = (totalSegundos % 3600) / 60;
  int s = totalSegundos % 60;

  snprintf(buf, 20, "%02d:%02d:%02d", h, m, s);
}

int getHoraAtual() {
  if (!horarioSincronizado) return -1;
  unsigned long segundosPassados = (millis() - milliBase) / 1000;
  unsigned long totalSegundos = unixBase + segundosPassados;
  return (totalSegundos / 3600) % 24;
}

// =========================
// REGISTRA EVENTO
// =========================
void registrarEvento(const char* tipo) {
  char ts[20];
  formatarTimestamp(ts);

  int hora = getHoraAtual();
  if (hora >= 0) porHora[hora]++;

  Serial.print("{\"tipo\":\"");
  Serial.print(tipo);
  Serial.print("\",\"timestamp\":\"");
  Serial.print(ts);
  Serial.print("\",\"entradas\":");
  Serial.print(entradas);
  Serial.print(",\"saidas\":");
  Serial.print(saidas);
  Serial.print(",\"saldo\":");
  Serial.println(entradas - saidas);
}

// =========================
// ATUALIZA LCD
// =========================
void atualizarLCD() {
  if (millis() - ultimoLCD < 3000) return;
  ultimoLCD = millis();
  tela = (tela + 1) % 3;

  lcd.clear();
  switch (tela) {
    case 0:
      lcd.setCursor(0, 0);
      lcd.print("Ent:");
      lcd.print(entradas);
      lcd.setCursor(8, 0);
      lcd.print("Sai:");
      lcd.print(saidas);
      lcd.setCursor(0, 1);
      lcd.print("Saldo: ");
      lcd.print(entradas - saidas);
      break;

    case 1: {
      char ts[20];
      formatarTimestamp(ts);
      lcd.setCursor(0, 0);
      lcd.print("Horario:");
      lcd.setCursor(0, 1);
      lcd.print(ts);
      break;
    }

    case 2: {
      int picoHora = 0, picoVal = 0;
      for (int h = 0; h < 24; h++) {
        if (porHora[h] > picoVal) {
          picoVal = porHora[h];
          picoHora = h;
        }
      }
      lcd.setCursor(0, 0);
      lcd.print("Pico de mov:");
      lcd.setCursor(0, 1);
      if (picoVal == 0) {
        lcd.print("sem dados");
      } else {
        lcd.print(picoHora < 10 ? "0" : "");
        lcd.print(picoHora);
        lcd.print(":00 (");
        lcd.print(picoVal);
        lcd.print("mov)");
      }
      break;
    }
  }
}

// =========================
// VERIFICA SENSORES
// =========================
void verificarSensores() {
  unsigned long agora = millis();

  static bool anteriorA = LOW;
  static bool anteriorB = LOW;

  bool leituraA = digitalRead(SENSOR_A);
  bool leituraB = digitalRead(SENSOR_B);

  bool trigA = (leituraA == HIGH && anteriorA == LOW) && (agora - ultimoA > DEBOUNCE);
  bool trigB = (leituraB == HIGH && anteriorB == LOW) && (agora - ultimoB > DEBOUNCE);

  if (trigA) ultimoA = agora;
  if (trigB) ultimoB = agora;

  anteriorA = leituraA;
  anteriorB = leituraB;

  if (estadoAtual != IDLE && (agora - tempoUltimoSensor > TIMEOUT_SEQUENCIA)) {
    Serial.println("timeout, resetando sequencia");
    estadoAtual = IDLE;
  }

  switch (estadoAtual) {
    case IDLE:
      if (trigA) {
        estadoAtual = A_ATIVO;
        tempoUltimoSensor = agora;
        Serial.println("sensor A ativado");
      } else if (trigB) {
        estadoAtual = B_ATIVO;
        tempoUltimoSensor = agora;
        Serial.println("sensor B ativado");
      }
      break;

    case A_ATIVO:
      if (trigB) {
        entradas++;
        registrarEvento("ENTRADA");
        digitalWrite(LED_ENTRADA, HIGH);
        delay(500);
        digitalWrite(LED_ENTRADA, LOW);
        estadoAtual = IDLE;
      }
      break;

    case B_ATIVO:
      if (trigA) {
        saidas++;
        registrarEvento("SAIDA");
        digitalWrite(LED_SAIDA, HIGH);
        delay(500);
        digitalWrite(LED_SAIDA, LOW);
        estadoAtual = IDLE;
      }
      break;
  }
}

// =========================
// SETUP
// =========================
void setup() {
  Serial.begin(115200);

  pinMode(LED_ENTRADA, OUTPUT);
  pinMode(LED_SAIDA, OUTPUT);
  pinMode(SENSOR_A, INPUT);
  pinMode(SENSOR_B, INPUT);
  pinMode(BTN_RESET, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("DoorFlow");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");

  connectWiFi();
  sincronizarHorario();

  lcd.clear();
  lcd.print("Pronto!");
  delay(1000);
  lcd.clear();
}

// =========================
// LOOP
// =========================
void loop() {
  unsigned long agora = millis();

  if (agora - ultimoSync >= INTERVALO_SYNC) {
    sincronizarHorario();
  }

  if (digitalRead(BTN_RESET) == LOW) {
    entradas = 0;
    saidas = 0;
    memset(porHora, 0, sizeof(porHora));
    Serial.println("contadores resetados");
    lcd.clear();
    lcd.print("Reset!");
    delay(1000);
    lcd.clear();
  }

  verificarSensores();
  atualizarLCD();
  delay(20);
}
