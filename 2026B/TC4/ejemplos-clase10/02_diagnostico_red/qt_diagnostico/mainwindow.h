#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebSocket>

class QLineEdit;
class QPushButton;
class QTextEdit;

// Empareja con el sketch 02_diagnostico_red por WebSocket (ws://<IP>:81):
// manda "info" y muestra los datos de red de la ESP32.
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *padre = nullptr);

private slots:
    void alConsultar();
    void alConectarSocket();
    void alRecibirMensaje(const QString &mensaje);

private:
    QLineEdit  *campoIp;
    QPushButton *botonConsultar;
    QTextEdit  *cuadroInfo;
    QWebSocket  socket;
};

#endif // MAINWINDOW_H
