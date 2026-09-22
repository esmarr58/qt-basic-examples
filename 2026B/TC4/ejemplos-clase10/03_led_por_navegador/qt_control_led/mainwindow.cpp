#include "mainwindow.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Ejemplo 3 - Control del LED (WebSocket + JSON)");

    QWidget *central = new QWidget(this);
    QVBoxLayout *raiz = new QVBoxLayout(central);

    QHBoxLayout *fila = new QHBoxLayout();
    fila->addWidget(new QLabel("IP de la ESP32:", central));
    campoIp = new QLineEdit("192.168.0.50", central);
    botonConectar = new QPushButton("Conectar", central);
    fila->addWidget(campoIp);
    fila->addWidget(botonConectar);
    raiz->addLayout(fila);

    QHBoxLayout *filaBotones = new QHBoxLayout();
    botonEncender = new QPushButton("ENCENDER", central);
    botonApagar   = new QPushButton("APAGAR", central);
    botonEncender->setMinimumHeight(48);
    botonApagar->setMinimumHeight(48);
    filaBotones->addWidget(botonEncender);
    filaBotones->addWidget(botonApagar);
    raiz->addLayout(filaBotones);

    etiquetaEstado = new QLabel("Conecta con la IP de la ESP32 y usa los botones.", central);
    etiquetaEstado->setAlignment(Qt::AlignCenter);
    etiquetaEstado->setWordWrap(true);
    raiz->addWidget(etiquetaEstado);

    setCentralWidget(central);
    resize(470, 240);

    connect(botonConectar, &QPushButton::clicked, this, &MainWindow::alConectar);
    connect(botonEncender, &QPushButton::clicked, this, &MainWindow::encender);
    connect(botonApagar,   &QPushButton::clicked, this, &MainWindow::apagar);
    connect(&socket, &QWebSocket::connected, this, &MainWindow::alConectarSocket);
    connect(&socket, &QWebSocket::textMessageReceived, this, &MainWindow::alRecibirMensaje);
}

void MainWindow::alConectar() {
    const QString ip = campoIp->text().trimmed();
    if (ip.isEmpty())
        return;
    socket.open(QUrl("ws://" + ip + ":81"));
    etiquetaEstado->setText("Conectando a ws://" + ip + ":81 ...");
}

void MainWindow::alConectarSocket() {
    etiquetaEstado->setText("Conectado. Usa ENCENDER / APAGAR.");
}

void MainWindow::enviarLed(bool encendido) {
    if (socket.state() != QAbstractSocket::ConnectedState) {
        etiquetaEstado->setText("Primero conecta con la ESP32.");
        return;
    }
    QJsonObject comando;
    comando["tipo"]      = "led";
    comando["encendido"] = encendido;
    socket.sendTextMessage(QJsonDocument(comando).toJson(QJsonDocument::Compact));
}

void MainWindow::encender() { enviarLed(true); }
void MainWindow::apagar()   { enviarLed(false); }

void MainWindow::alRecibirMensaje(const QString &mensaje) {
    const QJsonObject obj = QJsonDocument::fromJson(mensaje.toUtf8()).object();
    if (obj.value("tipo").toString() == "estado_led")
        etiquetaEstado->setText(obj.value("encendido").toBool() ? "LED ENCENDIDO" : "LED APAGADO");
}
