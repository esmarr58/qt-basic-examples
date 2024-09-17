#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug() << construirMensaje(4,1);
    cambiarFondoEtiqueta(ui->label,0);
    cambiarFondoEtiqueta(ui->label_2,0);
    cambiarFondoEtiqueta(ui->label_3,0);
    cambiarFondoEtiqueta(ui->label_4,0);
    cambiarFondoEtiqueta(ui->label_5,0);

}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::construirMensaje(int pin, int estado){
    QString mensajeJSON = QString("{\"pin\":%1,\"estado\":%2}").arg(pin).arg(estado);
    //QString mensajeJSON = "{\"pin\":"+QString::number(pin)+"\"estado\":"+QString::number(estado)+"}";
    return mensajeJSON;
}

void MainWindow::cambiarFondoEtiqueta(QLabel *etiqueta, bool estado){
    if(estado){
        //true
        etiqueta->setStyleSheet(
            "QLabel{"
                "background-color: #2ecc71;"
                "border-radius: 40px;"
                "border: 2px solid #27ae60;"
            "}"
            );
    }
    else{
        //false
        etiqueta->setStyleSheet(
            "QLabel{"
            "background-color: #bdc3c7;"
            "border-radius: 40px;"
            "border: 2px solid #7f8c8d;"
            "}"
            );
    }

}

void MainWindow::actualizarBoton(QPushButton *boton, QLabel *etiqueta, int pin){
    bool estadoBoton = boton->isChecked();
    if(estadoBoton) boton->setText("OFF");
    else            boton->setText("ON");
    cambiarFondoEtiqueta(etiqueta, estadoBoton);
    QString mensajeJSON = construirMensaje(pin, estadoBoton);
    QString ipESP32 = "http://Buscar la ip en el monitor serial de Arduino";
    QString nombreRed = "buscar con el comando ip address show";
    conexionWeb(ipESP32, mensajeJSON, nombreRed);

}

QString MainWindow::conexionWeb(QString url, QString mensaje, QString nombreRed) {
    QString respuesta = "error 1";

    // Paso #1: Verificar que la red sea una red funcional.
    QNetworkInterface redConectada = QNetworkInterface::interfaceFromName(nombreRed);
    QList<QNetworkAddressEntry> lista = redConectada.addressEntries();

    // Paso #2: Verificar que la red esté activa.
    if (!lista.empty()) {
        QNetworkAddressEntry IP = lista.first();
        qDebug() << "Red activa: " << IP.ip();

        // Crear el mensaje HTML/HTTP
        QByteArray longitudMensaje = QByteArray::number(mensaje.size());
        QNetworkRequest solicitud;
        QNetworkAccessManager *clienteWeb = new QNetworkAccessManager(this); // Manejo de memoria automático
        QUrl servidor(url.toUtf8().constData());

        // Paso #3: Verificar que la URL sea válida
        if (servidor.isValid()) {
            qDebug() << "Servidor válido";

            // Paso #4: Formar el mensaje HTTP
            solicitud.setUrl(servidor);
            solicitud.setRawHeader(QByteArray("User-Agent"), QByteArray("bot"));
            solicitud.setRawHeader(QByteArray("Connection"), QByteArray("close"));
            solicitud.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            solicitud.setHeader(QNetworkRequest::ContentLengthHeader, longitudMensaje);

            // Paso #5: Realizar la conexión
            QNetworkReply *conexionServidor = clienteWeb->post(solicitud, mensaje.toLatin1());

            // Paso #6: Esperar a que el servidor responda
            QEventLoop funcionLoop;
            QObject::connect(conexionServidor, &QNetworkReply::finished, &funcionLoop, &QEventLoop::quit);
            funcionLoop.exec();

            // Verificar errores en la respuesta
            if (conexionServidor->error() == QNetworkReply::NoError) {
                // Paso #7: Leer la respuesta del servidor
                QByteArray datosWeb = conexionServidor->readAll();
                respuesta = QString(datosWeb);
            } else {
                respuesta = "Error en la conexión: " + conexionServidor->errorString();
            }

            conexionServidor->deleteLater();
        } else {
            respuesta = "error 3";
        }
    } else {
        respuesta = "Error 2";
    }

    return respuesta;
}

void MainWindow::on_pushButton_clicked()
{
    actualizarBoton(ui->pushButton,ui->label,13);
}


void MainWindow::on_pushButton_2_clicked()
{
    actualizarBoton(ui->pushButton_2,ui->label_2,12);

}


void MainWindow::on_pushButton_3_clicked()
{
    actualizarBoton(ui->pushButton_3,ui->label_3,11);

}


void MainWindow::on_pushButton_4_clicked()
{
    actualizarBoton(ui->pushButton_4,ui->label_4,10);

}


void MainWindow::on_pushButton_5_clicked()
{
    actualizarBoton(ui->pushButton_5,ui->label_5,9);

}

