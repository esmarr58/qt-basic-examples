#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_webSocket(new QWebSocket)
    , m_connected(false)
    , series(new QLineSeries())
    , chartView(new QChartView())
    , axisX(new QValueAxis())   // Inicializar axisX aquí
    , axisY(new QValueAxis())   // Inicializar axisY también
    , startTime(0)  // Inicializar startTime
{
    ui->setupUi(this);

    // Configurar el gráfico
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Datos en tiempo real del último minuto");

    // Configurar los ejes
    axisX->setTickCount(6);
    qint64 currentTime = QDateTime::currentSecsSinceEpoch();
    startTime = currentTime;  // Inicializar startTime aquí si aún no se ha hecho
    axisX->setRange(0, 60);  // Mostrar últimos 60 segundos en el eje X

    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    axisY->setLabelFormat("%.2f");
    axisY->setTitleText("Valor ADC (V)");
    axisY->setRange(-0.2,4);  // Rango de 0 a 3.3V para el eje Y
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    // Asignar el gráfico a chartView
    chartView->setChart(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setParent(ui->widget);
    chartView->setGeometry(0, 0, ui->widget->width(), ui->widget->height());

    // Conectar WebSocket
    connect(m_webSocket, &QWebSocket::connected, this, &MainWindow::onConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &MainWindow::onDisconnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &MainWindow::onTextMessageReceived);
    connect(m_webSocket, &QWebSocket::errorOccurred, this, &MainWindow::onError);

    // Conectar botón de envío
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(on_pushButtonSend_clicked()));

    // Enviar un heartbeat cada 2 segundos
    QTimer *cronos = new QTimer(this);
    connect(cronos, SIGNAL(timeout()), this, SLOT(sendHeartbeat()));
    cronos->start(2000);
}


MainWindow::~MainWindow()
{
    delete ui;
    delete m_webSocket;
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
    QString ip = ui->lineEdit->text();
    if (!ip.isEmpty()) {
        QUrl url(QString("ws://") + ip + "/ws");
        if (!m_connected) {
            connectWebSocket(url);
        }
    } else {
        ui->textEdit->append("Error: Debes ingresar la IP y el mensaje.");
    }
}

void MainWindow::on_pushButton_2_clicked() {
    if (m_connected) {
        m_webSocket->sendTextMessage(ui->lineEdit_2->text().toUtf8().constData());
    } else {
        ui->textEdit->append("Error: WebSocket no está conectado.");
    }
}
