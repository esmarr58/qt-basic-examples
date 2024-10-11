#include "mainwindow.h"
#include "ui_mainwindow.h"

//Paso # 25. Hacer funcion guardarImagen
void MainWindow::guardarImagen(){
    //Paso # 27. Verificar si la imagen esta vacia
    if(imagenPrincipal.empty()) return;

    //Paso # 28. Guardar la imagen
    cv::imwrite("/home/r/Downloads/imagen.jpg", imagenPrincipal);

}

//Paso # 13. Crear la funcion leerImagenCamara
void MainWindow::leerImagenCamara(){
    //Paso # 17. Leer una imagen de la camara
    camara >> imagenPrincipal;

    //Paso # 18. Verificar que la imagen tenga informacion
    if(imagenPrincipal.empty()) return;

    //Paso # 20. Redimensionar la imagen Principal
    cv::resize(imagenPrincipal, imagenChica, cv::Size(800,600));

    //Paso # 21. Convertir Mat -> QImage
    //MatToQImage se hizo en el paso # 5.
    QImage imagenQT = MatToQImage(imagenChica);

    //Paso # 22. Convertir QImage -> QPixmap
    QPixmap mapaPixeles = QPixmap::fromImage(imagenQT);

    //Paso # 23. Visualizar el resultado
    ui->label->setPixmap(mapaPixeles);


}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //Paso # 8. Verificar y abrir la camara web
    if(!camara.open(0)) qDebug() << "Error al abrir la camara";


    //Paso #11. INicializar el QTimer
    cronometro = new QTimer(this);

    //Paso #14 - > Conexion del timeout -> leerImagenCamara
    connect(cronometro, SIGNAL(timeout()),this, SLOT(leerImagenCamara()));

    //Paso #15 -> Iniciar la cuenta del QTimer
    cronometro->start(30);

    //Paso # 26. Conectar la se;al clic del boton guardar con la funcion guardarImagen
    connect(ui->pushButton, SIGNAL(clicked()),this, SLOT(guardarImagen()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

//Paso # 5. Crear una funcion para Convertir Mat -> QImage
//Paso # 6. Agregar la funcion a las funciones privadas
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


