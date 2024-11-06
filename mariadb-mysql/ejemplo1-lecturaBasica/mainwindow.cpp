#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::loop(){
    double temperatura = QRandomGenerator::global()->generateDouble()*50;

    insertarTemperatura(temperatura);

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");       // Cambia a la IP de tu servidor MySQL si no es local
    db.setDatabaseName("interfaces");   // Reemplaza con el nombre de tu base de datos
    db.setUserName("admin");         // Reemplaza con tu usuario de MySQL
    db.setPassword("hola1234");      // Reemplaza con tu contraseña de MySQL

    if (!db.open()) {
        qDebug() << "Error al conectar con la base de datos:" << db.lastError().text();
    } else {
        qDebug() << "Conexión exitosa con la base de datos";
    }


    QTimer *cronometro = new QTimer(this);
    connect(cronometro, SIGNAL(timeout()),this, SLOT(loop()));
    cronometro->start(1000);
}

MainWindow::~MainWindow()
{
    delete ui;
}


bool MainWindow::insertarTemperatura(double temperatura) {
    // Obtener el timestamp en formato epoch (segundos desde 1970)
    qint64 timestamp = QDateTime::currentSecsSinceEpoch();

    // Crear la consulta SQL para insertar datos
    QSqlQuery query;
    query.prepare("INSERT INTO ejemplo1 (timestamp, temperatura) VALUES (:timestamp, :temperatura)");

    // Asignar los valores a los placeholders
    query.bindValue(":timestamp", timestamp);
    query.bindValue(":temperatura", temperatura);

    // Ejecutar la consulta e imprimir el resultado
    if (!query.exec()) {
        qDebug() << "Error al insertar en la tabla ejemplo1:" << query.lastError().text();
        return false;  // Si hubo un error, regresa false
    }

    qDebug() << "Datos insertados exitosamente en ejemplo1:" << "Timestamp:" << timestamp << ", Temperatura:" << temperatura;
    return true;  // Regresa true si la inserción fue exitosa
}
