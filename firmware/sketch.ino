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

void setup() {}

void loop() {}
