# Ejemplos de la Clase 10 (antes del TC4)

Tres programas para la **ESP32-S3** que veremos en la **Sesión 10** de Diseño de
Interfaces (I7262), **antes de empezar el TC4**. Sirven para que cada alumno pruebe su
**hardware** (botón + LED) y la **conexión a la red del salón** paso a paso.

- **Red (Access Point del salón):** SSID `GWN571D04`, contraseña `ESP32CUCEI$$`
  (la misma que usa el firmware del TC4). Cámbiala en el `#define`/`const` de arriba de
  cada sketch si tu red es otra.
- **Placa (Arduino IDE):** `ESP32S3 Dev Module`.
- **Librerías:** `WiFi` y `WebServer` vienen con el core de ESP32; instala
  **Adafruit NeoPixel** desde el gestor de librerías.

## Cableado (el mismo del TC4, para no recablear)
| Elemento | Pin ESP32-S3 | Nota |
|---|---|---|
| Pulsador | GPIO15 → GND | usa pull-up interna (reposo = HIGH) |
| LED externo | GPIO4 (+ R 330 Ω) | una de las salidas IN1..IN4 del TC4 |
| LED RGB de la placa | GPIO48 (WS2812) | verde = encendido, azul = conectando |

## Los tres ejemplos (en orden)

1. **`01_boton_retencion_led`** — Botón con **retención** (cada pulsación enciende/apaga
   el LED, como en la Práctica 1) **y** conexión a la red imprimiendo la IP.
   → Prueba **hardware + red** de una sola vez.

2. **`02_diagnostico_red`** — Solo red: se conecta e imprime IP, máscara, gateway, DNS,
   MAC y RSSI; el LED RGB va rojo (conectando) → verde (conectada) y reintenta si se cae.
   → Confirma que la PC y la ESP32 quedan en la **misma red**.

3. **`03_led_por_navegador`** — La placa levanta un **servidor web**; abre `http://<IP>`
   en el navegador de tu PC y prende/apaga el LED con botones. Es el mismo ida y vuelta
   PC ↔ ESP32 que en el TC4 haremos con **Qt + WebSockets**, pero aquí con el navegador.
   → Prueba el **camino de red completo** antes del TC4.

## Cómo usarlos
1. Abre el sketch en Arduino IDE, selecciona la placa `ESP32S3 Dev Module` y el puerto.
2. Compila y sube (**Upload**).
3. Abre el **Monitor Serial** a **115200** para ver la IP y los mensajes.
4. En el ejemplo 3, escribe esa IP en el navegador de tu PC (misma red).
