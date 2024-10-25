#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
//Paso # 4. Agregar las bibliotecas de opencv
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

//Paso # 7. Biblioteca QTIMER
#include<QTimer>

#include<QDateTime>

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
    void leerImagen();
    //Paso 23 -  Crear una funcion para guardarImagenes
    void guardarImagen();
    cv::Mat detectEdges(const cv::Mat& inputImage, int detectionLevel);
    cv::Mat processBinaryImage(const cv::Mat& binaryImage, cv::Point& centerPoint);

private:
    Ui::MainWindow *ui;
    //Paso # 8. Declaro un QTimer
    QTimer *cronometro;
    //Paso # 14. Declarar una variable para usar la camara
    cv::VideoCapture camara;

    //Paso # 16. Declaro una variable privada para guardar la imagenPrincipal de la camara
    cv::Mat imagenPrincipal;

    //Paso # 18. Declarar una variable para guardar una imagen mas reducida
    cv::Mat imagenChica;
};
#endif // MAINWINDOW_H
