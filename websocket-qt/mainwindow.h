#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebSocket>
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>
#include <QDateTime>
#include <QChartView>
#include <QLineSeries>
#include <QTimer>
#include <QList>
#include <QChart>
#include <QChartView>
#include <QValueAxis>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QTimer>



QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnected();
    void onTextMessageReceived(const QString &message);
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    void on_pushButtonSend_clicked(); // Función para el botón de enviar
    void sendHeartbeat();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
    QWebSocket *m_webSocket;   // Puntero al WebSocket
    bool m_connected;          // Estado de la conexión WebSocket

    void connectWebSocket(const QUrl &url);
    // Variables para el gráfico
    QLineSeries *series;            // Serie de datos para la gráfica
    QChartView *chartView;          // Vista del gráfico
    QList<QPointF> dataPoints;      // Almacena los puntos del gráfico (tiempo y valor)
    QValueAxis *axisX;              // Eje X del gráfico
    QValueAxis *axisY;              // Eje Y del gráfico

    QTimer *updateTimer;            // Timer para controlar la actualización del gráfico
    qint64 startTime;               // Tiempo inicial para manejar la gráfica en tiempo relativo
};
#endif // MAINWINDOW_H
