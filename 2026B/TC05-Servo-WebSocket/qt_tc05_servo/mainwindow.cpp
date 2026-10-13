// ============================================================================
//  TC05 - Diseño de Interfaces (I7262)
//  Ventana principal: se conecta a la ESP32 por WebSocket y manda el angulo de
//  un servomotor SG90 con el comando JSON {"tipo":"servo","angulo":n}.
//
//  Dos angulos DISTINTOS en pantalla:
//   - Angulo PEDIDO:     lo que el usuario eligio en el deslizador / caja.
//   - Angulo CONFIRMADO: lo que la ESP32 dice que aplico ("servo_ack").
//  Si la conexion falla o la ESP32 recorta el valor, ambos no coinciden: la
//  interfaz muestra la VERDAD del dispositivo, no solo lo que se pidio.
// ============================================================================
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QSignalBlocker>
#include <QJsonDocument>
#include <QDateTime>
#include <QUrl>

// Intervalo minimo entre dos mensajes "servo" (limite de envio / throttle)
static const int intervaloEnvioMs = 50;

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
    conectado     = false;
    anguloPedido  = 90;
    ultimoEnviado = -1;     // -1 = todavia no se ha enviado nada

    // 1) Cliente WebSocket y sus señales
    socket = new QWebSocket();
    connect(socket, &QWebSocket::connected,           this, &MainWindow::alConectar);
    connect(socket, &QWebSocket::disconnected,        this, &MainWindow::alDesconectar);
    connect(socket, &QWebSocket::textMessageReceived, this, &MainWindow::alRecibirMensaje);
    connect(socket, &QWebSocket::errorOccurred,       this, &MainWindow::alOcurrirError);

    // 2) Temporizador del limite de envio.
    //    ¿Por que? Al arrastrar el deslizador, valueChanged se emite por CADA
    //    grado (hasta cientos de veces por segundo). Mandar todos esos mensajes
    //    satura la red y la cola de la ESP32, y el servo (que solo se actualiza
    //    cada 20 ms) no puede seguirlos. Por eso enviamos como maximo UN mensaje
    //    cada 50 ms: el primero sale de inmediato y los siguientes esperan al
    //    temporizador, que manda solo el ULTIMO valor pedido.
    temporizadorEnvio = new QTimer(this);
    temporizadorEnvio->setSingleShot(true);
    temporizadorEnvio->setInterval(intervaloEnvioMs);
    connect(temporizadorEnvio, &QTimer::timeout, this, &MainWindow::alVencerTemporizador);

    // 3) Boton de conexion
    connect(ui->botonConectar, &QPushButton::clicked, this, &MainWindow::alPresionarConectar);

    // 4) Deslizador y caja numerica (0..180), sincronizados entre si
    connect(ui->deslizadorAngulo, &QSlider::valueChanged,  this, &MainWindow::alCambiarDeslizador);
    connect(ui->deslizadorAngulo, &QSlider::sliderReleased, this, &MainWindow::alSoltarDeslizador);
    connect(ui->cajaAngulo, &QSpinBox::valueChanged,       this, &MainWindow::alCambiarCaja);

    // 5) Botones de posiciones fijas: mueven el deslizador (y este envia)
    connect(ui->boton0,   &QPushButton::clicked, this, [this](){ ui->deslizadorAngulo->setValue(0);   });
    connect(ui->boton90,  &QPushButton::clicked, this, [this](){ ui->deslizadorAngulo->setValue(90);  });
    connect(ui->boton180, &QPushButton::clicked, this, [this](){ ui->deslizadorAngulo->setValue(180); });
    connect(ui->botonBarrido, &QPushButton::clicked, this, &MainWindow::alPresionarBarrido);

    // Sin conexion no se puede mover el servo
    habilitarControles(false);
    registrar("Aplicacion lista. Escribe la IP de la ESP32 y presiona Conectar.");
}

MainWindow::~MainWindow()
{
    socket->close();
    delete socket;
    delete ui;
}

// ----------------------------------------------------------------------------
// Boton Conectar / Desconectar
// ----------------------------------------------------------------------------
void MainWindow::alPresionarConectar()
{
    // Si ya estamos conectados (o intentando), el boton desconecta / cancela
    if (socket->state() != QAbstractSocket::UnconnectedState) {
        socket->abort();
        if (!conectado) {                     // se cancelo un intento de conexion
            ui->etiquetaEstado->setText("Desconectado");
            ui->botonConectar->setText("Conectar");
            registrar("Intento de conexion cancelado.");
        }
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
    ui->etiquetaEstado->setText("Conectando...");
    ui->botonConectar->setText("Cancelar");
    socket->open(direccion);
}

void MainWindow::alConectar()
{
    conectado = true;
    ultimoEnviado = -1;
    ui->etiquetaEstado->setText("Conectado");
    ui->botonConectar->setText("Desconectar");
    habilitarControles(true);
    registrar("Conectado a la ESP32.");
}

void MainWindow::alDesconectar()
{
    conectado = false;
    temporizadorEnvio->stop();
    ui->etiquetaEstado->setText("Desconectado");
    ui->botonConectar->setText("Conectar");
    ui->etiquetaConfirmado->setText("--°");   // ya no sabemos donde esta el servo
    habilitarControles(false);
    registrar("Desconectado.");
}

void MainWindow::alOcurrirError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    registrar("Error de conexion: " + socket->errorString());
    // Si el error ocurrio al intentar conectar, no llega "disconnected":
    // regresamos la interfaz al estado inicial a mano.
    if (!conectado) {
        ui->etiquetaEstado->setText("Desconectado");
        ui->botonConectar->setText("Conectar");
    }
}

// ----------------------------------------------------------------------------
// Controles activos solo cuando hay conexion
// ----------------------------------------------------------------------------
void MainWindow::habilitarControles(bool habilitar)
{
    ui->grupoServo->setEnabled(habilitar);
    ui->lineEditIp->setEnabled(!habilitar);       // no cambiar la IP en plena conexion
    ui->lineEditPuerto->setEnabled(!habilitar);
}

// ----------------------------------------------------------------------------
// Deslizador y caja numerica: cada uno actualiza al otro SIN generar un ciclo
// (QSignalBlocker evita que el cambio programatico vuelva a emitir la señal).
// ----------------------------------------------------------------------------
void MainWindow::alCambiarDeslizador(int valor)
{
    {
        QSignalBlocker bloqueo(ui->cajaAngulo);
        ui->cajaAngulo->setValue(valor);
    }
    solicitarAngulo(valor);
}

void MainWindow::alCambiarCaja(int valor)
{
    {
        QSignalBlocker bloqueo(ui->deslizadorAngulo);
        ui->deslizadorAngulo->setValue(valor);
    }
    solicitarAngulo(valor);
}

// Al soltar el deslizador mandamos YA el valor final (sin esperar los 50 ms)
void MainWindow::alSoltarDeslizador()
{
    if (anguloPedido != ultimoEnviado) {
        temporizadorEnvio->stop();
        enviarAngulo(anguloPedido);
        temporizadorEnvio->start();
    }
}

// ----------------------------------------------------------------------------
// Limite de envio (throttle)
//   - Si el temporizador NO corre: enviamos de inmediato y lo arrancamos.
//   - Si SI corre: solo guardamos el angulo; al vencer, se envia el ultimo.
// ----------------------------------------------------------------------------
void MainWindow::solicitarAngulo(int angulo)
{
    anguloPedido = angulo;
    ui->etiquetaPedido->setText(QString("Pedido: %1°").arg(angulo));
    if (!temporizadorEnvio->isActive()) {
        enviarAngulo(angulo);
        temporizadorEnvio->start();
    }
}

void MainWindow::alVencerTemporizador()
{
    if (anguloPedido != ultimoEnviado) {     // quedo un valor sin enviar
        enviarAngulo(anguloPedido);
        temporizadorEnvio->start();          // y abrimos otra ventana de 50 ms
    }
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

void MainWindow::enviarAngulo(int angulo)
{
    QJsonObject objeto;
    objeto["tipo"]   = "servo";
    objeto["angulo"] = angulo;
    enviarJson(objeto);
    ultimoEnviado = angulo;
}

void MainWindow::alPresionarBarrido()
{
    QJsonObject objeto;
    objeto["tipo"] = "barrido";
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
    QString tipo = objeto.value("tipo").toString();
    if (tipo == "saludo") {
        // Al conectar, la ESP32 nos dice donde esta el servo: alineamos la
        // interfaz con el dispositivo (sin mandar un comando de regreso).
        int angulo = objeto.value("angulo").toInt();
        mostrarConfirmado(angulo);
        sincronizarControles(angulo);
    }
    else if (tipo == "servo_ack") {
        int angulo = objeto.value("angulo").toInt();
        mostrarConfirmado(angulo);
        // Si el usuario NO esta moviendo los controles (por ejemplo, durante el
        // barrido o si otra computadora movio el servo), los alineamos.
        if (!ui->deslizadorAngulo->isSliderDown() && !temporizadorEnvio->isActive()) {
            sincronizarControles(angulo);
        }
    }
    else if (tipo == "error") {
        registrar("La ESP32 reporta un error: " + objeto.value("mensaje").toString());
    }
}

// Angulo REAL que aplico la ESP32 (etiqueta grande)
void MainWindow::mostrarConfirmado(int angulo)
{
    ui->etiquetaConfirmado->setText(QString("%1°").arg(angulo));
}

// Mueve deslizador y caja a un angulo SIN disparar un nuevo envio
void MainWindow::sincronizarControles(int angulo)
{
    QSignalBlocker bloqueoDeslizador(ui->deslizadorAngulo);
    QSignalBlocker bloqueoCaja(ui->cajaAngulo);
    ui->deslizadorAngulo->setValue(angulo);
    ui->cajaAngulo->setValue(angulo);
    anguloPedido  = angulo;
    ultimoEnviado = angulo;
    ui->etiquetaPedido->setText(QString("Pedido: %1°").arg(angulo));
}

// ----------------------------------------------------------------------------
// Registro de mensajes en pantalla (con hora)
// ----------------------------------------------------------------------------
void MainWindow::registrar(const QString &linea)
{
    QString hora = QDateTime::currentDateTime().toString("HH:mm:ss.zzz");
    ui->registro->appendPlainText("[" + hora + "] " + linea);
}
