#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

//Paso # 1. Biblioteca
#include<QFileDialog>

//PE-Paso#1
#include<QDateTime>

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
//Paso # 2. Crear una funcion privada, llamada abrirImagen
private slots:
    void abrirImagen();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
