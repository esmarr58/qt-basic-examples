# ===== Proyecto Qt de escritorio =====

QT += core gui widgets multimedia websockets network

CONFIG += c++17 console
CONFIG -= app_bundle

# Nombre del ejecutable
TARGET = test1
TEMPLATE = app

# ===== Archivos fuente =====
SOURCES += \
    detector_sonrisas.cpp \
    detectorcolores.cpp \
    detectordedos.cpp \
    main.cpp \
    mainwindow.cpp \
    trabajador_timer.cpp \
    websocketcliente.cpp

# ===== Archivos de encabezado =====
HEADERS += \
    detector_sonrisas.h \
    detectorcolores.h \
    detectordedos.h \
    mainwindow.h \
    trabajador_timer.h \
    websocketcliente.h

# ===== Archivos de interfaz (Qt Designer) =====
FORMS += \
    mainwindow.ui

# ===== (Opcional) Configuración de compilación =====
# QMAKE_CXXFLAGS += -Wall -Wextra
# DEFINES += QT_DEPRECATED_WARNINGS

# ===== Si usas OpenCV, descomenta y ajusta =====
# --- OpenCV ---
INCLUDEPATH += /usr/include/opencv4
LIBS += -L/usr/lib \
    -lopencv_core \
    -lopencv_imgproc \
    -lopencv_objdetect \
    -lopencv_imgcodecs \
    -lopencv_highgui \
    -lopencv_videoio

