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

    QSqlQuery consulta;
    consulta.prepare("INSERT INTO productos (nombre, precio, existencia, categoria) "
                     "VALUES (:nombre, :precio, :existencia, :categoria)");
    consulta.bindValue(":nombre", nombre);
    consulta.bindValue(":precio", ui->spinPrecio->value());
    consulta.bindValue(":existencia", ui->spinExistencia->value());
    consulta.bindValue(":categoria", ui->entradaCategoria->text().trimmed());

    if (consulta.exec()) {
        on_botonCargar_clicked();
        ui->etiquetaEstado->setText("Guardado con id " + consulta.lastInsertId().toString() + ".");
        ui->entradaNombre->clear();
        ui->spinPrecio->setValue(0);
        ui->spinExistencia->setValue(0);
        ui->entradaCategoria->clear();
        ui->entradaNombre->setFocus();
    } else {
        ui->etiquetaEstado->setText("No se guardó: " + consulta.lastError().text());
    }
}

void MainWindow::on_tablaProductos_cellClicked(int row, int column)
{
    Q_UNUSED(column);
    idSeleccionado = ui->tablaProductos->item(row, 0)->text().toInt();
    ui->etiquetaIdValor->setText(QString::number(idSeleccionado));
    ui->entradaNombre->setText(ui->tablaProductos->item(row, 1)->text());
    ui->spinPrecio->setValue(ui->tablaProductos->item(row, 2)->text().toDouble());
    ui->spinExistencia->setValue(ui->tablaProductos->item(row, 3)->text().toInt());
    ui->entradaCategoria->setText(ui->tablaProductos->item(row, 4)->text());
}

void MainWindow::on_botonActualizar_clicked()
{
    if (idSeleccionado < 0) {
        ui->etiquetaEstado->setText("Primero elige un producto en la tabla.");
        return;
    }
    const QString nombre = ui->entradaNombre->text().trimmed();
    if (nombre.isEmpty()) {
        ui->etiquetaEstado->setText("Escribe el nombre del producto.");
        ui->entradaNombre->setFocus();
        return;
    }

    QSqlQuery consulta;
    consulta.prepare("UPDATE productos SET nombre = :nombre, precio = :precio, "
                     "existencia = :existencia, categoria = :categoria WHERE id = :id");
    consulta.bindValue(":nombre", nombre);
    consulta.bindValue(":precio", ui->spinPrecio->value());
    consulta.bindValue(":existencia", ui->spinExistencia->value());
    consulta.bindValue(":categoria", ui->entradaCategoria->text().trimmed());
    consulta.bindValue(":id", idSeleccionado);

    if (consulta.exec()) {
        on_botonCargar_clicked();
        ui->etiquetaEstado->setText(QString("Filas actualizadas: %1.").arg(consulta.numRowsAffected()));
    } else {
        ui->etiquetaEstado->setText("No se actualizó: " + consulta.lastError().text());
    }
}

void MainWindow::on_botonEliminar_clicked()
{
    if (idSeleccionado < 0) {
        ui->etiquetaEstado->setText("Primero elige un producto en la tabla.");
        return;
    }
    QSqlQuery consulta;
    consulta.prepare("DELETE FROM productos WHERE id = :id");
    consulta.bindValue(":id", idSeleccionado);
    if (consulta.exec()) {
        on_botonCargar_clicked();
        ui->etiquetaEstado->setText(QString("Filas eliminadas: %1.").arg(consulta.numRowsAffected()));
        idSeleccionado = -1;
        ui->etiquetaIdValor->setText("(ninguno)");
    } else {
        ui->etiquetaEstado->setText("No se eliminó: " + consulta.lastError().text());
    }
}
