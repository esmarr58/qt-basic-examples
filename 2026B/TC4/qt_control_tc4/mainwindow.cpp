// ============================================================================
//  TC4 - Diseño de Interfaces (I7262)
//  Ventana principal: se conecta a la ESP32 por WebSocket, envia comandos JSON
//  para controlar las salidas y reacciona a los botones de la placa (sonido).
// ============================================================================
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QCheckBox>
#include <QPushButton>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFileDialog>
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

    // 1) Cliente WebSocket y sus señales
    socket = new QWebSocket();
    connect(socket, &QWebSocket::connected,           this, &MainWindow::alConectar);
    connect(socket, &QWebSocket::disconnected,        this, &MainWindow::alDesconectar);
    connect(socket, &QWebSocket::textMessageReceived, this, &MainWindow::alRecibirMensaje);
    connect(socket, &QWebSocket::errorOccurred,       this, &MainWindow::alOcurrirError);

    // 2) Reproductor de sonido (QMediaPlayer + salida de audio)
    reproductor = new QMediaPlayer(this);
    salidaAudio = new QAudioOutput(this);
    reproductor->setAudioOutput(salidaAudio);
    salidaAudio->setVolume(0.8);

    // 3) Boton de conexion
    connect(ui->botonConectar, &QPushButton::clicked, this, &MainWindow::alPresionarConectar);

    // 4) Casillas de salida: cada una envia su canal (1..4)
    connect(ui->casillaSalida1, &QCheckBox::toggled, this, [this](bool estado){ enviarSalida(1, estado); });
    connect(ui->casillaSalida2, &QCheckBox::toggled, this, [this](bool estado){ enviarSalida(2, estado); });
    connect(ui->casillaSalida3, &QCheckBox::toggled, this, [this](bool estado){ enviarSalida(3, estado); });
    connect(ui->casillaSalida4, &QCheckBox::toggled, this, [this](bool estado){ enviarSalida(4, estado); });
    connect(ui->casillaRgb,     &QCheckBox::toggled, this, [this](bool estado){ enviarRgb(estado); });

    // 5) Botones de sonido (tambien accionables a mano para probar)
    connect(ui->botonReproducir,   &QPushButton::clicked, this, &MainWindow::reproducirSonido);
    connect(ui->botonPausar,       &QPushButton::clicked, this, &MainWindow::pausarSonido);
    connect(ui->botonCargarSonido, &QPushButton::clicked, this, &MainWindow::cargarSonido);

    registrar("Aplicacion lista. Escribe la IP de la ESP32 y presiona Conectar.");
}

MainWindow::~MainWindow()
{
    socket->close();
    delete ui;
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

void MainWindow::enviarSalida(int canal, bool estado)
{
    QJsonObject objeto;
    objeto["comando"] = "salida";
    objeto["canal"]   = canal;
    objeto["estado"]  = estado;
    enviarJson(objeto);
}

void MainWindow::enviarRgb(bool estado)
{
    QJsonObject objeto;
    objeto["comando"] = "rgb";
    objeto["estado"]  = estado;
    enviarJson(objeto);
}

// ----------------------------------------------------------------------------
// Recepcion e interpretacion de mensajes JSON de la ESP32
// ----------------------------------------------------------------------------
void MainWindow::alRecibirMensaje(const QString &mensaje)
{
    registrar("<- " + mensaje);
    QJsonDocument documento = QJsonDocument::fromJson(mensaje.toUtf8());
    if (!documento.isObject()) return;
    interpretarEvento(documento.object());
}

void MainWindow::interpretarEvento(const QJsonObject &objeto)
{
    QString evento = objeto.value("evento").toString();
    if (evento == "boton") {
        QString boton = objeto.value("boton").toString();
        ui->etiquetaBoton->setText("Último botón: " + boton);
        if (boton == "reproducir")   reproducirSonido();
        else if (boton == "pausar")  pausarSonido();
        // el boton "boot" solo se muestra en la etiqueta y el registro
    }
}

// ----------------------------------------------------------------------------
// Control del sonido
// ----------------------------------------------------------------------------
void MainWindow::reproducirSonido()
{
    if (reproductor->source().isEmpty()) {
        registrar("No hay sonido cargado. Usa 'Cargar sonido...'.");
        return;
    }
    reproductor->play();
    registrar("Reproduciendo sonido.");
}

void MainWindow::pausarSonido()
{
    reproductor->pause();
    registrar("Sonido en pausa.");
}

void MainWindow::cargarSonido()
{
    QString ruta = QFileDialog::getOpenFileName(this, "Selecciona un sonido",
                                                QString(), "Audio (*.wav *.mp3)");
    if (ruta.isEmpty()) return;
    reproductor->setSource(QUrl::fromLocalFile(ruta));
    registrar("Sonido cargado: " + ruta);
}

// ----------------------------------------------------------------------------
// Registro de mensajes en pantalla (con hora)
// ----------------------------------------------------------------------------
void MainWindow::registrar(const QString &linea)
{
    QString hora = QDateTime::currentDateTime().toString("HH:mm:ss");
    ui->registro->append("[" + hora + "] " + linea);
}
