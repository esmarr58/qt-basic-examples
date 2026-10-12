# Clase 16 — Graficar en Qt con contratos JSON básicos

**Diseño de Interfaces (I7262)** — Sesión 16 (Semana 9).
La ESP32-S3 manda **una línea JSON por muestra** por el puerto serie y Qt la grafica con **Qt Charts**.
No se necesita ningún sensor: el firmware **simula** temperatura, humedad y luz.

## Contenido

```
Clase16-Graficar-JSON/
├── firmware_contratos/        # Firmware Arduino (ESP32-S3): contratos v1, v2 y v3
│   └── firmware_contratos.ino
├── qt_ej1_un_valor/           # Ejemplo 1: contrato v1 -> una curva
├── qt_ej2_dos_valores/        # Ejemplo 2: contrato v2 -> dos curvas, doble eje Y
├── firmware_ej3_websocket/    # Ejemplo 3 (ESP32): contrato v2 por WebSocket (servidor, puerto 81)
├── qt_ej3_websocket/          # Ejemplo 3 (Qt): cliente WebSocket + doble eje Y (estilo TC4)
├── diagrama-bloques.pdf             # Ejemplos 1 y 2: el camino del dato por el puerto serie
├── diagrama-bloques-websocket.pdf   # Ejemplo 3: el mismo camino por WebSocket
└── README.md
```

Los ejemplos 1 y 2 usan el **puerto serie (USB)**. El ejemplo 3 usa **WebSocket**, igual que el
TC4 y la Práctica 1: es la forma con la que trabajamos el resto del curso.

## Los contratos

Un **contrato** es el acuerdo de qué manda la ESP32 y qué espera Qt. Una línea por muestra, terminada en `\n`:

| Contrato | Línea que viaja por el cable | Lo usa |
|---|---|---|
| v1 | `{"t":24.80}` | `qt_ej1_un_valor` |
| v2 | `{"t":24.80,"h":55.30}` | `qt_ej2_dos_valores` |
| v3 | `{"t":24.80,"h":55.30,"lux":412}` | **ejercicio**: modifica el ejemplo 2 para graficar `lux` |

En el ejemplo 2, las líneas marcadas con `// [v2]` son las únicas que cambian respecto al ejemplo 1.

## Requisitos

- **Arduino IDE** con soporte **ESP32-S3** (placa: *ESP32S3 Dev Module*). No usa librerías externas.
- **Qt 6** con los módulos **serialport** y **charts** (Qt Creator).

## Cómo se usa

1. En `firmware_contratos.ino` elige `#define CONTRATO 1` (o 2, o 3) y carga el firmware.
   Puedes comprobarlo en el Monitor Serie (115200): aparece una línea JSON cada 0.5 s.
2. **Cierra el Monitor Serie** (si no, Qt no puede abrir el puerto).
3. Abre `qt_ej1_un_valor/qt_ej1_un_valor.pro` en Qt Creator, compila y ejecuta.
4. Elige el puerto (`COMx` o `/dev/ttyUSB0`) y presiona **Conectar**.
5. Para el ejemplo 2 cambia el firmware a `CONTRATO 2` y abre `qt_ej2_dos_valores`.

## El camino del dato

```
ESP32: medir -> serializar JSON -> enviar línea (\n)
   -> USB/serie -> buffer de QSerialPort
Qt:    readyRead -> canReadLine/readLine -> QJsonDocument::fromJson -> QLineSeries::append -> QChart
```

`readyRead` **no** equivale a un mensaje: el serie llega en pedazos. Por eso se lee solo con
`canReadLine()` y `readLine()`, nunca con `readAll()`.

## Ejemplo 3: el mismo contrato por WebSocket

Igual que en el TC4, la ESP32 es el **servidor WebSocket (puerto 81)** y Qt es el **cliente**.
Cada mensaje lleva el campo `"tipo"`:

| Sentido | Mensaje |
|---|---|
| ESP32 → Qt (al conectar) | `{"tipo":"saludo","mensaje":"...","puerto":81}` |
| ESP32 → Qt (cada 0.5 s) | `{"tipo":"medicion","t":24.8,"h":55.3}` |

**Qué cambia respecto al puerto serie:** cada mensaje WebSocket llega **completo** a
`textMessageReceived`, así que ya no hace falta `canReadLine()` ni el `\n`. Además ya no
importa el cable: basta con estar en la misma red.

**Requisitos extra:** librerías **WebSockets** (Links2004) y **ArduinoJson** v7 (las del TC4);
en Qt, el módulo **websockets**.

1. En `firmware_ej3_websocket.ino` revisa `nombreRed` y `claveRed` (Access Point del salón) y carga el firmware.
   En el Monitor Serie (115200) aparece la **IP** de la ESP32.
2. Abre `qt_ej3_websocket/qt_ej3_websocket.pro`, compila y ejecuta.
3. Escribe la IP, puerto **81**, y presiona **Conectar**.
