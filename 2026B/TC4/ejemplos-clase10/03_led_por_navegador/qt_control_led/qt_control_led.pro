# ============================================================================
#  Ejemplo 3 (Clase 10) - "Control del LED" - Diseno de Interfaces (I7262)
#  App de Qt que empareja con el sketch 03_led_por_navegador:
#  botones ENCENDER/APAGAR -> HTTP GET /encender y /apagar en la ESP32.
#  Es el mismo ida y vuelta del TC4, pero con HTTP en vez de WebSockets.
# ============================================================================
QT += core gui widgets network
CONFIG += c++17

SOURCES += main.cpp mainwindow.cpp
HEADERS += mainwindow.h
