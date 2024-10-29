#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_webSocket = new QWebSocket;
    m_textEdit = ui->textEdit;
    m_textEdit->setReadOnly(true);  // Solo lectura para la respuesta de la ESP32

    m_input = ui->lineEdit;  // Entrada para enviar mensajes

    // Conectar automáticamente al servidor WebSocket al iniciar
    connect(m_webSocket, &QWebSocket::connected, this, &MainWindow::onConnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &MainWindow::onTextMessageReceived);
    connect(m_webSocket, &QWebSocket::disconnected, this, &MainWindow::onDisconnected);
    connect(m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::errorOccurred), this, &MainWindow::onError);

    m_webSocket->open(QUrl("ws://192.168.0.100/ws"));  // Reemplaza <ESP32_IP> con la IP de tu ESP32

    // Conectar botón de envío
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(sendMessage()));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::onConnected() {
    m_textEdit->append("Conectado al servidor WebSocket.");
    qDebug() << "WebSocket conectado.";
}

void MainWindow::onDisconnected() {
    m_textEdit->append("Desconectado del servidor WebSocket.");
    qDebug() << "WebSocket desconectado.";
}

void MainWindow::onTextMessageReceived(const QString &message) {
    m_textEdit->append("Mensaje recibido: " + message);
    qDebug() << "Mensaje recibido:" << message;
}

void MainWindow::sendMessage() {
    QString message = "{" + m_input->text() + "}";  // Añade {} alrededor del mensaje
    m_webSocket->sendTextMessage(message.toUtf8().constData());          // Envía el mensaje al WebSocket
    m_textEdit->append("Mensaje enviado: " + message);
    m_input->clear();  // Limpia el QLineEdit después de enviar
}

void MainWindow::onError(QAbstractSocket::SocketError error) {
    QString errorMsg;

    switch (error) {
    case QAbstractSocket::HostNotFoundError:
        errorMsg = "Host no encontrado. Verifica la IP.";
        break;
    case QAbstractSocket::ConnectionRefusedError:
        errorMsg = "Conexión rechazada. El servidor no responde.";
        break;
    case QAbstractSocket::RemoteHostClosedError:
        errorMsg = "El host remoto cerró la conexión.";
        break;
    default:
        errorMsg = "Error desconocido. Código: " + QString::number(error);
    }

    m_textEdit->append("Error de WebSocket: " + errorMsg);
    qDebug() << "Error de WebSocket: " << errorMsg;
}

