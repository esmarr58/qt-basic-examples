#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    VentanaPrincipal ventana;
    ventana.show();
    return app.exec();
}
