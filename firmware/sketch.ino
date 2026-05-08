// DoorFlow - Contagem de pessoas com ESP32
// CP2 - IoT

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

// wifi
const char* WIFI_SSID = "SEU_WIFI";
const char* WIFI_PASSWORD = "SUA_SENHA";

// pinos dos componentes
#define SENSOR_A 34
#define SENSOR_B 35
#define LED_VERDE 26
#define LED_VERMELHO 27
#define LED_AZUL 25

// tempos
#define TIMEOUT_SEQUENCIA 3000
#define INTERVALO_SYNC 600000
#define DEBOUNCE 300

// lcd
LiquidCrystal_I2C lcd(0x27, 16, 2);

// contadores
int entradas = 0;
int saidas = 0;
int porHora[24] = {0};

// estado dos sensores
enum Estado { IDLE, A_ATIVO, B_ATIVO };
Estado estadoAtual = IDLE;
unsigned long tempoUltimoSensor = 0;

// debounce
unsigned long ultimoA = 0;
unsigned long ultimoB = 0;

// horario
unsigned long unixBase = 0;
unsigned long milliBase = 0;
bool horarioSincronizado = false;
unsigned long ultimoSync = 0;

// lcd
unsigned long ultimoLCD = 0;
int tela = 0;

bool sincronizarHorario() {
  if (WiFi.status() != WL_CONNECTED) return false;

  HTTPClient http;
  http.begin("http://worldtimeapi.org/api/timezone/America/Sao_Paulo");
  http.setTimeout(8000);

  int code = http.GET();
  if (code != 200) {
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  StaticJsonDocument<512> doc;
  DeserializationError erro = deserializeJson(doc, payload);
  if (erro) return false;

  unsigned long unix = doc["unixtime"].as<unsigned long>();
  if (unix == 0) return false;

  unixBase = unix;
  milliBase = millis();
  horarioSincronizado = true;
  ultimoSync = millis();

  Serial.println("horario sincronizado: " + String(unix));
  return true;
}

unsigned long getUnixAtual() {
  if (!horarioSincronizado) return 0;
  return unixBase + (millis() - milliBase) / 1000;
}

void formatarTimestamp(char* buf) {
  unsigned long t = getUnixAtual();
  if (t == 0) {
    strcpy(buf, "sem horario");
    return;
  }
  t -= 3 * 3600;
  struct tm* info = gmtime((time_t*)&t);
  strftime(buf, 20, "%Y-%m-%d %H:%M:%S", info);
}

int getHoraAtual() {
  unsigned long t = getUnixAtual();
  if (t == 0) return -1;
  t -= 3 * 3600;
  struct tm* info = gmtime((time_t*)&t);
  return info->tm_hour;
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);

  pinMode(SENSOR_A, INPUT);
  pinMode(SENSOR_B, INPUT);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lcd.setCursor(0, 1);
  lcd.print("Conectando wifi");

  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi conectado!");
    digitalWrite(LED_AZUL, HIGH);
    lcd.clear();
    lcd.print("WiFi OK!");
    delay(1000);
    sincronizarHorario();
  } else {
    Serial.println("WiFi falhou, sem horario");
    lcd.clear();
    lcd.print("Sem WiFi");
    delay(1500);
  }

  lcd.clear();
  lcd.print("DoorFlow pronto");
  delay(1000);
  lcd.clear();
}

void loop() {}
