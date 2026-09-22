# ============================================================================
#  Ejemplo 3 (Clase 10) - "Control del LED" (WebSocket) - Interfaces (I7262)
#  App de Qt que empareja con 03_led_por_navegador por WebSocket (ws://<IP>:81):
#  envia "ENCENDER"/"APAGAR" y muestra el estado que difunde la ESP32.
#  Es el mismo ida y vuelta del TC4; en el TC4 los mensajes se envuelven en JSON.
# ============================================================================
QT += core gui widgets websockets
CONFIG += c++17

SOURCES += main.cpp mainwindow.cpp
HEADERS += mainwindow.h
