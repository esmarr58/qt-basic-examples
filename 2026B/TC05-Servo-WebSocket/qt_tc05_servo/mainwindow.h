// ============================================================================
//  TC05 - Diseño de Interfaces (I7262)
//  Declaracion de la ventana principal: cliente WebSocket que manda el angulo
//  de un servomotor SG90 a la ESP32-S3 y muestra el angulo que ella confirma.
// ============================================================================
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebSocket>
#include <QAbstractSocket>
#include <QJsonObject>
#include <QTimer>

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
    // --- Acciones de la interfaz y del socket ---
    void alPresionarConectar();                                // boton Conectar/Desconectar
    void alConectar();                                         // el socket se conecto
    void alDesconectar();                                      // el socket se desconecto
    void alRecibirMensaje(const QString &mensaje);             // llego un mensaje JSON
    void alOcurrirError(QAbstractSocket::SocketError error);   // error del socket

    // --- Control del servo ---
    void alCambiarDeslizador(int valor);    // el usuario mueve el deslizador
    void alCambiarCaja(int valor);          // el usuario escribe en la caja numerica
    void alSoltarDeslizador();              // el usuario suelta el deslizador
    void alVencerTemporizador();            // pasaron 50 ms: ¿hay angulo pendiente?
    void alPresionarBarrido();              // boton "Barrido (demo)"

private:
    // --- Ayudantes propios ---
    void solicitarAngulo(int angulo);               // angulo pedido por la interfaz (con limite de envio)
    void enviarAngulo(int angulo);                  // comando "servo"
    void enviarJson(const QJsonObject &objeto);     // serializa y envia un JSON
    void interpretarEvento(const QJsonObject &objeto); // reacciona a mensajes de la ESP32
    void mostrarConfirmado(int angulo);             // angulo confirmado por la ESP32
    void sincronizarControles(int angulo);          // mueve deslizador y caja SIN enviar
    void habilitarControles(bool habilitar);        // controles activos solo con conexion
    void registrar(const QString &linea);           // escribe una linea en el registro

    Ui::MainWindow *ui;
    QWebSocket *socket;              // cliente WebSocket hacia la ESP32
    bool        conectado;           // ¿hay conexion activa?

    // Limite de envio (throttle): como maximo un mensaje cada 50 ms
    QTimer     *temporizadorEnvio;   // temporizador de un solo disparo (50 ms)
    int         anguloPedido;        // ultimo angulo que pidio el usuario
    int         ultimoEnviado;       // ultimo angulo que realmente se envio
};
#endif // MAINWINDOW_H
