#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QtNetwork>
#include <QLabel>
#include <QPushButton>
#include <QDebug>

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
    QString construirMensaje(int pin, int estado);
    void cambiarFondoEtiqueta(QLabel *etiqueta, bool estado);
    void actualizarBoton(QPushButton *boton, QLabel *etiqueta, int pin);
    QString conexionWeb(QString url, QString mensaje, QString nombreRed);
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
