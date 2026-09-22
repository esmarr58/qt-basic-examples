#include "mainwindow.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

VentanaPrincipal::VentanaPrincipal(QWidget *padre) : QMainWindow(padre) {
    setWindowTitle("Ejemplo 3 - Control del LED (Clase 10)");
    gestorRed = new QNetworkAccessManager(this);

    QWidget *central = new QWidget(this);
    QVBoxLayout *raiz = new QVBoxLayout(central);

    QHBoxLayout *fila = new QHBoxLayout();
    fila->addWidget(new QLabel("IP de la ESP32:", central));
    campoIp = new QLineEdit("192.168.0.50", central);
    fila->addWidget(campoIp);
    raiz->addLayout(fila);

    QHBoxLayout *filaBotones = new QHBoxLayout();
    botonEncender = new QPushButton("ENCENDER", central);
    botonApagar   = new QPushButton("APAGAR", central);
    botonEncender->setMinimumHeight(48);
    botonApagar->setMinimumHeight(48);
    filaBotones->addWidget(botonEncender);
    filaBotones->addWidget(botonApagar);
    raiz->addLayout(filaBotones);

    etiquetaEstado = new QLabel("Escribe la IP de la ESP32 y usa los botones.", central);
    etiquetaEstado->setAlignment(Qt::AlignCenter);
    etiquetaEstado->setWordWrap(true);
    raiz->addWidget(etiquetaEstado);

    setCentralWidget(central);
    resize(440, 220);

    connect(botonEncender, &QPushButton::clicked, this, &VentanaPrincipal::encender);
    connect(botonApagar,   &QPushButton::clicked, this, &VentanaPrincipal::apagar);
}

void VentanaPrincipal::encender() {
    enviarComando("/encender", "LED ENCENDIDO");
}

void VentanaPrincipal::apagar() {
    enviarComando("/apagar", "LED APAGADO");
}

void VentanaPrincipal::enviarComando(const QString &ruta, const QString &mensajeOk) {
    const QString ip = campoIp->text().trimmed();
    if (ip.isEmpty())
        return;
    QNetworkRequest peticion(QUrl("http://" + ip + ruta));
    QNetworkReply *respuesta = gestorRed->get(peticion);
    connect(respuesta, &QNetworkReply::finished, this, [this, respuesta, mensajeOk]() {
        if (respuesta->error() == QNetworkReply::NoError) {
            etiquetaEstado->setText(mensajeOk);
        } else {
            etiquetaEstado->setText("Sin conexion. Revisa la IP y la red GWN571D04.");
        }
        respuesta->deleteLater();
    });
}
