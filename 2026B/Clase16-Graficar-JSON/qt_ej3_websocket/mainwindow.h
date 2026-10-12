// ============================================================================
//  Clase 16 - Ejemplo 3 - Diseño de Interfaces (I7262)
//  Declaracion de la ventana principal: cliente WebSocket + grafica (Qt Charts).
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

private:
    // --- Ayudantes propios ---
    void configurarGrafica();                          // series + ejes (una sola vez)
    void interpretarEvento(const QJsonObject &objeto); // reacciona a los mensajes de la ESP32
    void graficar(double temperatura, double humedad); // (5) agrega el punto
    void registrar(const QString &linea);              // escribe una linea en el registro

    Ui::MainWindow *ui;
    QWebSocket   *socket;        // cliente WebSocket hacia la ESP32
    bool          conectado;     // ¿hay conexion activa?

    QLineSeries  *serieTemperatura;
    QLineSeries  *serieHumedad;
    QValueAxis   *ejeX;
    QValueAxis   *ejeYTemperatura;
    QValueAxis   *ejeYHumedad;
    int           muestra;       // indice de muestra = eje X

    static constexpr int VENTANA = 40;   // puntos visibles
};
#endif // MAINWINDOW_H
