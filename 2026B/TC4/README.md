# TC4 — Control de la ESP32-S3 desde Qt por WebSockets y JSON

Trabajo en Clase 4 (TC4) de **Diseño de Interfaces (I7262)** — Sesiones 10 y 11 (Semana 6).
Prepara la **Práctica 1** (robot móvil con puente H L298 controlado desde Qt).

La ESP32-S3 funciona como **servidor WebSocket** y la interfaz de Qt como **cliente**.
Intercambian mensajes **JSON**: la computadora manda **comandos** (encender salidas) y la
ESP32 manda **eventos** (botones presionados) para disparar acciones en la interfaz
(reproducir / pausar un sonido).

## Contenido

```
TC4/
├── firmware_esp32_tc4/     # Firmware Arduino para la ESP32-S3-DevKitC-1
│   └── firmware_esp32_tc4.ino
├── qt_control_tc4/         # Proyecto Qt (cliente WebSocket + reproductor)
│   ├── qt_control_tc4.pro
│   ├── main.cpp
│   ├── mainwindow.h
│   ├── mainwindow.cpp
│   └── mainwindow.ui
├── sonido/
│   └── tc4_sonido.wav      # Sonido para reproducir/pausar desde los botones
└── README.md
```

## Requisitos

- **Arduino IDE 1.8.19** con soporte **ESP32-S3** (placa: *ESP32S3 Dev Module*).
  Librerías: **WebSockets** (Markus Sattler / Links2004), **ArduinoJson** (v7),
  **Adafruit NeoPixel**.
- **Qt 6** con los módulos **widgets**, **websockets** y **multimedia** (Qt Creator).
- La computadora y la ESP32 deben estar en la **misma red WiFi** (Access Point del salón).

## Cómo se usa

1. En `firmware_esp32_tc4.ino` ajusta `nombreRed` y `claveRed` (tu WiFi) y carga el firmware.
   En el Monitor Serie (115200) aparece la **IP** de la ESP32.
2. Abre `qt_control_tc4/qt_control_tc4.pro` en Qt Creator, compila y ejecuta.
3. En la app escribe la **IP** de la ESP32, puerto **81**, y presiona **Conectar**.
4. Marca las casillas de salida para encender los LEDs; presiona los botones de la placa
   para reproducir/pausar el sonido en la interfaz.

## Mapa de pines (ESP32-S3-DevKitC-1)

| Función | GPIO |
|---|---|
| Salidas 1–4 (IN1–IN4 del L298) | 4, 5, 6, 7 (LEDs) |
| LED RGB integrado | 48 |
| Botón BOOT | 0 |
| Botón "reproducir" (externo) | 15 |
| Botón "pausar" (externo) | 16 |

Entradas con `INPUT_PULLUP` (reposo = HIGH, presionado = LOW).

## Protocolo JSON

**Computadora → ESP32 (comandos):**

```json
{"comando":"salida", "canal":1, "estado":true}
{"comando":"rgb",    "estado":true}
{"comando":"consulta"}
```

**ESP32 → Computadora (eventos y estado):**

```json
{"evento":"boton",  "boton":"reproducir"}
{"evento":"boton",  "boton":"pausar"}
{"evento":"boton",  "boton":"boot"}
{"evento":"estado", "salidas":[false,false,false,false], "rgb":false}
```

Sigue el patrón **JSON-RPC** (petición / respuesta / notificación) sobre el transporte WebSocket.

---

Dr. Rubén Estrada Marmolejo — CUCEI, Universidad de Guadalajara.
