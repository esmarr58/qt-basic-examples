// ============================================================================
//  Clase 16 - Ejemplo 3 - Diseño de Interfaces (I7262)
//  Ventana principal: se conecta a la ESP32 por WebSocket (como el TC4) y
//  grafica el contrato v2 {"tipo":"medicion","t":..,"h":..} con doble eje Y.
// ============================================================================
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>
#include <QJsonDocument>
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
    muestra = 0;

    // 1) Cliente WebSocket y sus señales
    socket = new QWebSocket();
    connect(socket, &QWebSocket::connected,           this, &MainWindow::alConectar);
    connect(socket, &QWebSocket::disconnected,        this, &MainWindow::alDesconectar);
    connect(socket, &QWebSocket::textMessageReceived, this, &MainWindow::alRecibirMensaje);
    connect(socket, &QWebSocket::errorOccurred,       this, &MainWindow::alOcurrirError);

    // 2) Grafica (series + ejes) dentro del QChartView del formulario
    configurarGrafica();

    // 3) Boton de conexion
    connect(ui->botonConectar, &QPushButton::clicked, this, &MainWindow::alPresionarConectar);

    registrar("Aplicacion lista. Escribe la IP de la ESP32 y presiona Conectar.");
}

MainWindow::~MainWindow()
{
    socket->close();
    delete ui;
}

// ----------------------------------------------------------------------------
// Grafica: dos series, un eje X comun y un eje Y para cada una
// ----------------------------------------------------------------------------
void MainWindow::configurarGrafica()
{
    serieTemperatura = new QLineSeries();
    serieTemperatura->setName("Temperatura (°C)");
    serieHumedad = new QLineSeries();
    serieHumedad->setName("Humedad (%)");

    QChart *grafico = new QChart();
    grafico->addSeries(serieTemperatura);
    grafico->addSeries(serieHumedad);
    grafico->setTitle("Contrato v2 por WebSocket: {\"tipo\":\"medicion\",\"t\":..,\"h\":..}");

    ejeX = new QValueAxis();
    ejeX->setTitleText("muestra");
    ejeX->setLabelFormat("%d");
    grafico->addAxis(ejeX, Qt::AlignBottom);
    serieTemperatura->attachAxis(ejeX);
    serieHumedad->attachAxis(ejeX);

    ejeYTemperatura = new QValueAxis();
    ejeYTemperatura->setTitleText("°C");
    ejeYTemperatura->setRange(20, 30);
    grafico->addAxis(ejeYTemperatura, Qt::AlignLeft);
    serieTemperatura->attachAxis(ejeYTemperatura);

    ejeYHumedad = new QValueAxis();
    ejeYHumedad->setTitleText("%");
    ejeYHumedad->setRange(0, 100);
    grafico->addAxis(ejeYHumedad, Qt::AlignRight);
    serieHumedad->attachAxis(ejeYHumedad);

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
    ui->etiquetaEstado->setText("Conectado");
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
// (4) Recepcion e interpretacion: cada mensaje WebSocket llega COMPLETO,
//     asi que no hace falta canReadLine() como en el puerto serie.
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
    if (tipo == "medicion") {
        double temperatura = objeto.value("t").toDouble();
        double humedad     = objeto.value("h").toDouble();
        ui->etiquetaMedicion->setText(QString("T = %1 °C   H = %2 %")
                                      .arg(temperatura, 0, 'f', 2)
                                      .arg(humedad, 0, 'f', 2));
        graficar(temperatura, humedad);
    }
    else if (tipo == "saludo") {
        registrar("<- " + objeto.value("mensaje").toString());
    }
}

// ----------------------------------------------------------------------------
// (5) Graficar: agregar el punto y recorrer la ventana de VENTANA muestras
// ----------------------------------------------------------------------------
void MainWindow::graficar(double temperatura, double humedad)
{
    serieTemperatura->append(muestra, temperatura);
    serieHumedad->append(muestra, humedad);
    if (serieTemperatura->count() > VENTANA) {
        serieTemperatura->removePoints(0, serieTemperatura->count() - VENTANA);   // memoria acotada
        serieHumedad->removePoints(0, serieHumedad->count() - VENTANA);
    }
    ejeX->setRange(qMax(0, muestra - VENTANA + 1), qMax(VENTANA - 1, muestra));
    muestra++;
}

// ----------------------------------------------------------------------------
// Registro de mensajes en pantalla (con hora)
// ----------------------------------------------------------------------------
void MainWindow::registrar(const QString &linea)
{
    QString hora = QDateTime::currentDateTime().toString("HH:mm:ss");
    ui->registro->append("[" + hora + "] " + linea);
}
