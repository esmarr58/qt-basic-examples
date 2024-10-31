#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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
#include <QLayout>


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

public slots:
    void enviarComando(int comando);
private slots:
    void onConnected();
    void onTextMessageReceived(const QString &message);
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    void activarConexion(); // Función para el botón de enviar
    void sendHeartbeat();
    void connectWebSocket(const QUrl &url);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void enviarComandoConVelocidad(int velocidad);
    void on_pushButton_4_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();


    void keyPressEvent(QKeyEvent *event);
    void updateHumidity(double humidity, qint64 timestamp);
    void updateVoltage(double voltage, qint64 timestamp);
    void updateTemperature(double temp, qint64 timestamp);
private:
    Ui::MainWindow *ui;
    QWebSocket *m_webSocket;   // Puntero al WebSocket
    bool m_connected;          // Estado de la conexión WebSocket

    QString esp32IP = "192.168.100.63";
       // Tiempo inicial para manejar la gráfica en tiempo relativo

    QChart *chartTemp, *chartVolt, *chartHum;
    QChartView *chartViewTemp, *chartViewVolt, *chartViewHum;
    QLineSeries *seriesTemp, *seriesVolt, *seriesHum;
    QValueAxis *axisXTemp, *axisXVolt, *axisXHum;
    QValueAxis *axisYTemp, *axisYVolt, *axisYHum;

    QList<QPointF> dataPointsTemp, dataPointsVolt, dataPointsHum;
    qint64 startTime;
};
#endif // MAINWINDOW_H
