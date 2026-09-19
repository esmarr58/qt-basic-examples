// ============================================================================
//  TC4 - Diseño de Interfaces (I7262)
//  Punto de entrada de la aplicacion: crea la ventana principal y la muestra.
// ============================================================================
#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication aplicacion(argc, argv);
    MainWindow ventana;
    ventana.show();
    return aplicacion.exec();
}
