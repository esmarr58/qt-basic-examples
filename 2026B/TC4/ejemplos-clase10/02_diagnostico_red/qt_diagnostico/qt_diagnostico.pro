# ============================================================================
#  Ejemplo 2 (Clase 10) - "Diagnostico de red" - Diseno de Interfaces (I7262)
#  App de Qt que empareja con el sketch 02_diagnostico_red:
#  hace HTTP GET /info y muestra los datos de red de la ESP32.
# ============================================================================
QT += core gui widgets network
CONFIG += c++17

SOURCES += main.cpp mainwindow.cpp
HEADERS += mainwindow.h
