#include "mainwindow.h"
#include "ui_mainwindow.h"
//Paso # 10. Crear una funcion para el QTimer
//Paso # 11. Agregar la funcion del paso #10 al refactor
//TC # 9 - Paso # 2.
cv::Mat imagenNueva;

void MainWindow::leerImagen(){
    //Paso # 17. Leer una imagen de la camara
    camara >> imagenPrincipal;

    //QString fecha = QDateTime::currentDateTime().toString("dd/MM/yyyy HH:mm:ss");
    //cv::putText(imagenPrincipal,fecha.toUtf8().constData(),cv::Point(0,75),0,1,cv::Scalar(0,0,255),1,8,0);


    //TC # 5 - Paso # 5. Decision
    if(ui->radioButton->isChecked()){
        imagenPrincipal.copyTo(imagenNueva);
    }
    else if(ui->radioButton_2->isChecked()){
        cv::cvtColor(imagenPrincipal, imagenNueva, cv::COLOR_BGR2GRAY);
    }
    else if(ui->radioButton_3->isChecked()){
        cv::cvtColor(imagenPrincipal, imagenNueva, cv::COLOR_BGR2HSV);
    }
    else if(ui->radioButton_4->isChecked()){

        cv::Mat paso2 = detectEdges(imagenPrincipal, ui->verticalScrollBar->value());
        cv::Point centroCamino;
        imagenNueva = processBinaryImage(paso2,centroCamino);


    }
    else{
        return;
    }



    //TC # 9 - Paso # 4.
    //Verificar que no procesar matrices vacias
    if(imagenNueva.empty()) return;

    //TC # 9 - Paso # 3.
    cv::resize(imagenNueva, imagenChica, cv::Size(800,600));

    //Paso TC9 # Paso #1. Comentar paso # 19.
    /*Vision artificial */
    //Paso # 19. Redimensionar la imagenPrincipal, guardar el resultado en imagenChica
    //cv::resize(imagenPrincipal, imagenChica, cv::Size(800,600));

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
    cv::imwrite("/home/r/Downloads/instruso.jpg", imagenNueva);

}



// Función para detectar bordes en una imagen BGR y devolver una imagen binaria en formato cv::Mat
cv::Mat MainWindow::detectEdges(const cv::Mat& inputImage, int detectionLevel)
{
    // Verificar si la imagen de entrada está vacía
    if (inputImage.empty()) {
        qDebug() << "La imagen está vacía!";
        return cv::Mat();
    }

    // Convertir la imagen BGR a escala de grises
    cv::Mat grayImage;
    cv::cvtColor(inputImage, grayImage, cv::COLOR_BGR2GRAY);

    // Aplicar desenfoque para reducir el ruido
    cv::Mat blurredImage;
    cv::GaussianBlur(grayImage, blurredImage, cv::Size(5, 5), 1.5);

    // Ajustar el valor del umbral de acuerdo al nivel de detección proporcionado (entre 0 y 100)
    int threshold = cv::max(1, detectionLevel * 2); // Umbral mínimo de 1, máximo de 200

    // Detectar bordes utilizando el algoritmo Canny
    cv::Mat edges;
    cv::Canny(blurredImage, edges, threshold, threshold * 2);

    // Devolver la imagen binaria resultante (1 canal)
    return edges;
}
// Función para procesar una imagen binaria de 1 canal y devolver una imagen BGR con una ROI roja, líneas amarillas y un círculo azul
cv::Mat MainWindow::processBinaryImage(const cv::Mat& binaryImage, cv::Point& centerPoint)
{
    // Verificar que la imagen de entrada sea válida y tenga 1 canal
    if (binaryImage.empty() || binaryImage.channels() != 1) {
        qDebug() << "La imagen de entrada no es válida o no es de 1 canal.";
        return cv::Mat();
    }

    // Dimensiones de la imagen
    int width = binaryImage.cols;
    int height = binaryImage.rows;

    // Verificar que la imagen tenga las dimensiones correctas
    if (width != 640 || height != 480) {
        qDebug() << "La imagen no es de 640x480.";
        return cv::Mat();
    }

    // Crear una imagen BGR a partir de la imagen binaria para visualización
    cv::Mat bgrImage;
    cv::cvtColor(binaryImage, bgrImage, cv::COLOR_GRAY2BGR);

    // Definir la región de interés (ROI) centrada de 640x50 píxeles
    int roiHeight = 50;
    int roiWidth = 640;
    int roiY = (height / 2) - (roiHeight / 2);  // Posición Y del centro
    cv::Rect roi(0, roiY, roiWidth, roiHeight);  // ROI centrado

    // Dibujar un rectángulo rojo alrededor de la región de interés
    cv::rectangle(bgrImage, roi, cv::Scalar(0, 0, 255), 2); // Color rojo (BGR)

    // Extraer la región de interés de la imagen binaria
    cv::Mat roiBinary = binaryImage(roi);

    // Variables para almacenar las posiciones de las líneas a la izquierda y derecha del centro
    int leftLine = -1;
    int rightLine = -1;
    int centerX = roiWidth / 2;  // Centro de la región de interés

    // Detectar las posiciones de las líneas a la izquierda y derecha del centro
    for (int x = 0; x < centerX; ++x) {
        // Buscar la primera línea blanca (valor 255) desde la izquierda hacia el centro
        if (cv::countNonZero(roiBinary.col(x)) > 0 && leftLine == -1) {
            leftLine = x;
        }
        // Buscar la primera línea blanca (valor 255) desde la derecha hacia el centro
        if (cv::countNonZero(roiBinary.col(roiWidth - 1 - x)) > 0 && rightLine == -1) {
            rightLine = roiWidth - 1 - x;
        }
        // Si ambas líneas han sido encontradas, salir del bucle
        if (leftLine != -1 && rightLine != -1) {
            break;
        }
    }

    // Si no se encontraron líneas, devolver la imagen original sin cambios
    if (leftLine == -1 || rightLine == -1) {
        qDebug() << "No se encontraron líneas a la izquierda o derecha.";
        return bgrImage;
    }

    // Dibujar las líneas detectadas en amarillo
    cv::line(bgrImage, cv::Point(leftLine, roiY), cv::Point(leftLine, roiY + roiHeight), cv::Scalar(0, 255, 255), 2); // Línea izquierda en amarillo
    cv::line(bgrImage, cv::Point(rightLine, roiY), cv::Point(rightLine, roiY + roiHeight), cv::Scalar(0, 255, 255), 2); // Línea derecha en amarillo

    // Calcular el punto medio entre las dos líneas detectadas
    int midX = (leftLine + rightLine) / 2;
    centerPoint = cv::Point(midX, roiY + (roiHeight / 2));  // Centro en la imagen completa

    // Dibujar un círculo azul en el punto medio encontrado
    cv::circle(bgrImage, centerPoint, 5, cv::Scalar(255, 0, 0), -1); // Círculo azul

    // Devolver la imagen BGR con el círculo azul, líneas amarillas y la ROI en rojo
    return bgrImage;
}
