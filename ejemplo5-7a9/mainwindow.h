#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
//Paso # 6. Agregar bibliotecas para opencv
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

//Paso #12 Uso de qtimer
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
    //Paso # 8. Agregar la funcion a las funciones privadas
    QImage MatToQImage(const cv::Mat &src);
    //Paso #15. Crear funcion para el cronometro
    void leerImagenesCamara();
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    //Paso #10. Crear una variable para la camaraWEB
    cv::VideoCapture camara;
    //Paso #13. Declarar variable para el QTimer
    QTimer *cronometro;
    //Paso #19. Crear una variable para guardar la imagen
    cv::Mat imagenPrincipal;
    //Paso #21. Crear una variable para una imagen mas peque;a
    cv::Mat imagenChica;

};
#endif // MAINWINDOW_H
