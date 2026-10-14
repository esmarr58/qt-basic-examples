# ============================================================================
#  TC05 - Diseño de Interfaces (I7262)
#  Interfaz en Qt que mueve un servomotor SG90 conectado a la ESP32-S3,
#  enviando el angulo en JSON por WebSocket.
# ============================================================================
#
#  Modulos de Qt que necesita el proyecto:
#    core, gui, widgets -> aplicacion de escritorio con ventanas y widgets
#    websockets         -> cliente WebSocket (QWebSocket)
#    charts             -> grafica del potenciometro (QChart, QChartView)
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
