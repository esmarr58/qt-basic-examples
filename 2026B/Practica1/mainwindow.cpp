#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QRandomGenerator>
#include <QDebug>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_webSocket(new QWebSocket)
{
    ui->setupUi(this);

    // Asegura que la ventana recibe eventos de teclado
    setFocusPolicy(Qt::StrongFocus);
    if (centralWidget()) centralWidget()->setFocusPolicy(Qt::StrongFocus);


    // Atajos directos (disparan sólo al presionar)
    auto mk = [&](int key, const char* accion){
        auto sc = new QShortcut(QKeySequence(key), this);
        sc->setAutoRepeat(false);
        connect(sc, &QShortcut::activated, this, [this, accion]{ sendMover(accion, m_keyMoveMs); });
    };
    mk(Qt::Key_Up,    "avanzar");
    mk(Qt::Key_Down,  "retroceder");
    mk(Qt::Key_Left,  "izquierda");
    mk(Qt::Key_Right, "derecha");

    // Parar con Space / Esc
    auto stop1 = new QShortcut(QKeySequence(Qt::Key_Space), this);
    stop1->setAutoRepeat(false);
    connect(stop1, &QShortcut::activated, this, [this]{ sendMover("parar", 0); });

    auto stop2 = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    stop2->setAutoRepeat(false);
    connect(stop2, &QShortcut::activated, this, [this]{ sendMover("parar", 0); });

    // Ajusta aquí la URL del WebSocket (IP o mDNS de la ESP32)
    // m_url = QUrl(QStringLiteral("ws://esp32-123abc.local:81"));
    m_url = QUrl(QStringLiteral("ws://esp32-5434e4.local:81"));

    // === Conexiones del WebSocket ===
    connect(m_webSocket, &QWebSocket::connected,
            this, &MainWindow::onConnected);
    connect(m_webSocket, &QWebSocket::disconnected,
            this, &MainWindow::onDisconnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived,
            this, &MainWindow::onTextMessageReceived);
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    connect(m_webSocket, &QWebSocket::errorOccurred,
            this, &MainWindow::onError);
#else
    connect(m_webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            this, &MainWindow::onError);
#endif
    connect(m_webSocket, &QWebSocket::pong, this, &MainWindow::onPong);
    connect(m_webSocket, &QWebSocket::stateChanged, this, &MainWindow::onStateChanged);

    // Timer de reconexión (backoff variable)
    m_reconnectTimer.setSingleShot(true);
    connect(&m_reconnectTimer, &QTimer::timeout,
            this, &MainWindow::tryReconnect);

    // Timer de keep-alive (ping/pong)
    m_pingTimer.setInterval(5000); // ping cada 5s
    connect(&m_pingTimer, &QTimer::timeout,
            this, &MainWindow::sendPing);

    statusBar()->showMessage(tr("Conectando..."));
    openSocket();

    // Conexiones de botones → comando "mover" con lambdas
    connect(ui->adelante, &QPushButton::clicked, this, [this]{
        sendMover("avanzar", 600);
    });
    connect(ui->atras, &QPushButton::clicked, this, [this]{
        sendMover("retroceder", 600);
    });
    connect(ui->izquierda, &QPushButton::clicked, this, [this]{
        sendMover("izquierda", 600);
    });
    connect(ui->derecha, &QPushButton::clicked, this, [this]{
        sendMover("derecha", 600);
    });

}

MainWindow::~MainWindow() {
    if (m_webSocket) {
        m_webSocket->close();
        delete m_webSocket;
    }
    delete ui;
}

void MainWindow::openSocket() {
    qDebug() << "Conectando a:" << m_url;
    statusBar()->showMessage(tr("Conectando a %1").arg(m_url.toString()));
    m_webSocket->open(m_url);
}

void MainWindow::onConnected() {
    qDebug() << "Conectado:" << m_url;
    statusBar()->showMessage(tr("Conectado"));

    // Reinicia backoff
    m_consecutiveFailures = 0;
    m_backoffMs = 2000;
    m_reconnectTimer.stop();

    // Enviar “latido” para que la ESP32 empiece a transmitir telemetría
    QJsonObject obj;
    obj["tipo"] = "latido";
    const QString json = QString::fromUtf8(
        QJsonDocument(obj).toJson(QJsonDocument::Compact));
    m_webSocket->sendTextMessage(json);

    // Inicia el keep-alive
    m_lastPongMs = QDateTime::currentMSecsSinceEpoch();
    m_pingTimer.start();
}

void MainWindow::onDisconnected() {
    qDebug() << "Desconectado del servidor WebSocket.";
    statusBar()->showMessage(tr("Desconectado"));
    m_pingTimer.stop();

    // backoff exponencial con jitter
    if (!m_reconnectTimer.isActive()) {
        m_consecutiveFailures++;
        if (m_consecutiveFailures > 1)
            m_backoffMs = qMin(m_backoffMaxMs, m_backoffMs * 2);
        int jitter = QRandomGenerator::global()->bounded(0, 501);
        int delayMs = qMin(m_backoffMaxMs, m_backoffMs + jitter);
        qDebug() << "Reintentando en" << delayMs << "ms...";
        m_reconnectTimer.start(delayMs);
    }
}

void MainWindow::onTextMessageReceived(const QString &message) {
    // Aquí puedes procesar telemetría o acks si quieres mostrar algo en UI
    qDebug() << "RX:" << message;
}

void MainWindow::onError(QAbstractSocket::SocketError) {
    const QString errStr = m_webSocket->errorString();
    qWarning() << "Error WebSocket:" << errStr;
    statusBar()->showMessage(tr("Error WebSocket: %1").arg(errStr));

    // Cierra y programa reintento con backoff
    m_webSocket->abort();
    if (!m_reconnectTimer.isActive()) {
        m_consecutiveFailures++;
        if (m_consecutiveFailures > 1)
            m_backoffMs = qMin(m_backoffMaxMs, m_backoffMs * 2);
        int jitter = QRandomGenerator::global()->bounded(0, 501);
        int delayMs = qMin(m_backoffMaxMs, m_backoffMs + jitter);
        qDebug() << "Reintentando en" << delayMs << "ms (por error)...";
        m_reconnectTimer.start(delayMs);
    }
}

void MainWindow::tryReconnect() {
    if (m_webSocket->state() == QAbstractSocket::UnconnectedState) {
        qDebug() << "Reintentando conexión...";
        openSocket();
    } else {
        qDebug() << "Forzando reinicio de socket...";
        m_webSocket->abort();
        openSocket();
    }
}

// Keep-alive
void MainWindow::sendPing() {
    if (!m_webSocket) return;

    const qint64 ahora = QDateTime::currentMSecsSinceEpoch();
    const qint64 msDesdePong = ahora - m_lastPongMs;
    const qint64 timeoutPongMs = 12000; // 12s sin PONG → reinicio

    if (msDesdePong > timeoutPongMs) {
        qWarning() << "No se recibió PONG a tiempo. Reiniciando conexión...";
        m_pingTimer.stop();
        m_webSocket->abort();
        onDisconnected();
        return;
    }

    m_webSocket->ping("keepalive");
}

void MainWindow::onPong(quint64 elapsedTime, const QByteArray &payload) {
    Q_UNUSED(payload);
    m_lastPongMs = QDateTime::currentMSecsSinceEpoch();
    qDebug() << "PONG (" << elapsedTime << "ms)";
}

void MainWindow::onStateChanged(QAbstractSocket::SocketState s) {
    qDebug() << "Estado WebSocket:" << s;
}

// -------------------- Botones de movimiento --------------------
void MainWindow::sendMover(const QString& accion, int ms) {
    if (!m_webSocket || m_webSocket->state() != QAbstractSocket::ConnectedState) {
        statusBar()->showMessage(tr("No conectado"));
        return;
    }
    QJsonObject obj;
    obj["tipo"]   = "mover";
    obj["accion"] = accion;      // "avanzar", "retroceder", "izquierda", "derecha", "parar"
    obj["ms"]     = ms;          // duración en ms (límite de seguridad lo maneja la ESP32)

    const QString json = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    m_webSocket->sendTextMessage(json);
    statusBar()->showMessage(tr("Enviado: %1").arg(json), 2000);
}

void MainWindow::handleKeyMove(int key, bool pressed) {
    // Mapea flechas y WASD
    auto sendByKey = [&](const char* accion) {
        if (pressed) {
            sendMover(accion, m_keyMoveMs);
        } else {
            sendMover("parar", 0); // detener al soltar
        }
    };

    switch (key) {
    case Qt::Key_Up:     // ↑
    case Qt::Key_W:
        sendByKey("avanzar");    break;
    case Qt::Key_Down:   // ↓
    case Qt::Key_S:
        sendByKey("retroceder"); break;
    case Qt::Key_Left:   // ←
    case Qt::Key_A:
        sendByKey("izquierda");  break;
    case Qt::Key_Right:  // →
    case Qt::Key_D:
        sendByKey("derecha");    break;

    // Extras útiles:
    case Qt::Key_Space:  // parar inmediato
    case Qt::Key_Escape:
        if (pressed) sendMover("parar", 0);
        break;
    default:
        break;
    }
}

void MainWindow::keyPressEvent(QKeyEvent *e) {
    if (e->isAutoRepeat()) { e->ignore(); return; }
    int k = e->key();

    // Modificador: con SHIFT duplica la duración
    int prev = m_keyMoveMs;
    if (e->modifiers() & Qt::ShiftModifier) m_keyMoveMs = 1200; else m_keyMoveMs = 600;

    if (!m_held.contains(k)) {
        m_held.insert(k);
        handleKeyMove(k, /*pressed=*/true);
    }
    // restaura por si el usuario suelta SHIFT antes que la flecha
    m_keyMoveMs = prev;
    QMainWindow::keyPressEvent(e);
}

void MainWindow::keyReleaseEvent(QKeyEvent *e) {
    if (e->isAutoRepeat()) { e->ignore(); return; }
    int k = e->key();
    if (m_held.contains(k)) {
        m_held.remove(k);
        handleKeyMove(k, /*pressed=*/false);
    }
    QMainWindow::keyReleaseEvent(e);
}


