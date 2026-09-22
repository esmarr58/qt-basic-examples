# Ejemplos de la Clase 10 (antes del TC4)

Tres ejemplos para la **Sesión 10** de Diseño de Interfaces (I7262), **antes de empezar el
TC4**. Cada uno trae **su sketch de ESP32-S3 (Arduino)**, **su app de Qt** y **su diagrama a
bloques en PDF**. Todos usan el **mismo protocolo que la Práctica 4 y el TC4**:
**WebSocket (puerto 81) + JSON** con un campo `"tipo"`. Sirven para que cada alumno pruebe su
**hardware** (botón + LED) y la **conexión a la red del salón** antes del TC4.

- **Red (Access Point del salón):** SSID `GWN571D04`, contraseña `ESP32CUCEI$$`.
- **Placa (Arduino IDE):** `ESP32S3 Dev Module`. **Librerías:** `WiFi` (core), **WebSockets**
  (Links2004), **ArduinoJson** (Benoit Blanchon) y **Adafruit NeoPixel**.
- **Qt:** proyectos `qmake` con módulo **websockets**. Ábrelos en Qt Creator o compila con
  `qmake6 && make`. Conéctate a **`ws://<IP>:81`** (la IP sale en el Monitor Serial; tu PC debe
  estar en la misma red).

## Protocolo (JSON con `tipo`, igual que el TC4)
- Al conectar, la ESP32 manda `{"tipo":"saludo", ...}`.
- **Ej. 1:** ESP32 → PC `{"tipo":"estado_led","encendido":true|false}` al pulsar el botón.
- **Ej. 2:** PC → ESP32 `{"tipo":"leer_red"}` · ESP32 → PC `{"tipo":"datos_red","ip":..,"rssi":..}`.
- **Ej. 3:** PC → ESP32 `{"tipo":"led","encendido":true|false}` · ESP32 → PC `{"tipo":"estado_led",...}`.

## Cableado (el mismo del TC4)
| Elemento | Pin ESP32-S3 | Nota |
|---|---|---|
| Pulsador | GPIO15 → GND | pull-up interna (reposo = HIGH) |
| LED externo | GPIO4 (+ R 330 Ω) | una de las salidas IN1..IN4 del TC4 |
| LED RGB de la placa | GPIO48 (WS2812) | verde = encendido, azul = conectando |

## Los tres ejemplos (en orden)
1. **`01_boton_retencion_led`** → Qt **`qt_espejo_boton`**: botón con retención → LED; la app
   refleja el LED en **tiempo real** (recibe `estado_led` por WebSocket).
2. **`02_diagnostico_red`** → Qt **`qt_diagnostico`**: la app pide `leer_red` y muestra los datos
   de red que responde la ESP32. Confirma que PC y ESP32 están en la **misma red**.
3. **`03_led_por_navegador`** → Qt **`qt_control_led`**: botones ENCENDER/APAGAR mandan
   `{"tipo":"led",...}`. Es el **mismo ida y vuelta del TC4** (ahí se agregan más comandos).

## Cómo usarlos en clase
1. Sube el **sketch** (Arduino IDE), abre el **Monitor Serial** a 115200 y anota la **IP**.
2. Abre el **proyecto Qt** correspondiente, compílalo y córrelo.
3. Escribe la IP y conecta (`ws://<IP>:81`).
4. Revisa el **diagrama a bloques** (`diagrama-bloques.pdf`) de cada carpeta.
