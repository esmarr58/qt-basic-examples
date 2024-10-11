#include "mainwindow.h"
#include "ui_mainwindow.h"
//Paso # 10. Crear una funcion para el QTimer
//Paso # 11. Agregar la funcion del paso #10 al refactor
void MainWindow::leerImagen(){
    //Paso # 17. Leer una imagen de la camara
    camara >> imagenPrincipal;

    QString fecha = QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss");
    cv::putText(imagenPrincipal,fecha.toUtf8().constData(),cv::Point(0,75),0,1,cv::Scalar(0,0,255),1,8,0);

    /*Vision artificial */
    //Paso # 19. Redimensionar la imagenPrincipal, guardar el resultado en imagenChica
    cv::resize(imagenPrincipal, imagenChica, cv::Size(800,600));

    //Paso # 20. Convertir la matriz anterior a una QImage
    QImage imagenQT = MatToQImage(imagenChica);

    //Paso # 21. Convertir QImage -> QPixmap
    QPixmap mapaPixeles = QPixmap::fromImage(imagenQT);

    //Paso # 22. Mostrar el resultado
    ui->label->setPixmap(mapaPixeles);

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //Paso # 9. Inicializar el cronometro
    cronometro = new QTimer(this);

    //Paso # 12, configurar la funcion leerImagen con la se;al timeout
    connect(cronometro, SIGNAL(timeout()),this, SLOT(leerImagen()));

    //Paso # 13. Inicio el cronometro
    cronometro->start(30);

    //Paso # 15. Abro y verifico que la camara este abierta
    if(!camara.open(0)) qDebug() << "Error al abrir la acamara";
    //0 es la camara por defecto
    //2 hay una camara por defecto y una camara usb, la camara usb 2

    //Paso # 25 Conectar el click del boton con la funcion guardarImagen
    connect(ui->pushButton, SIGNAL(clicked()),this, SLOT(guardarImagen()));


}

MainWindow::~MainWindow()
{
    delete ui;
}

//Paso # 5. Agregar una funcion que permita convertir Mat -> QImage
//Paso # 6. Agregar la funcion al refactor,private-slot-declaration
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

//Paso # 24. Crear la funcion para guardar Imagenes
void MainWindow::guardarImagen(){
    //Paso # 26. Verificar que la imagen no este vacia
    if(imagenPrincipal.empty()) return;

    //Paso # 26.1 Agregar la fecha de deteccion
    QString fecha = QDateTime::currentDateTime().toString("dd/MM/yyy HH:mm:ss");
    cv::putText(imagenPrincipal,fecha.toUtf8().constData(),cv::Point(0,75),0,1,cv::Scalar(0,0,255),1,8,0);
    //Paso # 27. Guardo la imagen
    cv::imwrite("/home/r/Downloads/instruso.jpg", imagenPrincipal);

}
