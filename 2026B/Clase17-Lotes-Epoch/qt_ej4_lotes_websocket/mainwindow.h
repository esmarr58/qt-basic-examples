// ============================================================================
//  Clase 17 - Ejemplo 4 - Diseño de Interfaces (I7262)
//  Declaracion de la ventana principal: cliente WebSocket + grafica con eje de
//  fecha y hora (QDateTimeAxis) para lotes con marca de tiempo epoch.
// ============================================================================
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebSocket>
#include <QAbstractSocket>
#include <QJsonObject>
#include <QtCharts>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // --- Acciones de la interfaz y del socket (iguales que en el TC4) ---
    void alPresionarConectar();                                // boton Conectar/Desconectar
    void alConectar();                                         // el socket se conecto
    void alDesconectar();                                      // el socket se desconecto
    void alRecibirMensaje(const QString &mensaje);             // llego un mensaje JSON
    void alOcurrirError(QAbstractSocket::SocketError error);   // error del socket
    void alPresionarSincronizar();                             // manda la hora real

private:
    // --- Ayudantes propios ---
    void configurarGrafica();                          // serie + ejes (una sola vez)
    void enviarJson(const QJsonObject &objeto);        // serializa y envia un JSON
    void interpretarEvento(const QJsonObject &objeto); // reacciona a los mensajes de la ESP32
    void procesarLote(const QJsonObject &objeto);      // (4b) reconstruye el tiempo y (5) grafica
    void registrar(const QString &linea);              // escribe una linea en el registro

    Ui::MainWindow *ui;
    QWebSocket    *socket;        // cliente WebSocket hacia la ESP32
    bool           conectado;     // ¿hay conexion activa?

    QLineSeries   *serieDistancia;
    QDateTimeAxis *ejeTiempo;
    QValueAxis    *ejeDistancia;

    qint64 ultimoSeq;             // -1 = todavia no llega ningun lote
    int    lotesRecibidos;
    int    lotesPerdidos;

    static constexpr qint64 VENTANA_MS = 10000;   // 10 s visibles
};
#endif // MAINWINDOW_H
