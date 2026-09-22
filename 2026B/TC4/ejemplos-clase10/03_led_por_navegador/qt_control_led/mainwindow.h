#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebSocket>

class QLineEdit;
class QPushButton;
class QLabel;

// Empareja con el sketch 03_led_por_navegador por WebSocket (ws://<IP>:81):
// envia "ENCENDER"/"APAGAR" y refleja el estado del LED.
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void alConectar();
    void alConectarSocket();
    void encender();
    void apagar();
    void alRecibirMensaje(const QString &mensaje);

private:
    void enviarLed(bool encendido);

    QLineEdit  *campoIp;
    QPushButton *botonConectar;
    QPushButton *botonEncender;
    QPushButton *botonApagar;
    QLabel     *etiquetaEstado;
    QWebSocket  socket;
};

#endif // MAINWINDOW_H
