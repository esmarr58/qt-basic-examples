#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Ejemplo 2 - Contrato v2: {"t":24.80,"h":55.30}
// Dos valores por linea JSON -> dos curvas, cada una con SU eje Y.
// [v2] marca lo que cambia respecto al ejemplo 1.

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
    void graficar(double t, double h);          // [v2]

    QSerialPort *puerto;
    QComboBox   *comboPuertos;
    QPushButton *botonConectar;
    QLabel      *etiquetaUltima;

    QLineSeries *serieT;
    QValueAxis  *ejeX;
    QValueAxis  *ejeY;
    QLineSeries *serieH;                        // [v2]
    QValueAxis  *ejeYH;                         // [v2] eje de humedad
    int muestra = 0;                   // indice de muestra = eje X

    static constexpr int VENTANA = 40; // puntos visibles
};

#endif // MAINWINDOW_H
