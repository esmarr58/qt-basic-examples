#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include <QTimer>
#include <QLocale>


void MainWindow::actualizarFecha(){

    //Paso # 4
    QDateTime tiempoActual = QDateTime::currentDateTime();
    QString tiempoTexto = tiempoActual.toString("dddd MMMM d hh:mm:ss yyyy");
    ui->label->setText(tiempoTexto);

    //Paso # 7.
    qint64 segundosDesdeElEpoch = tiempoActual.toSecsSinceEpoch();
    QString textoEpoch = QString::number(segundosDesdeElEpoch);
    ui->label->setText(textoEpoch);

    //Paso # 11.
    QDateTime tiempoFuturo = tiempoActual.addDays(10);
    QLocale locale(QLocale::Spanish, QLocale::Mexico); // Puedes cambiar 'Spain' por otros países si es necesario
    QString tiempoFuturoTexto = locale.toString(tiempoFuturo, "dddd, dd MMMM yyyy hh:mm:ss");
    ui->label->setText(tiempoFuturoTexto);



}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QTimer *cronometro = new QTimer(this);
    connect(cronometro, SIGNAL(timeout()),this, SLOT(actualizarFecha()));
    cronometro->start(1000);

}

MainWindow::~MainWindow()
{
    delete ui;
}
