#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

//Paso # 4. INcluir bibliotecas para opencv
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

//Paso # 9. Biblioteca
#include<QTimer>

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
    QImage MatToQImage(const cv::Mat &src);
    //Paso # 12. Declarar funcion para leer imagenes
    void leerImagenCamara();

    //Paso # 24. Crear una funcion para guardarImagen
    void guardarImagen();

private:
    Ui::MainWindow *ui;
    //Paso # 7. Crear una variable para la camara
    cv::VideoCapture camara;
    //Paso # 10. Declarar variable para el QTimer
    QTimer *cronometro;
    //Paso # 16. Declarar variable para guardar la imagen de la camara
    cv::Mat imagenPrincipal;

    //Paso # 19. Declarar variable para una imagen redimensionada
    cv::Mat imagenChica;



};
#endif // MAINWINDOW_H
