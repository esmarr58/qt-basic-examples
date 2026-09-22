#include "mainwindow.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

VentanaPrincipal::VentanaPrincipal(QWidget *padre) : QMainWindow(padre) {
    setWindowTitle("Ejemplo 1 - Espejo del boton (Clase 10)");
    gestorRed = new QNetworkAccessManager(this);
    temporizador = new QTimer(this);
    temporizador->setInterval(500);

    QWidget *central = new QWidget(this);
    QVBoxLayout *raiz = new QVBoxLayout(central);

    QHBoxLayout *fila = new QHBoxLayout();
    fila->addWidget(new QLabel("IP de la ESP32:", central));
    campoIp = new QLineEdit("192.168.0.50", central);
    botonConectar = new QPushButton("Conectar", central);
    fila->addWidget(campoIp);
    fila->addWidget(botonConectar);
    raiz->addLayout(fila);

    indicador = new QLabel("APAGADO", central);
    indicador->setAlignment(Qt::AlignCenter);
    indicador->setStyleSheet("font-size:36px; font-weight:bold; padding:40px;"
                             "background:#dddddd; border-radius:12px;");
    raiz->addWidget(indicador);

    etiquetaAyuda = new QLabel("Conecta y presiona el boton fisico de la ESP32: "
                               "la ventana reflejara el estado del LED por la red.", central);
    etiquetaAyuda->setWordWrap(true);
    raiz->addWidget(etiquetaAyuda);

    setCentralWidget(central);
    resize(440, 280);

    connect(botonConectar, &QPushButton::clicked, this, &VentanaPrincipal::alConectar);
    connect(temporizador, &QTimer::timeout, this, &VentanaPrincipal::consultarEstado);
}

void VentanaPrincipal::alConectar() {
    temporizador->start();
    consultarEstado();
}

void VentanaPrincipal::consultarEstado() {
    const QString ip = campoIp->text().trimmed();
    if (ip.isEmpty())
        return;
    QNetworkRequest peticion(QUrl("http://" + ip + "/estado"));
    QNetworkReply *respuesta = gestorRed->get(peticion);
    connect(respuesta, &QNetworkReply::finished, this, [this, respuesta]() {
        if (respuesta->error() == QNetworkReply::NoError) {
            const QString texto = QString::fromUtf8(respuesta->readAll()).trimmed();
            mostrarEstado(texto == "ON");
        } else {
            indicador->setText("sin conexion");
            indicador->setStyleSheet("font-size:24px; padding:40px;"
                                     "background:#f8d7da; border-radius:12px;");
        }
        respuesta->deleteLater();
    });
}

void VentanaPrincipal::mostrarEstado(bool encendido) {
    indicador->setText(encendido ? "ENCENDIDO" : "APAGADO");
    indicador->setStyleSheet(
        QString("font-size:36px; font-weight:bold; padding:40px; border-radius:12px;"
                "background:%1; color:%2;")
            .arg(encendido ? "#2e7d32" : "#dddddd")
            .arg(encendido ? "white" : "black"));
}
