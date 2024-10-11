QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui

#Paso # 1. Indicar a qt donde esta la instalacion de opencv
INCLUDEPATH += /usr/include/opencv4 #sudo apt install libopencv-dev

#Paso # 2. Que bibliotecas se usaran de opencv4
LIBS += -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_video -lopencv_videoio

#Paso # 3. Configurar pkgcofig
CONFIG += link_pkgconfig
PKGCONFIG += opencv4


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
