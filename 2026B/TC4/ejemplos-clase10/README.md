# Ejemplos de la Clase 10 (antes del TC4)

Tres ejemplos para la **Sesión 10** de Diseño de Interfaces (I7262), **antes de empezar el
TC4**. Cada uno trae **su sketch de ESP32-S3 (Arduino)**, **su app de Qt** que se empareja con
él por la red, y **su diagrama a bloques en PDF**. Sirven para que cada alumno pruebe su
**hardware** (botón + LED) y la **conexión a la red del salón** paso a paso, y para llegar al
TC4 con la placa y la red ya funcionando.

- **Red (Access Point del salón):** SSID `GWN571D04`, contraseña `ESP32CUCEI$$`
  (la misma del firmware del TC4). Cámbiala arriba de cada sketch si tu red es otra.
- **Placa (Arduino IDE):** `ESP32S3 Dev Module`. **Librerías:** `WiFi` y `WebServer` vienen con
  el core de ESP32; instala **Adafruit NeoPixel**.
- **Qt:** proyectos con `qmake` (módulos `widgets` + `network`). Ábrelos en Qt Creator, o compila
  con `qmake6 && make`. Escribe en la app la **IP** que la ESP32 muestra en el Monitor Serial
  (tu PC debe estar en la **misma red**).

## Cableado (el mismo del TC4, para no recablear)
| Elemento | Pin ESP32-S3 | Nota |
|---|---|---|
| Pulsador | GPIO15 → GND | pull-up interna (reposo = HIGH) |
| LED externo | GPIO4 (+ R 330 Ω) | una de las salidas IN1..IN4 del TC4 |
| LED RGB de la placa | GPIO48 (WS2812) | verde = encendido, azul = conectando |

## Los tres ejemplos (en orden)

### 1. `01_boton_retencion_led`  →  Qt: `qt_espejo_boton`
- **ESP32:** botón con **retención** (cada pulsación enciende/apaga el LED, como en la Práctica 1),
  se conecta a la red e imprime su IP, y **sirve** `GET /estado` → `ON`/`OFF`.
- **Qt (Espejo del botón):** cada 500 ms hace `GET /estado` y **refleja** el LED de la ESP32.
- Prueba **hardware + red + emparejamiento** de una vez.

### 2. `02_diagnostico_red`  →  Qt: `qt_diagnostico`
- **ESP32:** se conecta e imprime IP/máscara/gateway/DNS/MAC/RSSI (RGB rojo→verde) y **sirve**
  `GET /info` con esos datos.
- **Qt (Diagnóstico):** botón *Consultar* → `GET /info` → muestra los datos.
- Confirma que la PC y la ESP32 están en la **misma red**.

### 3. `03_led_por_navegador`  →  Qt: `qt_control_led`
- **ESP32:** servidor web con `/`, `/encender`, `/apagar` (controla el LED).
- **Qt (Control del LED):** botones **ENCENDER/APAGAR** → `GET /encender` / `/apagar`.
  (También puedes abrir `http://<IP>` en el navegador.)
- Es el **mismo ida y vuelta PC ↔ ESP32 del TC4**, pero con HTTP en vez de WebSockets + JSON.

## Cómo usarlos en clase
1. Sube el **sketch** a la ESP32 (Arduino IDE), abre el **Monitor Serial** a 115200 y anota la **IP**.
2. Abre el **proyecto Qt** correspondiente, compílalo y córrelo.
3. Escribe la IP en la app y prueba (conectar / consultar / encender-apagar).
4. Revisa el **diagrama a bloques** (`diagrama-bloques.pdf`) de cada carpeta para ver el sistema completo.
