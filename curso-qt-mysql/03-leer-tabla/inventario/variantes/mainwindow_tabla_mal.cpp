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

void MainWindow::on_botonCargar_clicked()
{
    QSqlQuery consulta("SELECT id, nombre, precio, existencia, categoria FROM producto");
    if (!consulta.isActive()) {
        ui->etiquetaEstado->setText("Error: " + consulta.lastError().text());
        return;
    }

    ui->tablaProductos->setRowCount(0);
    while (consulta.next()) {
        const int fila = ui->tablaProductos->rowCount();
        ui->tablaProductos->insertRow(fila);
        for (int columna = 0; columna < 5; ++columna) {
            ui->tablaProductos->setItem(fila, columna,
                                        new QTableWidgetItem(consulta.value(columna).toString()));
        }
    }
    ui->etiquetaEstado->setText(QString("Se leyeron %1 productos.").arg(ui->tablaProductos->rowCount()));
}
