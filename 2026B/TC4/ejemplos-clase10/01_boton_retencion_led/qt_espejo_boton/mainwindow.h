#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>

class QLineEdit;
class QPushButton;
class QLabel;
class QTimer;

// Ventana que "empareja" con el sketch 01_boton_retencion_led:
// consulta GET /estado por la red y refleja el LED de la ESP32.
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *padre = nullptr);

private slots:
    void alConectar();
    void consultarEstado();

private:
    void mostrarEstado(bool encendido);

    QLineEdit  *campoIp;
    QPushButton *botonConectar;
    QLabel     *indicador;
    QLabel     *etiquetaAyuda;
    QTimer     *temporizador;
    QNetworkAccessManager *gestorRed;
};

#endif // MAINWINDOW_H
