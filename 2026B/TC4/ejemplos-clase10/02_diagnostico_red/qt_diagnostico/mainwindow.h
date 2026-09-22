#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>

class QLineEdit;
class QPushButton;
class QTextEdit;

// Ventana que "empareja" con el sketch 02_diagnostico_red:
// consulta GET /info y muestra los datos de red de la ESP32.
class VentanaPrincipal : public QMainWindow {
    Q_OBJECT
public:
    explicit VentanaPrincipal(QWidget *padre = nullptr);

private slots:
    void alConsultar();

private:
    QLineEdit  *campoIp;
    QPushButton *botonConsultar;
    QTextEdit  *cuadroInfo;
    QNetworkAccessManager *gestorRed;
};

#endif // MAINWINDOW_H
