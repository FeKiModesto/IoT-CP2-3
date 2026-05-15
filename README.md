# DoorFlow

Sistema de contagem direcional de pessoas com ESP32. Detecta entradas e saídas através de dois sensores PIR posicionados em sequência na porta — se a pessoa ativa o Sensor A e depois o Sensor B, é uma entrada; B depois A, é uma saída.

---

## Sobre o projeto

Este projeto está sendo desenvolvido como CP2/3 da disciplina **Disruptive Architectures: IoT IOB e Generative IA**, do primeiro semestre. O objetivo é criar um sistema funcional de contagem direcional de pessoas utilizando ESP32 e sensores PIR.

O protótipo simulado no Wokwi serve de base para a confecção do projeto real — toda a lógica de detecção, sincronização de horário e registro de eventos foi validada na simulação antes de ser aplicada ao hardware físico.

---

## Como funciona
```
[ Lado interno ]  [ Porta ]  [ Lado externo ]
Sensor A   ←————————————→   Sensor B
A → B  =  ENTRADA
B → A  =  SAÍDA
```

O ESP32 sincroniza o horário real via [TimeAPI](https://timeapi.io) e registra cada evento com timestamp completo (data + hora). Os dados são enviados para a API própria e armazenados no banco de dados Oracle. As contagens ficam acumuladas por hora para calcular o horário de pico de movimentação.

---

## Funcionalidades

- Detecção direcional de entrada e saída
- Timestamp real com data completa via TimeAPI (UTC-3 / Brasília)
- Re-sincronização automática de horário a cada 10 minutos
- Envio automático de eventos para API REST com retry em caso de falha
- Armazenamento dos eventos no banco de dados Oracle
- LCD 16x2 com 3 telas rotativas:
  - Contadores de entrada, saída e saldo
  - Horário atual
  - Hora de pico do dia
- LEDs de feedback para cada evento
- Log em formato JSON via Serial Monitor

---

## Hardware

| Componente | Modelo |
|---|---|
| Microcontrolador | ESP32 DevKit C v4 |
| Sensores de movimento | PIR × 2 |
| Display | LCD 16x2 + módulo I2C |
| LEDs | Verde, Vermelho |
| Resistores | 1kΩ × 2 |

Diagrama completo de ligações em [`hardware/wiring_diagram.md`](hardware/wiring_diagram.md).

---

## Simulação

O projeto pode ser simulado no Wokwi sem precisar do hardware físico.

[![Simular no Wokwi](https://img.shields.io/badge/Simular-Wokwi-blue?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCAyNCAyNCI+PHBhdGggZmlsbD0id2hpdGUiIGQ9Ik0xMiAyQzYuNDggMiAyIDYuNDggMiAxMnM0LjQ4IDEwIDEwIDEwIDEwLTQuNDggMTAtMTBTMTcuNTIgMiAxMiAyem0tMiAxNGwtNC00IDEuNDEtMS40MUwxMCAxMy4xN2w2LjU5LTYuNTlMMTggOGwtOCA4eiIvPjwvc3ZnPg==)](https://wokwi.com/projects/463409131964863489)

---

## API

A API REST foi desenvolvida em Python com Flask e está hospedada no Render.

**Base URL:** `https://doorflow-api.onrender.com`

| Método | Endpoint | Descrição |
|---|---|---|
| GET | `/` | Status da API |
| POST | `/evento` | Registra um evento de entrada ou saída |
| GET | `/eventos` | Lista todos os eventos registrados |
| GET | `/pico` | Retorna a hora de maior movimentação |

**Exemplo de payload enviado pelo ESP32:**
```json
{
  "tipo": "ENTRADA",
  "timestamp": "2026-05-15 07:07:34",
  "entradas": 1,
  "saidas": 0,
  "saldo": 1
}
```

---

## Banco de dados

Os eventos são armazenados no Oracle Database da FIAP. A tabela principal `eventos` guarda tipo, timestamp completo, contadores e saldo de cada detecção.

---

## Estrutura do repositório
```
IoT-CP2-3/
├── api/
│   ├── app.py              # API Flask
│   ├── requirements.txt    # dependências Python
│   └── Procfile            # configuração do Render
├── hardware/
│   └── wiring_diagram.md   # pinagem e lista de componentes
├── firmware/
│   └── sketch.ino          # código do ESP32
├── http-client/            # projeto base fornecido pelo professor
├── diagram.json            # circuito para simulação no Wokwi
└── README.md
```

---

## Bibliotecas necessárias

Instalar via Arduino IDE (Sketch → Include Library → Manage Libraries):

| Biblioteca | Autor |
|---|---|
| `LiquidCrystal_I2C` | Frank de Brabander |
| `ArduinoJson` | Benoit Blanchon |
| `WiFi` | Espressif (já inclusa no pacote ESP32) |
| `HTTPClient` | Espressif (já inclusa no pacote ESP32) |

---

## Log de eventos (Serial Monitor)

Cada evento gera uma linha JSON no Serial Monitor (115200 baud):

```json
{"tipo":"ENTRADA","timestamp":"2026-05-15 07:07:34","entradas":1,"saidas":0,"saldo":1}
{"tipo":"SAIDA","timestamp":"2026-05-15 07:07:48","entradas":1,"saidas":1,"saldo":0}
```

---

## Roadmap

- [x] Circuito e pinagem definidos
- [x] Simulação no Wokwi
- [x] Firmware — constantes e variáveis globais
- [x] Firmware — setup e inicialização
- [x] Firmware — sincronização de horário via TimeAPI
- [x] Firmware — máquina de estados dos sensores
- [x] Firmware — log de eventos com timestamp completo
- [x] Firmware — LCD com telas rotativas
- [x] Firmware — loop principal
- [x] API REST em Flask hospedada no Render
- [x] Integração com banco de dados Oracle
- [x] Desenvolvimento do projeto físico

---

## Integrantes

- Enrico Delesporte | RM: 565760
- Felipe Kirschner Modesto | RM: 561810
- Vitor Dias dos Santos | RM: 565422
