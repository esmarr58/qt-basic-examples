#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// Clase 17 - Lotes con marca de tiempo epoch
// Contrato: {"seq":7,"t0":1792069200123,"dt":60,"sinc":true,"dist":[37.4,37.9,null,38.6]}
// Cada muestra i se grafica en x = t0 + i*dt (ms desde 1970-01-01 00:00:00 UTC).

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
    void sincronizarHora();
    void leerDatos();

private:
    void procesarLote(const QJsonObject &lote);
    void actualizarEstado();

    QSerialPort *puerto;
    QComboBox   *comboPuertos;
    QPushButton *botonConectar;
    QPushButton *botonSincronizar;
    QLabel      *etiquetaTiempo;
    QLabel      *etiquetaEstado;

    QLineSeries   *serieDist;
    QDateTimeAxis *ejeTiempo;
    QValueAxis    *ejeDist;

    qint64 ultimoSeq = -1;
    int lotesRecibidos = 0;
    int lotesPerdidos = 0;

    static constexpr qint64 VENTANA_MS = 10000;   // 10 s visibles
};

#endif // MAINWINDOW_H
