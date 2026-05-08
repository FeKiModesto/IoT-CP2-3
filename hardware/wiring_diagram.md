# Wiring Diagram
 
Referência completa de ligações entre os componentes do DoorFlow.
 
## Componentes
 
| Componente | Modelo | Qtd |
|---|---|---|
| Microcontrolador | ESP32 DevKit v1 | 1 |
| Sensor de movimento | HC-SR501 (PIR) | 2 |
| Display | LCD 16x2 + módulo I2C (0x27) | 1 |
| LED verde | 5mm | 1 |
| LED vermelho | 5mm | 1 |
| LED azul | 5mm | 1 |
| Resistor | 220Ω | 3 |
| Protoboard | 400 ou 830 pontos | 1 |
| Jumpers | Macho-Macho e Macho-Fêmea | ~20 |
 
---
 
## Pinagem ESP32
 
| Pino ESP32 | Componente | Função |
|---|---|---|
| GPIO 34 | Sensor A (PIR) — OUT | Detecção lado interno |
| GPIO 35 | Sensor B (PIR) — OUT | Detecção lado externo |
| GPIO 21 | LCD I2C — SDA | Dados I2C |
| GPIO 22 | LCD I2C — SCL | Clock I2C |
| GPIO 26 | LED verde (via 220Ω) | Indica entrada |
| GPIO 27 | LED vermelho (via 220Ω) | Indica saída |
| GPIO 25 | LED azul (via 220Ω) | Indica status Wi-Fi |
| 3V3 | PIR x2, LCD | Alimentação 3.3V |
| GND | PIR x2, LCD, LEDs | Terra comum |
 
---
 
## Sensores PIR (HC-SR501)
 
Cada sensor possui 3 pinos:
 
| Pino PIR | Conecta em |
|---|---|
| VCC | 3V3 do ESP32 |
| GND | GND do ESP32 |
| OUT | GPIO 34 (Sensor A) ou GPIO 35 (Sensor B) |
 
> **Posicionamento físico:** Sensor A fica do lado de dentro da porta, Sensor B do lado de fora. A sequência de ativação (A→B ou B→A) determina a direção do movimento.
 
---
 
## LCD 16x2 com módulo I2C
 
O módulo I2C elimina a necessidade de ligar os 6 pinos paralelos do LCD diretamente.
 
| Pino I2C | Conecta em |
|---|---|
| VCC | 3V3 do ESP32 |
| GND | GND do ESP32 |
| SDA | GPIO 21 |
| SCL | GPIO 22 |
 
> **Endereço padrão:** `0x27`. Se o LCD não inicializar, tente `0x3F`.
 
---
 
## LEDs
 
Cada LED é conectado em série com um resistor de 220Ω para limitar a corrente.
 
```
ESP32 GPIO → Resistor 220Ω → LED (anodo) → LED (catodo) → GND
```
 
| LED | GPIO | Evento |
|---|---|---|
| Verde | 26 | Entrada detectada |
| Vermelho | 27 | Saída detectada |
| Azul | 25 | Wi-Fi conectado / sync de horário |
 
---
 
## Simulação
 
O circuito completo pode ser simulado no [Wokwi](https://wokwi.com/projects/463409131964863489) usando o arquivo `diagram.json` na raiz do repositório.
