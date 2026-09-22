#include "mainwindow.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

VentanaPrincipal::VentanaPrincipal(QWidget *padre) : QMainWindow(padre) {
    setWindowTitle("Ejemplo 2 - Diagnostico de red (Clase 10)");
    gestorRed = new QNetworkAccessManager(this);

    QWidget *central = new QWidget(this);
    QVBoxLayout *raiz = new QVBoxLayout(central);

    QHBoxLayout *fila = new QHBoxLayout();
    fila->addWidget(new QLabel("IP de la ESP32:", central));
    campoIp = new QLineEdit("192.168.0.50", central);
    botonConsultar = new QPushButton("Consultar", central);
    fila->addWidget(campoIp);
    fila->addWidget(botonConsultar);
    raiz->addLayout(fila);

    cuadroInfo = new QTextEdit(central);
    cuadroInfo->setReadOnly(true);
    cuadroInfo->setStyleSheet("font-family:monospace;");
    cuadroInfo->setPlaceholderText("Aqui apareceran los datos de red que reporta la ESP32 (GET /info).");
    raiz->addWidget(cuadroInfo);

    setCentralWidget(central);
    resize(460, 320);

    connect(botonConsultar, &QPushButton::clicked, this, &VentanaPrincipal::alConsultar);
}

void VentanaPrincipal::alConsultar() {
    const QString ip = campoIp->text().trimmed();
    if (ip.isEmpty())
        return;
    cuadroInfo->setPlainText("Consultando...");
    QNetworkRequest peticion(QUrl("http://" + ip + "/info"));
    QNetworkReply *respuesta = gestorRed->get(peticion);
    connect(respuesta, &QNetworkReply::finished, this, [this, respuesta]() {
        if (respuesta->error() == QNetworkReply::NoError) {
            cuadroInfo->setPlainText(QString::fromUtf8(respuesta->readAll()));
        } else {
            cuadroInfo->setPlainText("No se pudo consultar la ESP32.\n"
                                     "Revisa la IP y que tu PC este en la red GWN571D04.\n\n"
                                     "Detalle: " + respuesta->errorString());
        }
        respuesta->deleteLater();
    });
}
