#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QLineEdit>
#include <QPropertyAnimation>
#include <QPalette>


QString estilo =
    "QLineEdit{"
        "border: 2px solid #2c3e50;"        //Borde azul por defecto
        "border-radius: 8px;"               //Borde redondeado
        "padding: 8px;"                     //Espaciado interno
        "background-color: #ecf0f1;"        //Fondo gris claro
        "color: #2c3e50;"                   //Color del texto
        "font-size: 14px;"                  //tama;o de la fuente
        "font-family: 'Segoe UI', sans-serif;"
    "}"
    "QLineEdit: hover{"
        "background-color: #d5d8dc;"
        "border:2px solid #2980b9;"
    "}"
    "QLineEdit:focus{"
        "background-color: #ffffff;"
        "border:2px solid #3498db;"
        "color: #2c3e50;"
    "}"
    ;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);


    ui->lineEdit->setPlaceholderText("Ingrese su nombre:...");
    ui->lineEdit->setStyleSheet(estilo);
    ui->lineEdit_2->setPlaceholderText("Ingrese su edad");
    ui->lineEdit_2->setStyleSheet(estilo);
    ui->lineEdit_3->setPlaceholderText("Ingrese su peso");
    ui->lineEdit_3->setStyleSheet(estilo);
    ui->lineEdit_4->setPlaceholderText("Ingrese su apellido");
    ui->lineEdit_4->setStyleSheet(estilo);
}

MainWindow::~MainWindow()
{
    delete ui;
}
