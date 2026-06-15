# sysembarcados

Repositório do trabalho da disciplina de Sistemas Embarcados da Escola
Politécnica da USP.

## Projeto

O projeto implementa um controle remoto com ESP32 usando:

- Bluetooth HID
- FreeRTOS
- Programacao orientada a objetos

Hardware principal:

- 4 botoes
- 1 joystick analogico
- 1 IMU
- 1 motor de vibracao

## Funcionalidades implementadas

- envio de joystick via Bluetooth HID
- envio dos 4 botoes via Bluetooth HID
- leitura da IMU
- envio dos dados da IMU no relatorio HID
- vibracao local por comando serial
- vibracao remota via Bluetooth HID Output Report

## Estrutura principal

- `ControleESP32/ControleESP32.ino`: ponto de entrada do firmware
- `ControleESP32/src/controller/`: coordenacao geral e tarefas do FreeRTOS
- `ControleESP32/src/bluetooth/`: integracao com `BleGamepad`
- `ControleESP32/src/imu/`: leitura do sensor inercial
- `ControleESP32/src/haptics/`: controle do motor de vibracao

## Teste rapido

### Controle Bluetooth

1. Grave o firmware no ESP32.
2. Pareie o dispositivo `ESP32 Gamepad` no Windows.
3. Verifique joystick e botoes em `joy.cpl`.

### Vibracao local

No monitor serial a `115200`, envie:

- `VIB ON`
- `VIB OFF`
- `VIB:180`

### Vibracao remota por Bluetooth

O script de teste esta em:

- `ControleESP32/tools/test_rumble.py`

Instalacao da dependencia no Windows:

```bash
py -m pip install pywinusb
```

Execucao:

```bash
py ControleESP32\tools\test_rumble.py
```

O script procura o controle HID exposto no Windows e envia:

- intensidade `255`
- espera `2` segundos
- intensidade `0`

## Ponte para jogos comerciais

Para jogos que nao reconhecem bem o HID generico do ESP32, existe uma
ponte no PC:

- `ControleESP32/tools/bridge_xinput.py`

Ela faz:

- leitura do `ESP32 Gamepad` via HID no Windows
- criacao de um controle virtual Xbox 360 via `vgamepad`
- envio do rumble do jogo de volta para o ESP32

Dependencias:

```bash
py -m pip install pywinusb vgamepad
```

Execucao:

```bash
py ControleESP32\tools\bridge_xinput.py
```

Modo para jogos de corrida:

```bash
py ControleESP32\tools\bridge_xinput.py --mode racing
```

No modo `racing`:

- IMU controla a direcao
- joystick Y vira acelerador/freio analogico
- botao 1 tambem acelera no maximo
- botao 2 tambem freia no maximo
- rumble do jogo volta para o ESP32 pela ponte
