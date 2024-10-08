#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
//Paso # 1. Agregar la biblioteca para crear QTimer
#include <QTimer>
//Paso # 7. Agregar la biblioteca para leer el tiempo
#include <QDateTime>
//Paso # 10. Agregar la biblioteca qlocale
#include <QLocale>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void actualizarTiempo();
private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
