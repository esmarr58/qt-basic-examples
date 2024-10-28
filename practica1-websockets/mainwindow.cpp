#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFocus();  // Establecer el foco en la ventana principal



    m_webSocket = new QWebSocket;
    m_connected = false;
    // Conectar WebSocket
    connect(m_webSocket, &QWebSocket::connected, this, &MainWindow::onConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &MainWindow::onDisconnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &MainWindow::onTextMessageReceived);
    connect(m_webSocket, &QWebSocket::errorOccurred, this, &MainWindow::onError);

    // Conectar botón de envío
    //connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(on_pushButtonSend_clicked()));
    //Conexion automatica
    on_pushButtonSend_clicked();

    // Enviar un heartbeat cada 2 segundos
    QTimer *cronos = new QTimer(this);
    connect(cronos, SIGNAL(timeout()), this, SLOT(sendHeartbeat()));
    cronos->start(2000);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::onDisconnected() {
    qDebug() << "WebSocket disconnected!";
    ui->textEdit->append("Desconectado del WebSocket");
    m_connected = false;  // Actualizar el estado de la conexión
}

void MainWindow::onError(QAbstractSocket::SocketError errores) {
    QString errorMsg;
    switch (errores) {
    case QAbstractSocket::HostNotFoundError:
        errorMsg = "Host no encontrado.";
        break;
    case QAbstractSocket::ConnectionRefusedError:
        errorMsg = "Conexión rechazada.";
        break;
    case QAbstractSocket::RemoteHostClosedError:
        errorMsg = "El host remoto cerró la conexión.";
        break;
    default:
        errorMsg = "Error desconocido.";
        break;
    }
    qDebug() << "Error de WebSocket: " << errorMsg;
    ui->textEdit->append("Error de WebSocket: " + errorMsg);
}

void MainWindow::connectWebSocket(const QUrl &url) {
    if (!m_connected) {
        m_webSocket->open(url);
        qDebug() << "Conectando a WebSocket en" << url;
        ui->textEdit->append("Conectando a WebSocket en: " + url.toString());
    } else {
        qDebug() << "WebSocket ya está conectado";
        ui->textEdit->append("WebSocket ya está conectado");
    }
}

void MainWindow::on_pushButtonSend_clicked() {

    if (!esp32IP.isEmpty()) {
        QUrl url(QString("ws://") + esp32IP + "/ws");
        if (!m_connected) {
            connectWebSocket(url);
        }
    } else {
        ui->textEdit->append("Error: Debes ingresar la IP y el mensaje.");
    }
}

void MainWindow::sendHeartbeat() {
    QJsonObject heartbeatMessage;
    heartbeatMessage["type"] = "heartbeat";
    heartbeatMessage["timestamp"] = QDateTime::currentSecsSinceEpoch();  // Unix timestamp

    QJsonDocument doc(heartbeatMessage);
    QString jsonString = doc.toJson(QJsonDocument::Compact);

    if (m_connected) {
        m_webSocket->sendTextMessage(jsonString);
        qDebug() << "Enviando heartbeat: " << jsonString;
    }
}

void MainWindow::onConnected() {
    qDebug() << "WebSocket connected!";
    ui->textEdit->append("Conectado a WebSocket");
    m_connected = true;  // Actualizar el estado de la conexión
}


void MainWindow::onTextMessageReceived(const QString &message) {
    qDebug() << "Mensaje recibido:" << message;
    ui->textEdit->append("Mensaje recibido: " + message);

    // Procesar el mensaje JSON
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isNull() && doc.isObject()) {
        QJsonObject jsonObj = doc.object();

        if (jsonObj.contains("type") && jsonObj["type"] == "adc_reading") {
            double adcValue = jsonObj["adc_value"].toDouble();
            qint64 currentTime = QDateTime::currentSecsSinceEpoch();

            if (dataPoints.isEmpty()) {
                startTime = currentTime;  // Guarda el primer tiempo
            }

            // Ajustar el tiempo relativo al inicio
            dataPoints.append(QPointF(currentTime - startTime, adcValue));

            // Eliminar puntos de más de 60 segundos
            while (!dataPoints.isEmpty() && currentTime - startTime - dataPoints.first().x() > 60) {
                dataPoints.removeFirst();
            }

            // Actualizar el rango del eje X
            axisX->setRange(currentTime - startTime - 60, currentTime - startTime);

            // Actualizar la serie
            series->clear();
            for (const QPointF &point : dataPoints) {
                series->append(point);
                qDebug() << "Agregando punto: " << point;
            }

            chartView->update();  // Redibuja el gráfico después de agregar los puntos
        }
    }
}





void MainWindow::on_pushButton_clicked()
{
    enviarComando(11);
}

void MainWindow::enviarComando(int comando) {
    // Crear el objeto JSON
    QJsonObject jsonObject;
    jsonObject["type"] = "comando";  // Tipo de mensaje
    jsonObject["comando"] = comando;  // Enviar el comando como entero

    // Convertir el objeto JSON a una cadena
    QJsonDocument jsonDoc(jsonObject);
    QString jsonString = QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));

    // Enviar el mensaje a través del WebSocket
    m_webSocket->sendTextMessage(jsonString);
}


void MainWindow::on_pushButton_2_clicked()
{
    enviarComando(12);
}


void MainWindow::on_pushButton_3_clicked()
{
    enviarComando(13);
}

void MainWindow::enviarComandoConVelocidad(int velocidad) {
    // Crear el objeto JSON
    QJsonObject jsonObject;
    jsonObject["type"] = "comando";  // Tipo de mensaje
    jsonObject["comando"] = 5;       // Comando 5 indica que se envía la velocidad
    jsonObject["velocidad"] = velocidad;  // Añadir el valor de velocidad

    // Convertir el objeto JSON a una cadena
    QJsonDocument jsonDoc(jsonObject);
    QString jsonString = QString::fromUtf8(jsonDoc.toJson(QJsonDocument::Compact));

    // Enviar el mensaje a través del WebSocket
    m_webSocket->sendTextMessage(jsonString);
}


void MainWindow::on_pushButton_4_clicked()
{
  enviarComando(1);
}


void MainWindow::on_pushButton_5_clicked()
{
    enviarComando(2);
}


void MainWindow::on_pushButton_6_clicked()
{
    enviarComando(3);
}


void MainWindow::on_pushButton_7_clicked()
{
    enviarComando(4);
}


void MainWindow::on_verticalScrollBar_sliderMoved(int position)
{
    enviarComandoConVelocidad(position);
}


void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch(event->key()) {
    case Qt::Key_Up:
        enviarComando(1);  // Comando para avanzar
        break;
    case Qt::Key_Down:
        enviarComando(2);  // Comando para retroceder
        break;
    case Qt::Key_Left:
        enviarComando(4);  // Comando para girar a la izquierda
        break;
    case Qt::Key_Right:
        enviarComando(3);  // Comando para girar a la derecha
        break;
    default:
        QMainWindow::keyPressEvent(event);
    }
}
