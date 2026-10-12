#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Ejemplo 1 - Contrato v1: {"t":24.80}
// Un valor por linea JSON -> una curva en Qt Charts.

#include <QMainWindow>
#include <QSerialPort>
#include <QtCharts>

class QComboBox;
class QPushButton;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void conectar();
    void leerDatos();

private:
    void graficar(double t);

    QSerialPort *puerto;
    QComboBox   *comboPuertos;
    QPushButton *botonConectar;
    QLabel      *etiquetaUltima;

    QLineSeries *serieT;
    QValueAxis  *ejeX;
    QValueAxis  *ejeY;
    int muestra = 0;                   // indice de muestra = eje X

    static constexpr int VENTANA = 40; // puntos visibles
};

#endif // MAINWINDOW_H
