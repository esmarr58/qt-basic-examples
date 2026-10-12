# ============================================================================
#  Clase 16 - Ejemplo 3 - Diseño de Interfaces (I7262)
#  Grafica el contrato v2 {"tipo":"medicion","t":..,"h":..} recibido por WebSocket.
# ============================================================================
#
#  Modulos de Qt que necesita el proyecto:
#    core, gui, widgets -> aplicacion de escritorio con ventanas y widgets
#    websockets         -> cliente WebSocket (QWebSocket)
#    charts             -> graficas (QChart, QLineSeries, QValueAxis)
QT += core gui widgets websockets charts

CONFIG += c++17

# Archivos de codigo fuente
SOURCES += \
    main.cpp \
    mainwindow.cpp

# Archivos de encabezado
HEADERS += \
    mainwindow.h

# Formulario de la interfaz (se edita en el Diseñador de Qt)
FORMS += \
    mainwindow.ui

# Reglas de despliegue por defecto
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
