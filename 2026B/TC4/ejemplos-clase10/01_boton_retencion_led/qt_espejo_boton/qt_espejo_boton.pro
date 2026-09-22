# ============================================================================
#  Ejemplo 1 (Clase 10) - "Espejo del boton" - Diseno de Interfaces (I7262)
#  App de Qt que empareja con el sketch 01_boton_retencion_led:
#  cada 500 ms hace HTTP GET /estado y refleja el LED de la ESP32.
# ============================================================================
QT += core gui widgets network
CONFIG += c++17

SOURCES += main.cpp mainwindow.cpp
HEADERS += mainwindow.h
