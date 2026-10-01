#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebSocket>
#include <QTimer>
#include <QUrl>
#include <QSet>
#include <QKeyEvent>
#include <QShortcut>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
protected:
    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // WebSocket
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    void onError(QAbstractSocket::SocketError error);
#else
    void onError(QAbstractSocket::SocketError error);
#endif
    void tryReconnect();

    // Keep-alive
    void sendPing();
    void onPong(quint64 elapsedTime, const QByteArray &payload);
    void onStateChanged(QAbstractSocket::SocketState s);



private:
    Ui::MainWindow *ui;

    int m_keyMoveMs = 600;     // duración por pulsación
    QSet<int> m_held;          // teclas actualmente presionadas
    void handleKeyMove(int key, bool pressed);

    QWebSocket *m_webSocket;
    QTimer m_reconnectTimer;
    QTimer m_pingTimer;
    QUrl m_url;

    int    m_backoffMs = 2000;
    const  int m_backoffMaxMs = 30000;
    int    m_consecutiveFailures = 0;
    qint64 m_lastPongMs = 0;

    void openSocket();
    void sendMover(const QString& accion, int ms = 600);
};

#endif // MAINWINDOW_H
