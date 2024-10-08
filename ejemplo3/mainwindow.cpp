#include "mainwindow.h"
#include "ui_mainwindow.h"

//Paso # 2. Crear una funcion para el QTimer.
//Paso # 3. Agregar el prototipo de esta funcion
//a la cabecera, clic-derecho al nombre de la funcion
//refactor -> add private slot declation
void MainWindow::actualizarTiempo(){
    //Paso # 8. Crear una variable y asignarle el tiempo actual
    //QDateTime tiempoActual = QDateTime::currentDateTime();

    //Paso # 9. Mostrarlo en la etiqueta.
    //ui->label->setText(tiempoActual.toString());

    //Punto Extra.
    //Agregar LCDNumber para visualizar la hora, minuto y el segundo
    //QTime tiempo = QTime::currentTime();
    //int horas = tiempo.hour();

    //Paso # 12. Cambiar la forma de visualizar el tiempo
    //COmentar el paso 8 y el 9.

    //Paso # 13. Obtener el tiempo epoch.
    //El tiempo epoch son los segundos que han transcurrido,
    //desde el January 1, 1970 (midnight UTC/GMT)
    long int tiempoEpoch = QDateTime::currentSecsSinceEpoch();

    //Paso # 14. Convertir el tiempo epoch (long int) a un texto
    QString tiempoEpochTexto = QString::number(tiempoEpoch);

    //Paso # 15. Visualizar el texto en el label
    ui->label->setText(tiempoEpochTexto);

    //Pregunta, que tiempo se maneja en Mexico?
    //UTC-6
    //UTC? El tiempo en el meridiano de Greenwich



}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //Paso # 4. Crear el QTimer
    QTimer *cronos = new QTimer(this);
    //Paso # 5. Conectar la se;al timeout con la funcion anterior
    connect(cronos, SIGNAL(timeout()), this, SLOT(actualizarTiempo()));
    //Paso # 6. Iniciar el QTimer
    cronos->start(1000);

    //Paso # 11.Cambiar la variable locale a espa;ol
    QLocale cambiarEsp(QLocale::Spanish, QLocale::Mexico);


    //Paso # 11.2 Establecer la variable anterior por defecto
    QLocale::setDefault(cambiarEsp);


}

MainWindow::~MainWindow()
{
    delete ui;
}
