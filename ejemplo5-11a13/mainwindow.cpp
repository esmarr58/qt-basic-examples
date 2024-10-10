#include "mainwindow.h"
#include "ui_mainwindow.h"

//Paso # 26. Crear la funcion guardarImagen
void MainWindow::guardarImagen(){
    //Paso #27. Verificar que la imagen no este vacia
    if(imagenPrincipal.empty()) return;

    //Paso #28. Guardar la imagen
    cv::imwrite("imagen.jpg",imagenPrincipal);
}

//Paso # 12. Crear la funcion del paso #11
void MainWindow::leerImagenCamara(){
    //Paso # 19. Leer una imagen
    camara >> imagenPrincipal;

    //Paso # 21. Redimensionar la imagenPrincipal
    cv::resize(imagenPrincipal, imagenChica, cv::Size(800,600));

    //Paso # 22. Convertir Mat->QImage
    QImage imagenQT = MatToQImage(imagenChica);

    //Paso # 23. Convertir QImage -> QPixmap
    QPixmap mapaPixeles = QPixmap::fromImage(imagenQT);

    //Paso #24. Visualizar el resultado (mapaPixeles)
    ui->label->setPixmap(mapaPixeles);

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //Paso # 10. Inicializar el QTimer
    cronometro = new QTimer(this);

    //Paso # 13. Configurar el QTimer
    connect(cronometro, SIGNAL(timeout()),this,SLOT(leerImagenCamara()));

    //Paso # 14. Iniciar el QTimer
    cronometro->start(30);

    //Paso #17. Abrir una camara.Si la camara no se abre, le mandamos un mensaje al usuario
    if(!camara.open(0)) QMessageBox::information(this, "Error","No se pudo abrir la camara");

    //Paso #29. Conectar la se;al del clicked del boton con la funcion guardarImagen
    connect(ui->pushButton, SIGNAL(clicked()),this, SLOT(guardarImagen()));
    //En el directorio de trabajo se guarda la imagen

}

MainWindow::~MainWindow()
{
    delete ui;
}

//Paso # 6. Agregar una funcion, que permita convertir un imagen de opencv
//(cv::Mat a una QImage)
//Paso # 7. Agregar con clic derecho-refactor-add privateslotdeclaration
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
