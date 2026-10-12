#include "mainwindow.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QSerialPortInfo>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), puerto(new QSerialPort(this))
{
    // --- Barra superior: puerto + boton + ultima linea recibida ---
    comboPuertos = new QComboBox;
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts())
        comboPuertos->addItem(info.portName());
    botonConectar  = new QPushButton("Conectar");
    etiquetaUltima = new QLabel("Sin datos");

    // --- Grafica: serie + ejes ---
    serieT = new QLineSeries;
    serieT->setName("Temperatura (°C)");

    QChart *grafico = new QChart;
    grafico->addSeries(serieT);
    grafico->setTitle("Contrato v1: {\"t\": valor}");

    ejeX = new QValueAxis;
    ejeX->setTitleText("muestra");
    ejeX->setLabelFormat("%d");
    grafico->addAxis(ejeX, Qt::AlignBottom);
    serieT->attachAxis(ejeX);

    ejeY = new QValueAxis;
    ejeY->setTitleText("°C");
    ejeY->setRange(20, 30);
    grafico->addAxis(ejeY, Qt::AlignLeft);
    serieT->attachAxis(ejeY);

    QChartView *vista = new QChartView(grafico);
    vista->setRenderHint(QPainter::Antialiasing);

    // --- Layout ---
    QHBoxLayout *barra = new QHBoxLayout;
    barra->addWidget(comboPuertos);
    barra->addWidget(botonConectar);
    barra->addWidget(etiquetaUltima, 1);
    QVBoxLayout *raiz = new QVBoxLayout;
    raiz->addLayout(barra);
    raiz->addWidget(vista, 1);
    QWidget *central = new QWidget;
    central->setLayout(raiz);
    setCentralWidget(central);
    resize(900, 520);

    connect(botonConectar, &QPushButton::clicked, this, &MainWindow::conectar);
    connect(puerto, &QSerialPort::readyRead, this, &MainWindow::leerDatos);
}

void MainWindow::conectar()
{
    if (puerto->isOpen()) puerto->close();
    puerto->setPortName(comboPuertos->currentText());
    puerto->setBaudRate(QSerialPort::Baud115200);   // 8N1 es el valor por defecto
    if (puerto->open(QIODevice::ReadWrite))
        etiquetaUltima->setText("Conectado a " + puerto->portName());
    else
        etiquetaUltima->setText("Error: " + puerto->errorString());
}

// (4) Leer y parsear: solo lineas COMPLETAS (el serie llega en pedazos)
void MainWindow::leerDatos()
{
    while (puerto->canReadLine()) {
        QByteArray linea = puerto->readLine().trimmed();
        etiquetaUltima->setText(QString::fromUtf8(linea));

        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(linea, &err);
        if (err.error != QJsonParseError::NoError || !doc.isObject())
            continue;                                // linea rota: se ignora

        QJsonObject obj = doc.object();
        if (obj.contains("t"))
            graficar(obj["t"].toDouble());
    }
}

// (5) Graficar: agregar el punto y recorrer la ventana
void MainWindow::graficar(double t)
{
    serieT->append(muestra, t);
    if (serieT->count() > VENTANA)
        serieT->removePoints(0, serieT->count() - VENTANA);   // memoria acotada

    ejeX->setRange(qMax(0, muestra - VENTANA + 1), qMax(VENTANA - 1, muestra));
    muestra++;
}
