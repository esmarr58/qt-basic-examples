#include "mainwindow.h"
#include "ui_mainwindow.h"

//Paso #9. Declarar el espacio de nombre
using namespace cv;

//Paso #16. Crear la funcion del paso #15
void MainWindow::leerImagenesCamara(){
    //Paso #20. Leer la imagen de la camara
    camara >> imagenPrincipal;
    //Paso #22. Si la imagen esta vacia, termino el programa
    if(imagenPrincipal.empty()) return;

    //Paso #23. Redimensionar la imagen
    cv::resize(imagenPrincipal, imagenChica, cv::Size(800,600));

    //Paso #24. Convertir la imagen del #23 a una imagen QImage, usando la funcion del paso #7
    QImage imagenQT = MatToQImage(imagenChica);

    //Paso #25. Convertir QImage a QPixmap
    QPixmap mapaPixeles = QPixmap::fromImage(imagenQT);

    //Paso #26. Visualizar el resultado en un label
    ui->label->setPixmap(mapaPixeles);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //Paso #11. Abrir la camara.
    if(!camara.open(0)) qDebug() << "No pude abrir la camara";
    //Paso #14 - Inicializar el cronometro
    cronometro = new QTimer(this);
    //Paso #17 - Configurar el cronometro
    connect(cronometro, SIGNAL(timeout()), this, SLOT(leerImagenesCamara()));
    //Paso #18 - Iniciar el cronometro
    cronometro->start(30);
}

MainWindow::~MainWindow()
{
    delete ui;
}


//Paso # 7. Funcion para convertir una imagen de qt a una QImage
QImage MainWindow::MatToQImage(const cv::Mat& src) {
    QImage dest(src.cols, src.rows, QImage::Format_ARGB32);

    const float scale = 255.0;

    if (src.depth() == CV_8U) {
        if (src.channels() == 1) {
            for (int i = 0; i < src.rows; ++i) {
                for (int j = 0; j < src.cols; ++j) {
                    int level = src.at<quint8>(i, j);
                    dest.setPixel(j, i, qRgb(level, level, level));
                }
            }
        } else if (src.channels() == 3) {
            for (int i = 0; i < src.rows; ++i) {
                for (int j = 0; j < src.cols; ++j) {
                    cv::Vec3b bgr = src.at<cv::Vec3b>(i, j);
                    dest.setPixel(j, i, qRgb(bgr[2], bgr[1], bgr[0]));
                }
            }
        }
    } else if (src.depth() == CV_32F) {
        if (src.channels() == 1) {
            for (int i = 0; i < src.rows; ++i) {
                for (int j = 0; j < src.cols; ++j) {
                    int level = scale * src.at<float>(i, j);
                    dest.setPixel(j, i, qRgb(level, level, level));
                }
            }
        } else if (src.channels() == 3) {
            for (int i = 0; i < src.rows; ++i) {
                for (int j = 0; j < src.cols; ++j) {
                    cv::Vec3f bgr = scale * src.at<cv::Vec3f>(i, j);
                    dest.setPixel(j, i, qRgb(bgr[2], bgr[1], bgr[0]));
                }
            }
        }
    }

    return dest;
}

void MainWindow::on_pushButton_clicked()
{
    //Paso #27. Verificar si la imagen esta vacia
    if(imagenPrincipal.empty()) return;

    //Paso #28. Tomar una imagen
    cv::imwrite("imagen.jpg",imagenPrincipal);

}

