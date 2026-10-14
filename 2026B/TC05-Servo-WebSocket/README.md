# TC05 — Control de un servomotor SG90 desde Qt por WebSocket

Trabajo en Clase 5 (TC05) de **Diseño de Interfaces (I7262)** — actividad autoguiada.

Continúa el TC4: la ESP32-S3 sigue siendo el **servidor WebSocket** (puerto 81) y la interfaz
de Qt el **cliente**, con mensajes **JSON** que llevan el campo `"tipo"`. Ahora, en lugar de
encender LEDs, la interfaz manda un **ángulo (0 a 180°)** y la ESP32 mueve un **servomotor SG90**.
La ESP32 **confirma** el ángulo que realmente aplicó (`servo_ack`) y la interfaz muestra ese valor
confirmado, distinto del ángulo pedido.

![Diagrama a bloques](diagrama-bloques.pdf)

## Contenido

```
TC05-Servo-WebSocket/
├── firmware_tc05_servo/      # Firmware Arduino para la ESP32-S3-DevKitC-1
│   └── firmware_tc05_servo.ino
├── qt_tc05_servo/            # Proyecto Qt (cliente WebSocket)
│   ├── qt_tc05_servo.pro
│   ├── main.cpp
│   ├── mainwindow.h
│   ├── mainwindow.cpp
│   └── mainwindow.ui
├── diagrama-bloques.tex      # Diagrama a bloques (TikZ)
├── diagrama-bloques.pdf
└── README.md
```

## Requisitos

- **Arduino IDE** con el paquete **esp32 3.3.x** (placa: *ESP32S3 Dev Module*).
- Librerías (Gestor de librerías):

  | Librería | Autor | Versión probada |
  |---|---|---|
  | **WebSockets** | Markus Sattler (Links2004) | 2.7.2 |
  | **ArduinoJson** | Benoit Blanchon | 7.4.3 (cualquier 7.x) |
  | **ESP32Servo** | Kevin Harrington / madhephaestus | 3.2.1 |

  Con `arduino-cli`: `arduino-cli lib install "ESP32Servo"`.
  Al compilar, ESP32Servo muestra *warnings* internos de la librería (variables sin usar,
  driver MCPWM antiguo); son de la librería, no de nuestro código, y no afectan.
- **Qt 6** con los módulos **widgets** y **websockets** (Qt Creator). Probado con Qt 6.10.2 + MinGW.
- La computadora y la ESP32 deben estar en la **misma red WiFi** (Access Point del salón).

## Conexiones

En la **ESP32-S3-DevKitC-1** los últimos tres pines del conector **J1** (lado del USB) son
**GPIO14, 5V y GND**, en ese orden. El conector del SG90 entra **directo**, sin protoboard ni cables extra:

| Cable del SG90 | Pin de la placa (J1) |
|---|---|
| **Naranja** (señal) | **GPIO 14** |
| **Rojo** (+5 V) | **5V** |
| **Café** (GND) | **GND** |

**¿Por qué el GPIO 14?** Además de quedar junto a 5V y GND, no es pin de arranque (*strapping*: 0, 3, 45, 46), no lo ocupa la
PSRAM octal de los módulos N8R8 / N16R8 (GPIO 35, 36, 37), no es el USB nativo (19, 20) y no
choca con los pines del TC4 (4–7, 0, 15, 16, 48), así que el protoboard del TC4 puede quedarse armado.

> **Advertencia — picos de corriente.** Al arrancar o al moverse con carga, el SG90 puede pedir
> picos de 500 mA a 700 mA. Alimentado desde el pin 5V de la placa (que viene del USB de la
> computadora) puede provocar **reinicios** de la ESP32 o desconexiones del WiFi. Para un servo
> sin carga suele bastar; si ves reinicios, usa una **fuente externa de 5 V** y conecta su GND
> con el GND de la ESP32 (**tierra común**: sin ella la señal no tiene referencia y el servo no responde).
> Un capacitor de 100 µF–470 µF entre 5 V y GND cerca del servo ayuda. **Nunca** alimentes el servo con 3.3 V.

## Cómo se usa

1. En `firmware_tc05_servo.ino` revisa `nombreRed` y `claveRed` (Access Point del salón).
2. Arduino IDE: placa **ESP32S3 Dev Module**, elige el puerto y carga el firmware.
   En el Monitor Serie (115200) aparece la **IP** de la ESP32. El servo arranca en **90°**.
3. Abre `qt_tc05_servo/qt_tc05_servo.pro` en Qt Creator, elige el *kit* (Desktop Qt 6 MinGW),
   compila y ejecuta.
4. Escribe la **IP** de la ESP32, puerto **81**, y presiona **Conectar**. Los controles del servo
   se activan solo cuando hay conexión.
5. Mueve el **deslizador**, escribe un valor en la **caja numérica** o usa los botones **0° / 90° / 180°**.
   La etiqueta grande muestra el ángulo **confirmado por la ESP32**. **Barrido (demo)** recorre
   0 → 180 → 0 sin bloquear la ESP32.

### Límite de envío (throttle)

Al arrastrar el deslizador, `valueChanged` se emite por **cada grado**. En lugar de mandar cientos
de mensajes por segundo, la interfaz envía **como máximo uno cada 50 ms** (`QTimer` de un solo
disparo en `solicitarAngulo()` / `alVencerTemporizador()`), y al **soltar** el deslizador envía de
inmediato el valor final (`alSoltarDeslizador()`). El servo solo se actualiza cada 20 ms, así que
no se pierde nada visible.

## Protocolo JSON

Todos los mensajes llevan un campo **`"tipo"`** (mismo esquema que el TC4).

| Sentido | Mensaje | Cuándo |
|---|---|---|
| Qt → ESP32 | `{"tipo":"servo","angulo":90}` | Mover el servo (0..180; fuera de rango se recorta) |
| Qt → ESP32 | `{"tipo":"barrido"}` | Demo 0 → 180 → 0 (un comando `servo` lo cancela) |
| Qt → ESP32 | `{"tipo":"consulta"}` | Pedir el ángulo actual |
| ESP32 → Qt | `{"tipo":"saludo","mensaje":"Servidor TC05 listo","puerto":81,"angulo":90}` | Al conectar |
| ESP32 → Qt | `{"tipo":"servo_ack","angulo":90}` | Tras cada movimiento, a **todos** los clientes |
| ESP32 → Qt | `{"tipo":"error","mensaje":"..."}` | JSON inválido, falta `tipo`/`angulo` o tipo desconocido (solo a ese cliente) |

Ejemplo: si la interfaz manda `{"tipo":"servo","angulo":250}`, la ESP32 aplica 180 y responde
`{"tipo":"servo_ack","angulo":180}`: el ángulo **pedido** (250) y el **confirmado** (180) no coinciden.

## Funciones y widgets principales

- **Firmware:** `alEventoWebSocket()` → `procesarComando()` → `aplicarAngulo()` (`servo.write`)
  → `avisarAngulo()` (`broadcastTXT`); `enviarError()`, `actualizarBarrido()`.
- **Qt:** `alPresionarConectar()`, `solicitarAngulo()`, `enviarAngulo()`, `enviarJson()`,
  `alRecibirMensaje()`, `interpretarEvento()`, `registrar()`.
- **Widgets (.ui):** `lineEditIp`, `lineEditPuerto`, `botonConectar`, `etiquetaEstado`,
  `grupoServo`, `deslizadorAngulo`, `cajaAngulo`, `boton0`, `boton90`, `boton180`,
  `botonBarrido`, `etiquetaPedido`, `etiquetaConfirmado`, `registro`.

## Solución de problemas

| Síntoma | Causa probable / solución |
|---|---|
| El servo no se mueve, pero llega `servo_ack` | El conector quedó desfasado un pin (el naranja debe quedar en **GPIO 14**, el rojo en 5V y el café en GND). |
| La ESP32 se reinicia al mover el servo | Picos de corriente: usa fuente externa de 5 V y un capacitor. |
| El servo zumba o vibra en 0° / 180° | Ajusta `pulsoMinimo` / `pulsoMaximo` (p. ej. 600 y 2300 µs). |
| "Error de conexion" en Qt | IP equivocada, la ESP32 no está en la red del salón o la PC está en otra red. |
| El Monitor Serie se queda en "Conectando a WiFi....." | SSID o contraseña incorrectos, o el AP está fuera de alcance. |
| El servo se mueve "a saltos" al arrastrar | Normal: el throttle manda un valor cada 50 ms. |
| `{"tipo":"error",...}` en el registro | El mensaje enviado no tiene el formato del protocolo. |

---

Dr. Rubén Estrada Marmolejo — CUCEI, Universidad de Guadalajara.
