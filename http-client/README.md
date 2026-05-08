# ESP32 HTTP Client Weather

## Descricao

Este projeto demonstra o ESP32 atuando como cliente HTTP para consumir uma API externa de clima 

Fluxo da aula:

1. Conectar o ESP32 no Wi-Fi.
2. Fazer requisicao HTTP GET para API publica (Open-Meteo).
3. Ler JSON com ArduinoJson.


## Endpoint utilizado

- `GET http://api.open-meteo.com/v1/forecast?latitude=-23.55&longitude=-46.63&current=temperature_2m`

## Como Executar

Para instrucoes gerais de configuracao e execucao no Wokwi, consulte o [README principal](../README.md) do repositorio.

### Passos especificos deste projeto

1. Abra a pasta `http-client/` no VS Code.
2. Compile `http-client.ino` no Arduino IDE e exporte o binario compilado para gerar a pasta `build`.
3. Abra o arquivo `diagram.json`.
4. Inicie a simulacao no Wokwi.
5. Observe o Serial Monitor para acompanhar status HTTP, parsing do JSON e regra aplicada.

## Dependencias

- Biblioteca `ArduinoJson` (instale pelo Gerenciador de Bibliotecas do Arduino IDE).

## Estrutura do Projeto

- `http-client.ino`: Codigo principal com cliente HTTP, parse JSON e regra de controle.
- `diagram.json`: Diagrama Wokwi com ESP32 e dois LEDs.
- `wokwi.toml`: Configuracao de Wi-Fi e firmware para simulacao.
- `build/`: Pasta gerada apos compilacao (opcional para simulacao).

## Desafios para os alunos

- Alterar cidade (latitude/longitude) e comparar comportamento.
- get de outros parametros (ex: umidade, vento).
- Exibir informaçoes no lcd
- Adicionar botao para atualizar manualmente a temperatura.
