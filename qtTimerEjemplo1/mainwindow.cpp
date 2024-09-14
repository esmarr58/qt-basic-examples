#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QMessageBox>

void MainWindow::mostrarMensaje(){
    //QMessageBox::information(this, "Advertencia", "Mensaje despues de 3 segundos"); //Descomentar en el primer caso.
    static int contador = 0;
    ui->lcdNumber->display(++contador);

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //QTimer::singleShot(3000, this, SLOT(mostrarMensaje()));
    QTimer *cronometro = new QTimer(this);
    cronometro->start(1000);
    connect(cronometro, SIGNAL(timeout()),this, SLOT(mostrarMensaje()));





}

MainWindow::~MainWindow()
{
    delete ui;
}
