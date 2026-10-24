# Práctica 3 · El robot de la Práctica 2 desde Android (Qt 6 + WebSocket)

Diseño de Interfaces (I7262) · 2026B · Dr. Rubén Estrada Marmolejo

App de Qt 6 (qmake) que controla el robot de la Práctica 2 desde un teléfono Android:
mover el robot (mantén presionado un botón), orientar el servo del HC-SR04, medir la
distancia y graficarla. El mismo proyecto compila para escritorio y para Android: solo
cambia el kit en Qt Creator.

- Video de instalación y configuración de Android para Qt: https://youtu.be/Jr2Uc-txARs
- Scripts del video: https://github.com/esmarr58/tutoriales/tree/main/qt-creator/android-sdk-hola-mundo
- Diagrama a bloques: `diagrama-bloques-p3.pdf`

## Antes de compilar

1. En `qt_practica3_android/mainwindow.cpp` cambia `ALUMNO` por tu nombre y código (regla antiplagio).
2. Instala con el Qt Maintenance Tool el kit **Android** de tu versión de Qt (incluye Qt WebSockets y Qt Charts).
3. En Qt Creator: *Preferencias › Dispositivos › Android*: JDK, SDK y NDK en verde.
4. En el teléfono: Opciones de desarrollador y Depuración por USB.

## Protocolo (firmware de la P2, puerto 81)

| Qt → ESP32-S3 | ESP32-S3 → Qt |
|---|---|
| `{"tipo":"latido"}` cada 1 s | `latido_ok` |
| `{"tipo":"mover","accion":"avanzar\|retroceder\|izquierda\|derecha\|parar","ms":400}` | `mover_ack` / `mover_err`, `mover_inicio`, `mover_fin` |
| `{"tipo":"servo","angulo":0..180}` | — |
| `{"tipo":"leer_distancia"}` | `{"tipo":"distancia_lectura","d":cm}` |
| `{"tipo":"scan","angulo":n}` | `{"tipo":"scan","d":cm,"angulo":n}` |

Mientras el dedo está en un botón, la app reenvía `mover` cada 200 ms con `ms=400`; al
soltar manda `parar`. El firmware reemplaza el movimiento en curso con cada orden nueva,
así que las órdenes no se acumulan.

## Archivos

- `qt_practica3_android.pro`: `QT += core gui widgets websockets charts`
- `main.cpp`: pantalla completa en Android; ventana de 420×820 en escritorio
- `mainwindow.h/.cpp`: interfaz armada en código, temporizadores y protocolo
