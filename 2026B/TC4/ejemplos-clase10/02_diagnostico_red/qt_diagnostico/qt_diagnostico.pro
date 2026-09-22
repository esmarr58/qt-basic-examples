# ============================================================================
#  Ejemplo 2 (Clase 10) - "Diagnostico de red" (WebSocket) - Interfaces (I7262)
#  App de Qt que empareja con 02_diagnostico_red por WebSocket (ws://<IP>:81):
#  manda "info" y muestra los datos de red que reporta la ESP32.
# ============================================================================
QT += core gui widgets websockets
CONFIG += c++17

SOURCES += main.cpp mainwindow.cpp
HEADERS += mainwindow.h
