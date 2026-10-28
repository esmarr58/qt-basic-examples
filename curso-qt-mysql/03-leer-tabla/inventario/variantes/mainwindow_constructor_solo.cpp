#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("tienda");
    db.setUserName("curso");
    db.setPassword("curso1234");

    if (db.open()) {
        ui->etiquetaEstado->setText("Conectado a la base tienda.");
    } else {
        ui->etiquetaEstado->setText("Sin conexión: " + db.lastError().text());
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
