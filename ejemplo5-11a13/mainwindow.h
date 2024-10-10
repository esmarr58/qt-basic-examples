#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
//Paso # 5. Agregar las bibliotecas para opencv
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>


//Paso # 8. Biblioteca
#include<QTimer>

//Paso # 16
#include<QMessageBox>

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

    //Paso # 11. Declarar una funcion privada, que llamara el QTimer
    void leerImagenCamara();

    //Paso # 25. Declarar una funcion para guardarImagenes
    void guardarImagen();

private:
    Ui::MainWindow *ui;
    //Paso # 9. Declarar variable para QTimer
    QTimer *cronometro;

    //Paso # 15. Declarar una variable para la camara
    cv::VideoCapture camara;

    //Paso #18. Declarar una variable privada, para guardar la imagen de la camara
    cv::Mat imagenPrincipal;

    //Paso #20. Declarar una variable para guardar el proceso de redimensionar de la imagenPrincipal
    cv::Mat imagenChica;
};
#endif // MAINWINDOW_H
