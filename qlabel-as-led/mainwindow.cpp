#include "mainwindow.h"
#include "ui_mainwindow.h"


QString ledOn =
    "QLabel{"
        "background-color: #2ecc71;"
        "border-radius: 40px;"
        "border:2px solid #27ae60;"
        "min-width: 40px;"
        "min-height: 40px;"
    "}" ;
QString ledOff =
    "QLabel{"
    "background-color: #bdc3c7;"
    "border-radius: 40px;"
    "border:2px solid #7f8c8d;"
    "min-width: 40px;"
    "min-height: 40px;"
    "}" ;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->label->setStyleSheet(ledOff);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked(bool checked)
{
    if(checked){
        ui->label->setStyleSheet(ledOn);
    }
    else{
        ui->label->setStyleSheet(ledOff);

    }
}

