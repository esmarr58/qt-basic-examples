#include "mainwindow.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Ejemplo 2 - Diagnostico de red (WebSocket + JSON)");

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
    cuadroInfo->setPlaceholderText("Datos de red que reporta la ESP32 (JSON por WebSocket).");
    raiz->addWidget(cuadroInfo);

    setCentralWidget(central);
    resize(480, 340);

    connect(botonConsultar, &QPushButton::clicked, this, &MainWindow::alConsultar);
    connect(&socket, &QWebSocket::connected, this, &MainWindow::alConectarSocket);
    connect(&socket, &QWebSocket::textMessageReceived, this, &MainWindow::alRecibirMensaje);
}

void MainWindow::alConsultar() {
    const QString ip = campoIp->text().trimmed();
    if (ip.isEmpty())
        return;
    cuadroInfo->setPlainText("Consultando...");
    if (socket.state() != QAbstractSocket::ConnectedState) {
        socket.open(QUrl("ws://" + ip + ":81"));   // al conectar se pide "leer_red"
    } else {
        QJsonObject peticion; peticion["tipo"] = "leer_red";
        socket.sendTextMessage(QJsonDocument(peticion).toJson(QJsonDocument::Compact));
    }
}

void MainWindow::alConectarSocket() {
    QJsonObject peticion; peticion["tipo"] = "leer_red";
    socket.sendTextMessage(QJsonDocument(peticion).toJson(QJsonDocument::Compact));
}

void MainWindow::alRecibirMensaje(const QString &mensaje) {
    const QJsonObject obj = QJsonDocument::fromJson(mensaje.toUtf8()).object();
    if (obj.value("tipo").toString() != "datos_red")
        return;
    QString t;
    t += "SSID:    " + obj.value("ssid").toString()    + "\n";
    t += "IP:      " + obj.value("ip").toString()      + "\n";
    t += "Mascara: " + obj.value("mascara").toString() + "\n";
    t += "Gateway: " + obj.value("gateway").toString() + "\n";
    t += "DNS:     " + obj.value("dns").toString()     + "\n";
    t += "MAC:     " + obj.value("mac").toString()     + "\n";
    t += "RSSI:    " + QString::number(obj.value("rssi").toInt()) + " dBm\n";
    cuadroInfo->setPlainText(t);
}
