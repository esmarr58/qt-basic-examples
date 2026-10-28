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
    QSqlQuery consulta("SELECT id, nombre, precio, existencia, categoria FROM productos");
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

void MainWindow::on_botonGuardar_clicked()
{
    const QString nombre = ui->entradaNombre->text().trimmed();
    if (nombre.isEmpty()) {
        ui->etiquetaEstado->setText("Escribe el nombre del producto.");
        ui->entradaNombre->setFocus();
        return;
    }

    // VARIANTE (no la uses): texto pegado con + → una comilla rompe el SQL (error 1064) o permite inyección SQL
    QSqlQuery consulta;
    const QString sql = "INSERT INTO productos (nombre, precio, existencia, categoria) VALUES ('" + nombre + "', "
                        + QString::number(ui->spinPrecio->value()) + ", " + QString::number(ui->spinExistencia->value())
                        + ", '" + ui->entradaCategoria->text().trimmed() + "')";

    if (consulta.exec(sql)) {
        ui->etiquetaEstado->setText("Guardado con id " + consulta.lastInsertId().toString() + ".");
        ui->entradaNombre->clear();
        ui->spinPrecio->setValue(0);
        ui->spinExistencia->setValue(0);
        ui->entradaCategoria->clear();
        ui->entradaNombre->setFocus();
        on_botonCargar_clicked();
    } else {
        ui->etiquetaEstado->setText("No se guardó: " + consulta.lastError().text());
    }
}
