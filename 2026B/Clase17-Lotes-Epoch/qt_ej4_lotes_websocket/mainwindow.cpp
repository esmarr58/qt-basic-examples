// ============================================================================
//  Clase 17 - Ejemplo 4 - Diseño de Interfaces (I7262)
//  Ventana principal: se conecta a la ESP32 por WebSocket (como el TC4),
//  recibe lotes {"tipo":"lote","seq","t0","dt","sinc","dist":[...]} y los grafica
//  en un eje de fecha y hora. El boton "Sincronizar hora" le manda a la ESP32
//  la hora real (epoch en ms) con {"tipo":"sincronizar","epoch_ms":...}.
// ============================================================================
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDateTime>
#include <QUrl>

// ----------------------------------------------------------------------------
// Constructor: PRIMERO setupUi (construye la interfaz del formulario .ui) y
// DESPUES la configuracion: crear objetos y conectar señales con las funciones.
// ----------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // ---------- Configuracion posterior a setupUi ----------
    conectado = false;
    ultimoSeq = -1;
    lotesRecibidos = 0;
    lotesPerdidos = 0;

    // 1) Cliente WebSocket y sus señales
    socket = new QWebSocket();
    connect(socket, &QWebSocket::connected,           this, &MainWindow::alConectar);
    connect(socket, &QWebSocket::disconnected,        this, &MainWindow::alDesconectar);
    connect(socket, &QWebSocket::textMessageReceived, this, &MainWindow::alRecibirMensaje);
    connect(socket, &QWebSocket::errorOccurred,       this, &MainWindow::alOcurrirError);

    // 2) Grafica con eje X de fecha y hora
    configurarGrafica();

    // 3) Botones
    connect(ui->botonConectar,    &QPushButton::clicked, this, &MainWindow::alPresionarConectar);
    connect(ui->botonSincronizar, &QPushButton::clicked, this, &MainWindow::alPresionarSincronizar);

    registrar("Aplicacion lista. Escribe la IP de la ESP32 y presiona Conectar.");
}

MainWindow::~MainWindow()
{
    socket->close();
    delete ui;
}

// ----------------------------------------------------------------------------
// Grafica: el eje X es QDateTimeAxis; la x de cada punto = epoch en ms
// ----------------------------------------------------------------------------
void MainWindow::configurarGrafica()
{
    serieDistancia = new QLineSeries();
    serieDistancia->setName("Distancia (cm)");

    QChart *grafico = new QChart();
    grafico->addSeries(serieDistancia);
    grafico->setTitle("Lotes por WebSocket: muestra i en t0 + i*dt");

    ejeTiempo = new QDateTimeAxis();
    ejeTiempo->setFormat("HH:mm:ss");          // la zona horaria se aplica AL MOSTRAR
    ejeTiempo->setTitleText("hora local");
    ejeTiempo->setTickCount(6);
    grafico->addAxis(ejeTiempo, Qt::AlignBottom);
    serieDistancia->attachAxis(ejeTiempo);

    ejeDistancia = new QValueAxis();
    ejeDistancia->setTitleText("cm");
    ejeDistancia->setRange(0, 120);
    grafico->addAxis(ejeDistancia, Qt::AlignLeft);
    serieDistancia->attachAxis(ejeDistancia);

    ui->vistaGrafica->setChart(grafico);          // QChartView promovido en el .ui
    ui->vistaGrafica->setRenderHint(QPainter::Antialiasing);
}

// ----------------------------------------------------------------------------
// Boton Conectar / Desconectar
// ----------------------------------------------------------------------------
void MainWindow::alPresionarConectar()
{
    if (conectado) {          // si ya estamos conectados, el boton desconecta
        socket->close();
        return;
    }
    QString ip     = ui->lineEditIp->text().trimmed();
    QString puerto = ui->lineEditPuerto->text().trimmed();
    if (ip.isEmpty()) {
        registrar("Escribe la IP de la ESP32 (aparece en el Monitor Serie).");
        return;
    }
    QUrl direccion(QString("ws://%1:%2/").arg(ip, puerto));
    registrar("Conectando a " + direccion.toString() + " ...");
    socket->open(direccion);
}

void MainWindow::alConectar()
{
    conectado = true;
    ui->etiquetaEstado->setText("Conectado (sin sincronizar)");
    ui->botonConectar->setText("Desconectar");
    registrar("Conectado a la ESP32.");
}

void MainWindow::alDesconectar()
{
    conectado = false;
    ui->etiquetaEstado->setText("Desconectado");
    ui->botonConectar->setText("Conectar");
    registrar("Desconectado.");
}

void MainWindow::alOcurrirError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    registrar("Error de conexion: " + socket->errorString());
}

// ----------------------------------------------------------------------------
// Envio de comandos JSON
// ----------------------------------------------------------------------------
void MainWindow::enviarJson(const QJsonObject &objeto)
{
    if (!conectado) {
        registrar("No hay conexion. Conecta primero.");
        return;
    }
    QJsonDocument documento(objeto);
    QString texto = QString::fromUtf8(documento.toJson(QJsonDocument::Compact));
    socket->sendTextMessage(texto);
    registrar("-> " + texto);
}

// Le decimos a la ESP32 la hora real: epoch en ms (UTC)
void MainWindow::alPresionarSincronizar()
{
    QJsonObject objeto;
    objeto["tipo"]     = "sincronizar";
    objeto["epoch_ms"] = QDateTime::currentMSecsSinceEpoch();
    enviarJson(objeto);

    serieDistancia->clear();      // los puntos viejos estan en 1970: se descartan
    ultimoSeq = -1;
}

// ----------------------------------------------------------------------------
// (4) Recepcion e interpretacion: cada mensaje WebSocket = un lote COMPLETO
// ----------------------------------------------------------------------------
void MainWindow::alRecibirMensaje(const QString &mensaje)
{
    QJsonDocument documento = QJsonDocument::fromJson(mensaje.toUtf8());
    if (!documento.isObject()) {
        registrar("JSON invalido: " + mensaje);
        return;
    }
    interpretarEvento(documento.object());
}

void MainWindow::interpretarEvento(const QJsonObject &objeto)
{
    QString tipo = objeto.value("tipo").toString();
    if (tipo == "lote") {
        procesarLote(objeto);
    }
    else if (tipo == "sincronizado") {
        ui->etiquetaEstado->setText("Conectado y sincronizado");
        registrar("<- ESP32 sincronizada");
    }
    else if (tipo == "saludo") {
        registrar("<- " + objeto.value("mensaje").toString());
    }
}

// ----------------------------------------------------------------------------
// (4b) Reconstruir el tiempo de cada muestra y (5) graficar el lote completo
// ----------------------------------------------------------------------------
void MainWindow::procesarLote(const QJsonObject &objeto)
{
    const qint64 seq = objeto.value("seq").toInteger();
    const qint64 t0  = objeto.value("t0").toInteger();    // 13 digitos: no cabe en int
    const qint64 dt  = objeto.value("dt").toInteger();
    const QJsonArray distancias = objeto.value("dist").toArray();

    // Lotes perdidos: seq debe crecer de 1 en 1
    if (ultimoSeq >= 0 && seq > ultimoSeq + 1)
        lotesPerdidos += int(seq - ultimoSeq - 1);
    ultimoSeq = seq;
    lotesRecibidos++;

    // (4b) La muestra i ocurrio en t0 + i*dt
    QList<QPointF> puntos;
    for (int i = 0; i < distancias.size(); i++) {
        if (!distancias[i].isDouble()) continue;     // null = lectura fallida: se omite
        puntos.append(QPointF(t0 + i * dt, distancias[i].toDouble()));
    }

    // (5) Agregar TODO el lote de una vez (un solo redibujado)
    serieDistancia->append(puntos);

    // Ventana de tiempo: borrar lo mas viejo que VENTANA_MS
    const qint64 tFinal  = t0 + (distancias.size() - 1) * dt;
    const qint64 tInicio = tFinal - VENTANA_MS;
    int viejos = 0;
    while (viejos < serieDistancia->count() && serieDistancia->at(viejos).x() < tInicio)
        viejos++;
    if (viejos > 0) serieDistancia->removePoints(0, viejos);

    ejeTiempo->setRange(QDateTime::fromMSecsSinceEpoch(tInicio),
                        QDateTime::fromMSecsSinceEpoch(tFinal));

    ui->etiquetaTiempo->setText(QString("t0 = %1  ->  %2 (hora local)   |   sinc: %3")
        .arg(t0)
        .arg(QDateTime::fromMSecsSinceEpoch(t0).toString("yyyy-MM-dd HH:mm:ss.zzz"))
        .arg(objeto.value("sinc").toBool() ? "si" : "NO"));
    ui->etiquetaLotes->setText(QString("lotes: %1   perdidos: %2   puntos en pantalla: %3")
        .arg(lotesRecibidos).arg(lotesPerdidos).arg(serieDistancia->count()));
}

// ----------------------------------------------------------------------------
// Registro de mensajes en pantalla (con hora)
// ----------------------------------------------------------------------------
void MainWindow::registrar(const QString &linea)
{
    QString hora = QDateTime::currentDateTime().toString("HH:mm:ss");
    ui->registro->append("[" + hora + "] " + linea);
}
