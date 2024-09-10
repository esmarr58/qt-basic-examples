#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*
void MainWindow::on_pushButton_clicked()
{
    ui->label->setText("Hola mundo!");
}
*/

void MainWindow::on_pushButton_clicked(bool checked)
{
    if(checked){
        //Verdadero
        ui->label->setText("Boton presionado");
    }
    else{
        //Falso
        ui->label->setText("Boton desactivado");
    }
}

