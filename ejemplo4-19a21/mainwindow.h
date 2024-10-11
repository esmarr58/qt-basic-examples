#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
//Paso # 4. Agregar una biblioteca para QFileDialog
#include <QFileDialog>

//PE - Paso # 1
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

    //Paso # 1. Crear una funcion privada, llamada buscarImagen
private slots:
    void buscarImagen();

private:
    Ui::MainWindow *ui;


};
#endif // MAINWINDOW_H
