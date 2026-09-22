#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>

class QLineEdit;
class QPushButton;
class QLabel;

// Ventana que "empareja" con el sketch 03_led_por_navegador:
// envia GET /encender o /apagar para controlar el LED de la ESP32.
class VentanaPrincipal : public QMainWindow {
    Q_OBJECT
public:
    explicit VentanaPrincipal(QWidget *padre = nullptr);

private slots:
    void encender();
    void apagar();

private:
    void enviarComando(const QString &ruta, const QString &mensajeOk);

    QLineEdit  *campoIp;
    QPushButton *botonEncender;
    QPushButton *botonApagar;
    QLabel     *etiquetaEstado;
    QNetworkAccessManager *gestorRed;
};

#endif // MAINWINDOW_H
