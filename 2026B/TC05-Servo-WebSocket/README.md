# TC05 — Control de un servomotor SG90 desde Qt por WebSocket

Trabajo en Clase 5 (TC05) de **Diseño de Interfaces (I7262)** — actividad autoguiada.

Continúa el TC4: la ESP32-S3 sigue siendo el **servidor WebSocket** (puerto 81) y la interfaz
de Qt el **cliente**, con mensajes **JSON** que llevan el campo `"tipo"`. Ahora, en lugar de
encender LEDs, la interfaz manda un **ángulo (0 a 180°)** y la ESP32 mueve un **servomotor SG90**.
La ESP32 **confirma** el ángulo que realmente aplicó (`servo_ack`) y la interfaz muestra ese valor
confirmado, distinto del ángulo pedido. Además, la ESP32 lee un **potenciómetro** (GPIO 8) y la interfaz
lo grafica en tiempo real con **Qt Charts**.

![Diagrama a bloques](diagrama-bloques.pdf)

## Contenido

> **La app de Qt no está en este repositorio:** la construyes tú, paso a paso, siguiendo la guía del TC05
> (Word/PDF en Classroom). Aquí solo están el firmware de la ESP32-S3, el simulador y el diagrama.

```
TC05-Servo-WebSocket/
├── firmware_tc05_servo/      # Firmware Arduino para la ESP32-S3-DevKitC-1
│   └── firmware_tc05_servo.ino
├── simulador_esp32.py        # Simulador del firmware para probar la app sin hardware
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
- **Qt 6** con los módulos **widgets**, **websockets** y **charts** (Qt Creator; en el instalador, *Additional Libraries → Qt Charts*). Probado con Qt 6.10.2 + MinGW.
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

### Potenciómetro (para la gráfica)

| Pin del potenciómetro (10 kΩ) | Pin de la placa |
|---|---|
| Un extremo | **3V3** |
| Otro extremo | **GND** |
| Cursor (pin central) | **GPIO 8** (ADC1) |

Nunca lo conectes a 5V: el ADC de la ESP32-S3 solo acepta hasta 3.3 V.

## Cómo se usa

1. En `firmware_tc05_servo.ino` revisa `nombreRed` y `claveRed` (Access Point del salón).
2. Arduino IDE: placa **ESP32S3 Dev Module**, elige el puerto y carga el firmware.
   En el Monitor Serie (115200) aparece la **IP** de la ESP32. El servo arranca en **90°**.
3. Crea tu propio proyecto en Qt Creator siguiendo la guía del TC05 (Pasos 2 a 7), compílalo y ejecútalo.
4. Escribe la **IP** de la ESP32, puerto **81**, y presiona **Conectar**. Los controles del servo
   se activan solo cuando hay conexión.
5. Mueve el **deslizador**: la etiqueta de la izquierda muestra el ángulo **pedido** y la de la derecha
   el ángulo **confirmado por la ESP32**. **Consultar** pide el ángulo actual y **Barrido** recorre
   0 → 180 → 0 sin bloquear la ESP32.
6. Gira el potenciómetro: la **gráfica** muestra el voltaje (0 a 3.3 V) de los últimos 10 s. Las
   lecturas `pot` van solo a la gráfica, no al registro.

### Límite de envío (throttle)

Al arrastrar el deslizador, `valueChanged` se emite por **cada grado**. En lugar de mandar cientos
de mensajes por segundo, el slot solo guarda el ángulo en `anguloPendiente`, y un `QTimer` de
**50 ms** envía el último pendiente (como máximo 20 mensajes por segundo). El servo solo se
actualiza cada 20 ms, así que no se pierde nada visible.

### Probar sin hardware

`simulador_esp32.py` responde igual que el firmware y simula el potenciómetro
(`pip install websockets`; `python simulador_esp32.py 81`, o el 8181 por defecto). Conéctate a `127.0.0.1`.

## Protocolo JSON

Todos los mensajes llevan un campo **`"tipo"`** (mismo esquema que el TC4).

| Sentido | Mensaje | Cuándo |
|---|---|---|
| Qt → ESP32 | `{"tipo":"servo","angulo":90}` | Mover el servo (0..180; fuera de rango se recorta) |
| Qt → ESP32 | `{"tipo":"barrido"}` | Demo 0 → 180 → 0 (un comando `servo` lo cancela) |
| Qt → ESP32 | `{"tipo":"consulta"}` | Pedir el ángulo actual |
| ESP32 → Qt | `{"tipo":"saludo","mensaje":"Servidor TC05 listo","puerto":81,"angulo":90}` | Al conectar |
| ESP32 → Qt | `{"tipo":"servo_ack","angulo":90}` | Tras cada movimiento, a **todos** los clientes |
| ESP32 → Qt | `{"tipo":"pot","valor":2048,"voltaje":1.65}` | Cada 100 ms, a **todos**: lectura del potenciómetro (0..4095 → 0..3.3 V) |
| ESP32 → Qt | `{"tipo":"error","mensaje":"..."}` | JSON inválido, falta `tipo`/`angulo` o tipo desconocido (solo a ese cliente) |

Ejemplo: si la interfaz manda `{"tipo":"servo","angulo":250}`, la ESP32 aplica 180 y responde
`{"tipo":"servo_ack","angulo":180}`: el ángulo **pedido** (250) y el **confirmado** (180) no coinciden.

## Funciones y widgets principales

- **Firmware:** `alEventoWebSocket()` → `procesarComando()` → `aplicarAngulo()` (`servo.write`)
  → `avisarAngulo()` (`broadcastTXT`); `enviarError()`, `actualizarBarrido()`.
- **Qt:** slots creados con *Go to slot* (`on_botonConectar_clicked()`, `on_deslizadorAngulo_valueChanged()`,
  `on_botonConsultar_clicked()`, `on_botonBarrido_clicked()`); señales de `socket` y `temporizador`
  conectadas con `connect` + lambda en el constructor; ayudas `enviarJson()`, `registrar()` y `agregarLectura()`.
- **Widgets (.ui):** `lineEditIp`, `spinPuerto`, `botonConectar`, `etiquetaEstado`, `grupoServo`,
  `deslizadorAngulo`, `etiquetaPedido`, `etiquetaConfirmado`, `botonConsultar`, `botonBarrido`,
  `graficaPot` (Widget promovido a `QChartView`), `registro`.

## Solución de problemas

| Síntoma | Causa probable / solución |
|---|---|
| El servo no se mueve, pero llega `servo_ack` | El conector quedó desfasado un pin (el naranja debe quedar en **GPIO 14**, el rojo en 5V y el café en GND). |
| La ESP32 se reinicia al mover el servo | Picos de corriente: usa fuente externa de 5 V y un capacitor. |
| El servo zumba o vibra en 0° / 180° | Ajusta `pulsoMinimo` / `pulsoMaximo` (p. ej. 600 y 2300 µs). |
| "Error de conexion" en Qt | IP equivocada, la ESP32 no está en la red del salón o la PC está en otra red. |
| El Monitor Serie se queda en "Conectando a WiFi....." | SSID o contraseña incorrectos, o el AP está fuera de alcance. |
| El servo se mueve "a saltos" al arrastrar | Normal: el throttle manda un valor cada 50 ms. |
| `Unknown module(s) in QT: charts` | Falta instalar Qt Charts (Qt Maintenance Tool → Add or remove components). |
| La gráfica se queda plana | El cursor del potenciómetro no está en el GPIO 8, o falta 3V3/GND en los extremos. |
| `{"tipo":"error",...}` en el registro | El mensaje enviado no tiene el formato del protocolo. |

---

Dr. Rubén Estrada Marmolejo — CUCEI, Universidad de Guadalajara.
