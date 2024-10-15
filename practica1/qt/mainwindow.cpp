#include "mainwindow.h"
#include "ui_mainwindow.h"

QString nombreRedPrincipal = "wlo1";
QString esp32 = "http://192.168.0.109";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    /*
    QList<QNetworkInterface> allInterfaces = QNetworkInterface::allInterfaces();
    foreach (const QNetworkInterface &interface, allInterfaces) {
        qDebug() << "Interface Name:" << interface.humanReadableName();
        qDebug() << "IP Address:" << interface.addressEntries().first().ip().toString();
    }
*/

    QList<QNetworkInterface> allInterfaces = QNetworkInterface::allInterfaces();
    bool interfaceFound = false;

    foreach (const QNetworkInterface &interface, allInterfaces) {
        if (interface.humanReadableName() == nombreRedPrincipal) {
            interfaceFound = true;
            qDebug() << "Interfaz encontrada:" << nombreRedPrincipal;
            // Aquí puedes continuar con la lógica de conexión
        }
    }

    if (!interfaceFound) {
        qDebug() << "No se encontró la interfaz:" << nombreRedPrincipal;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
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
    qDebug() << conexionWeb(esp32, "{\"comando\":1}", nombreRedPrincipal);
}


void MainWindow::on_pushButton_2_clicked()
{
    qDebug() << conexionWeb(esp32, "{\"comando\":2}", nombreRedPrincipal);

}


void MainWindow::on_pushButton_3_clicked()
{
    qDebug() << conexionWeb(esp32, "{\"comando\":3}", nombreRedPrincipal);

}


void MainWindow::on_pushButton_4_clicked()
{
    qDebug() << conexionWeb(esp32, "{\"comando\":4}", nombreRedPrincipal);

}

