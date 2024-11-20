#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QChartView>
#include <QLineSeries>
#include <QChart>
#include <QRandomGenerator>
#include <QValueAxis>
#include <QTimer>

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>
#include <QMessageBox>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>



QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void actualizarGrafico(bool prueba = true, double temperatura = 25.0, double humedad = 50.0);
    void loop();
    void manejarPuertosSeriales(QComboBox *comboBox);
    void setupDatabase();
    void saveSensorData(float temperatura, float humedad);
private:
    Ui::MainWindow *ui;
    QChart *grafico;
    QChartView *vistaGrafico;

    QLineSeries *serieTemperatura;  // Serie para la temperatura
    QLineSeries *serieHumedad;      // Serie para la humedad
    QValueAxis *ejeYTemperatura;    // Eje Y para la temperatura
    QValueAxis *ejeYHumedad;        // Eje Y para la humedad
    QValueAxis *ejeX;
    QGraphicsTextItem *etiquetaDinamica = nullptr;


    QSerialPort *serialActual = nullptr;



};
#endif // MAINWINDOW_H
