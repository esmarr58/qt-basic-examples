# Clase 17 — Mandar datos por lotes con marca de tiempo epoch

**Diseño de Interfaces (I7262)** — Sesión 17 (Semana 9). Continúa la Clase 16 y se aplica a la
**Práctica #2** (ultrasónico HC-SR04).

En lugar de una línea por muestra, la ESP32 junta **10 muestras** y las manda en **una sola línea**
con la hora de la primera (`t0`) y la separación entre muestras (`dt`). Qt reconstruye el tiempo
de cada punto y lo grafica en un **eje de fecha y hora** (`QDateTimeAxis`).

## Contenido

```
Clase17-Lotes-Epoch/
├── firmware_lotes/            # Firmware Arduino (ESP32-S3): lotes + sincronización de hora
│   └── firmware_lotes.ino
├── qt_lotes_epoch/            # App Qt: QDateTimeAxis, lotes perdidos, botón "Sincronizar hora"
├── firmware_ej4_lotes_websocket/   # Ejemplo 4 (ESP32): los mismos lotes por WebSocket (puerto 81)
├── qt_ej4_lotes_websocket/         # Ejemplo 4 (Qt): cliente WebSocket (estilo TC4)
├── diagrama-bloques.pdf             # lotes por puerto serie
├── diagrama-bloques-websocket.pdf   # Ejemplo 4: lotes por WebSocket
└── README.md
```

`firmware_lotes` + `qt_lotes_epoch` usan el **puerto serie**. El **ejemplo 4** hace lo mismo por
**WebSocket**, igual que el TC4 y la Práctica 1: es la versión con la que seguimos trabajando.

## El contrato del lote

```json
{"seq":7,"t0":1792069200123,"dt":60,"sinc":true,"dist":[37.4,37.9,null,38.6, ...]}
```

| Campo | Significado |
|---|---|
| `seq` | número de lote (0, 1, 2…). Si Qt ve un salto, se perdieron lotes. |
| `t0` | **epoch en milisegundos** de la primera muestra del lote |
| `dt` | milisegundos entre muestras: la muestra `i` ocurrió en `t0 + i*dt` |
| `sinc` | `false` mientras Qt no le haya dicho la hora real a la ESP32 |
| `dist` | distancias en cm; `null` = lectura fallida (JSON **no** acepta `NaN`) |

## ¿Qué es el epoch?

Es el número de **milisegundos (o segundos) transcurridos desde el 1 de enero de 1970 a las 00:00:00 UTC**.
- En **segundos** tiene 10 dígitos (`1792069200`); en **milisegundos**, 13 (`1792069200123`). Qt usa milisegundos.
- Es **UTC**: no depende de la zona horaria. La zona horaria solo se aplica **al mostrar** la hora.
- La ESP32 **no sabe la hora real**: `millis()` cuenta desde que arrancó. Sin sincronizar, `t0`
  es un número pequeño y la gráfica cae en **1970** (o en el 31/12/1969 en hora de México).

## Sincronizar la hora

Al presionar **Sincronizar hora**, Qt manda:

```json
{"cmd":"sync","epoch_ms":1792069200000}
```

La ESP32 guarda `offset = epoch_ms - ms_desde_arranque` y desde entonces `t0 = offset + ms_desde_arranque`.
Usa `esp_timer_get_time()` (64 bits) en vez de `millis()`, que es de 32 bits y regresa a 0 a los ~49.7 días.

## Cómo se usa

1. Sin hardware deja `#define USAR_SENSOR_REAL 0` (distancia simulada). Con el HC-SR04 pon `1`
   (TRIG = GPIO 12, ECHO = GPIO 13, igual que la Práctica #2). Carga el firmware.
2. Cierra el Monitor Serie. Abre `qt_lotes_epoch/qt_lotes_epoch.pro`, compila y ejecuta.
3. **Conectar** → observa la fecha de `t0` (¿qué año aparece?).
4. **Sincronizar hora** → la gráfica salta a la hora actual.

## Ejemplo 4: los mismos lotes por WebSocket

La ESP32 es el **servidor WebSocket (puerto 81)** y Qt el **cliente**, como en el TC4.
Mismo contrato, ahora con el campo `"tipo"`:

| Sentido | Mensaje |
|---|---|
| ESP32 → Qt (al conectar) | `{"tipo":"saludo","mensaje":"...","puerto":81}` |
| ESP32 → Qt (cada 0.6 s) | `{"tipo":"lote","seq":7,"t0":1792069200123,"dt":60,"sinc":true,"dist":[...]}` |
| Qt → ESP32 (botón) | `{"tipo":"sincronizar","epoch_ms":1792069200000}` |
| ESP32 → Qt | `{"tipo":"sincronizado","epoch_ms":1792069200000}` |

1. En `firmware_ej4_lotes_websocket.ino` revisa `nombreRed` y `claveRed`. Sin hardware deja
   `usarSensorReal = false`; con el HC-SR04 pon `true` (TRIG = GPIO 12, ECHO = GPIO 13).
2. Abre `qt_ej4_lotes_websocket/qt_ej4_lotes_websocket.pro`, compila y ejecuta.
3. Escribe la IP, puerto **81**, **Conectar** → ¿qué fecha tiene `t0`? → **Sincronizar hora**.

Librerías: **WebSockets** (Links2004) y **ArduinoJson** v7. ArduinoJson guarda enteros de 64 bits
en la ESP32, así que `t0` (13 dígitos) viaja completo.
