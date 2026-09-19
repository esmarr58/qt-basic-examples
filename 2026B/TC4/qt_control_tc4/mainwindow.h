// ============================================================================
//  TC4 - Diseño de Interfaces (I7262)
//  Declaracion de la ventana principal: cliente WebSocket + reproductor de audio.
// ============================================================================
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebSocket>
#include <QAbstractSocket>
#include <QJsonObject>
#include <QMediaPlayer>
#include <QAudioOutput>

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

    // --- Control del sonido ---
    void reproducirSonido();
    void pausarSonido();
    void cargarSonido();

private:
    // --- Ayudantes propios ---
    void enviarSalida(int canal, bool estado);      // comando "salida" (canal 1..4)
    void enviarRgb(bool estado);                    // comando "rgb"
    void enviarJson(const QJsonObject &objeto);     // serializa y envia un JSON
    void interpretarEvento(const QJsonObject &objeto); // reacciona a eventos de la ESP32
    void registrar(const QString &linea);           // escribe una linea en el registro

    Ui::MainWindow *ui;
    QWebSocket   *socket;        // cliente WebSocket hacia la ESP32
    bool          conectado;     // ¿hay conexion activa?
    QMediaPlayer *reproductor;   // reproductor del sonido
    QAudioOutput *salidaAudio;   // salida de audio del reproductor
};
#endif // MAINWINDOW_H
