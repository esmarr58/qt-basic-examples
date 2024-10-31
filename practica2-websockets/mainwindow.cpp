#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFocus();  // Establecer el foco en la ventana principal

    startTime = QDateTime::currentSecsSinceEpoch();

    // Configuración para gráfica de temperatura
    chartTemp = new QChart();
    seriesTemp = new QLineSeries();
    chartTemp->addSeries(seriesTemp);
    chartTemp->setTitle("Temperatura");

    axisXTemp = new QValueAxis();
    axisXTemp->setTitleText("Tiempo (s)");
    axisXTemp->setRange(0, 40);
    chartTemp->addAxis(axisXTemp, Qt::AlignBottom);
    seriesTemp->attachAxis(axisXTemp);

    // Inicializar el eje Y para temperatura y configurarlo
    axisYTemp = new QValueAxis();
    axisYTemp->setTitleText("Temperatura (°C)");
    axisYTemp->setRange(0, 50);  // Ajusta según el rango esperado de temperatura
    chartTemp->addAxis(axisYTemp, Qt::AlignLeft);
    seriesTemp->attachAxis(axisYTemp);

    ui->widget->setChart(chartTemp);

    // Configuración para gráfica de voltaje
    chartVolt = new QChart();
    seriesVolt = new QLineSeries();
    chartVolt->addSeries(seriesVolt);
    chartVolt->setTitle("Voltaje");

    axisXVolt = new QValueAxis();
    axisXVolt->setTitleText("Tiempo (s)");
    axisXVolt->setRange(0, 40);
    chartVolt->addAxis(axisXVolt, Qt::AlignBottom);
    seriesVolt->attachAxis(axisXVolt);

    // Inicializar el eje Y para voltaje y configurarlo
    axisYVolt = new QValueAxis();
    axisYVolt->setTitleText("Voltaje (V)");
    axisYVolt->setRange(0, 5);  // Ajusta según el rango esperado de voltaje
    chartVolt->addAxis(axisYVolt, Qt::AlignLeft);
    seriesVolt->attachAxis(axisYVolt);

    ui->widget_2->setChart(chartVolt);

    // Configuración para gráfica de humedad
    chartHum = new QChart();
    seriesHum = new QLineSeries();
    chartHum->addSeries(seriesHum);
    chartHum->setTitle("Humedad");

    axisXHum = new QValueAxis();
    axisXHum->setTitleText("Tiempo (s)");
    axisXHum->setRange(0, 40);
    chartHum->addAxis(axisXHum, Qt::AlignBottom);
    seriesHum->attachAxis(axisXHum);

    // Inicializar el eje Y para humedad y configurarlo
    axisYHum = new QValueAxis();
    axisYHum->setTitleText("Humedad (%)");
    axisYHum->setRange(0, 100);  // Ajusta según el rango esperado de humedad
    chartHum->addAxis(axisYHum, Qt::AlignLeft);
    seriesHum->attachAxis(axisYHum);

    ui->widget_3->setChart(chartHum);



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
    activarConexion();

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

void MainWindow::activarConexion() {

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
    qDebug() << "Mensaje recibido:" << message;  // Muestra el mensaje JSON completo recibido

    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isNull() && doc.isObject()) {
        QJsonObject jsonObj = doc.object();

        // Si el mensaje no contiene un timestamp, lo asigna desde Qt
        qint64 timestamp;
        if (jsonObj.contains("timestamp")) {
            timestamp = jsonObj["timestamp"].toVariant().toLongLong();
            qDebug() << "Timestamp recibido:" << timestamp;
        } else {
            timestamp = QDateTime::currentSecsSinceEpoch();  // Asigna el timestamp actual
            qDebug() << "El mensaje JSON no contiene un campo de 'timestamp', se usa timestamp actual:" << timestamp;
        }

        // Procesar temperatura
        if (jsonObj.contains("temperature")) {
            double tempValue = jsonObj["temperature"].toDouble();
            qDebug() << "Temperatura recibida:" << tempValue;
            updateTemperature(tempValue, timestamp);
            ui->lcdNumber->display(tempValue);
        } else {
            qDebug() << "No se recibió campo de 'temperature' en el mensaje JSON.";
        }

        // Procesar voltaje
        if (jsonObj.contains("adc_value")) {
            double voltValue = jsonObj["adc_value"].toDouble();
            qDebug() << "Voltaje recibido:" << voltValue;
            updateVoltage(voltValue, timestamp);
            ui->lcdNumber_2->display(voltValue);
        } else {
            qDebug() << "No se recibió campo de 'voltage' en el mensaje JSON.";
        }

        // Procesar humedad
        if (jsonObj.contains("humidity")) {
            double humValue = jsonObj["humidity"].toDouble();
            qDebug() << "Humedad recibida:" << humValue;
            updateHumidity(humValue, timestamp);
            ui->lcdNumber_3->display(humValue);
        } else {
            qDebug() << "No se recibió campo de 'humidity' en el mensaje JSON.";
        }
    } else {
        qDebug() << "Error: mensaje JSON no válido.";
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


void MainWindow::updateTemperature(double temp, qint64 timestamp) {
    qint64 currentTime = timestamp - startTime;
    dataPointsTemp.append(QPointF(currentTime, temp));
    while (!dataPointsTemp.isEmpty() && currentTime - dataPointsTemp.first().x() > 60) {
        dataPointsTemp.removeFirst();
    }
    seriesTemp->replace(dataPointsTemp);
    axisXTemp->setRange(currentTime - 60, currentTime);
    axisYTemp->setRange(0, 50);


}

void MainWindow::updateVoltage(double voltage, qint64 timestamp) {
    qint64 currentTime = timestamp - startTime;
    dataPointsVolt.append(QPointF(currentTime, voltage));
    while (!dataPointsVolt.isEmpty() && currentTime - dataPointsVolt.first().x() > 60) {
        dataPointsVolt.removeFirst();
    }
    seriesVolt->replace(dataPointsVolt);
    axisXVolt->setRange(currentTime - 60, currentTime);
    axisYVolt->setRange(0, 4);


}

void MainWindow::updateHumidity(double humidity, qint64 timestamp) {
    qint64 currentTime = timestamp - startTime;
    dataPointsHum.append(QPointF(currentTime, humidity));
    while (!dataPointsHum.isEmpty() && currentTime - dataPointsHum.first().x() > 60) {
        dataPointsHum.removeFirst();
    }
    seriesHum->replace(dataPointsHum);
    axisXHum->setRange(currentTime - 60, currentTime);
    axisYHum->setRange(0, 100);


}
