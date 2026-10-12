#include "mainwindow.h"

#include <QComboBox>
#include <QDateTime>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QPushButton>
#include <QSerialPortInfo>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), puerto(new QSerialPort(this))
{
    comboPuertos = new QComboBox;
    for (const QSerialPortInfo &info : QSerialPortInfo::availablePorts())
        comboPuertos->addItem(info.portName());
    botonConectar    = new QPushButton("Conectar");
    botonSincronizar = new QPushButton("Sincronizar hora");
    etiquetaTiempo   = new QLabel("t0: -");
    etiquetaEstado   = new QLabel("Sin datos");

    // --- Grafica con eje X de FECHA/HORA ---
    serieDist = new QLineSeries;
    serieDist->setName("Distancia (cm)");

    QChart *grafico = new QChart;
    grafico->addSeries(serieDist);
    grafico->setTitle("Lotes: x = t0 + i*dt");

    ejeTiempo = new QDateTimeAxis;
    ejeTiempo->setFormat("HH:mm:ss");          // la zona horaria se aplica AL MOSTRAR
    ejeTiempo->setTitleText("hora local");
    ejeTiempo->setTickCount(6);
    grafico->addAxis(ejeTiempo, Qt::AlignBottom);
    serieDist->attachAxis(ejeTiempo);

    ejeDist = new QValueAxis;
    ejeDist->setTitleText("cm");
    ejeDist->setRange(0, 120);
    grafico->addAxis(ejeDist, Qt::AlignLeft);
    serieDist->attachAxis(ejeDist);

    QChartView *vista = new QChartView(grafico);
    vista->setRenderHint(QPainter::Antialiasing);

    QHBoxLayout *barra = new QHBoxLayout;
    barra->addWidget(comboPuertos);
    barra->addWidget(botonConectar);
    barra->addWidget(botonSincronizar);
    barra->addWidget(etiquetaEstado, 1);
    QVBoxLayout *raiz = new QVBoxLayout;
    raiz->addLayout(barra);
    raiz->addWidget(etiquetaTiempo);
    raiz->addWidget(vista, 1);
    QWidget *central = new QWidget;
    central->setLayout(raiz);
    setCentralWidget(central);
    resize(980, 560);

    connect(botonConectar, &QPushButton::clicked, this, &MainWindow::conectar);
    connect(botonSincronizar, &QPushButton::clicked, this, &MainWindow::sincronizarHora);
    connect(puerto, &QSerialPort::readyRead, this, &MainWindow::leerDatos);
}

void MainWindow::conectar()
{
    if (puerto->isOpen()) puerto->close();
    puerto->setPortName(comboPuertos->currentText());
    puerto->setBaudRate(QSerialPort::Baud115200);
    if (puerto->open(QIODevice::ReadWrite))
        etiquetaEstado->setText("Conectado a " + puerto->portName() + " (sin sincronizar)");
    else
        etiquetaEstado->setText("Error: " + puerto->errorString());
}

// Le decimos a la ESP32 la hora real: epoch en ms (UTC)
void MainWindow::sincronizarHora()
{
    if (!puerto->isOpen()) return;
    QJsonObject cmd;
    cmd["cmd"] = "sync";
    cmd["epoch_ms"] = QDateTime::currentMSecsSinceEpoch();
    puerto->write(QJsonDocument(cmd).toJson(QJsonDocument::Compact) + '\n');

    serieDist->clear();          // los puntos viejos estan en 1970: se descartan
    ultimoSeq = -1;
}

void MainWindow::leerDatos()
{
    while (puerto->canReadLine()) {
        QByteArray linea = puerto->readLine().trimmed();
        QJsonDocument doc = QJsonDocument::fromJson(linea);
        if (!doc.isObject()) continue;
        QJsonObject obj = doc.object();

        if (obj.contains("dist") && obj["dist"].isArray())
            procesarLote(obj);
        else if (obj["ack"].toString() == "sync")
            etiquetaEstado->setText("ESP32 sincronizada");
    }
}

void MainWindow::procesarLote(const QJsonObject &lote)
{
    const qint64 seq = lote["seq"].toInteger();
    const qint64 t0  = lote["t0"].toInteger();    // ms desde 1970 (13 digitos si esta sincronizada)
    const qint64 dt  = lote["dt"].toInteger();
    const QJsonArray dist = lote["dist"].toArray();

    // 1) Lotes perdidos: seq debe crecer de 1 en 1
    if (ultimoSeq >= 0 && seq > ultimoSeq + 1)
        lotesPerdidos += int(seq - ultimoSeq - 1);
    ultimoSeq = seq;
    lotesRecibidos++;

    // 2) Reconstruir el tiempo de CADA muestra: t0 + i*dt
    QList<QPointF> puntos;
    for (int i = 0; i < dist.size(); i++) {
        if (!dist[i].isDouble()) continue;        // null = lectura fallida: se omite (la linea une a los vecinos)
        puntos.append(QPointF(t0 + i * dt, dist[i].toDouble()));
    }

    // 3) Agregar TODO el lote de una vez (un solo redibujado)
    serieDist->append(puntos);

    // 4) Ventana de tiempo: borrar lo mas viejo que VENTANA_MS
    const qint64 tFinal = t0 + (dist.size() - 1) * dt;
    const qint64 tInicio = tFinal - VENTANA_MS;
    int viejos = 0;
    while (viejos < serieDist->count() && serieDist->at(viejos).x() < tInicio)
        viejos++;
    if (viejos > 0) serieDist->removePoints(0, viejos);

    ejeTiempo->setRange(QDateTime::fromMSecsSinceEpoch(tInicio),
                        QDateTime::fromMSecsSinceEpoch(tFinal));

    etiquetaTiempo->setText(QString("t0 = %1  ->  %2 (hora local)   |   sinc: %3")
        .arg(t0)
        .arg(QDateTime::fromMSecsSinceEpoch(t0).toString("yyyy-MM-dd HH:mm:ss.zzz"))
        .arg(lote["sinc"].toBool() ? "si" : "NO"));
    actualizarEstado();
}

void MainWindow::actualizarEstado()
{
    etiquetaEstado->setText(QString("lotes: %1   perdidos: %2   puntos en pantalla: %3")
        .arg(lotesRecibidos).arg(lotesPerdidos).arg(serieDist->count()));
}
