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
#define SENSOR_A 18
#define SENSOR_B 19
#define LED_VERDE 26
#define LED_VERMELHO 27
#define LED_AZUL 25

// tempos
#define TIMEOUT_SEQUENCIA 3000
#define INTERVALO_SYNC 600000
#define DEBOUNCE 300
#define INTERVALO_SIMULACAO 4000

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

// simulacao
unsigned long ultimaSimulacao = 0;
int etapaSimulacao = 0;

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
      if (horarioSincronizado) {
        lcd.print(ts + 11);
      } else {
        lcd.print("sem sync");
      }
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

void simularPessoa() {
  unsigned long agora = millis();
  if (agora - ultimaSimulacao < INTERVALO_SIMULACAO) return;
  ultimaSimulacao = agora;

  if (etapaSimulacao % 2 == 0) {
    Serial.println("[sim] entrada");
    entradas++;
    registrarEvento("ENTRADA");
    digitalWrite(LED_VERDE, HIGH);
    delay(1500);
    digitalWrite(LED_VERDE, LOW);
  } else {
    Serial.println("[sim] saida");
    saidas++;
    registrarEvento("SAIDA");
    digitalWrite(LED_VERMELHO, HIGH);
    delay(1500);
    digitalWrite(LED_VERMELHO, LOW);
  }

  etapaSimulacao++;
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

void loop() {
  simularPessoa();
  atualizarLCD();
  delay(20);
}
