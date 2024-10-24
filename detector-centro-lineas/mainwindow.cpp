#include "mainwindow.h"
#include "ui_mainwindow.h"

//Paso #9. Declarar el espacio de nombre
using namespace cv;
//Paso # TC9-1.
cv::Mat imagenNueva;

#include <opencv2/opencv.hpp>

#include <opencv2/opencv.hpp>

#include <opencv2/opencv.hpp>

cv::Mat MainWindow::detectarCentroEntreLineas(const cv::Mat& imagenBinaria) {
    // Verificar que la imagen de entrada sea binaria y tenga las dimensiones correctas
    if (imagenBinaria.empty() || imagenBinaria.type() != CV_8UC1 || imagenBinaria.cols != 640 || imagenBinaria.rows != 480) {
        throw std::invalid_argument("La imagen debe ser binaria (1 canal) de tamaño 640x480");
    }

    // Crear la región de interés (ROI) centrada en el origen, de 50px de alto por 640px de ancho
    int alturaROI = 50;
    int anchoROI = 640;
    int yInicioROI = (imagenBinaria.rows - alturaROI) / 2;  // Centrada verticalmente
    cv::Rect regionInteres(0, yInicioROI, anchoROI, alturaROI);

    // Extraer la región de interés de la imagen binaria
    cv::Mat roi = imagenBinaria(regionInteres);

    // Detectar las líneas blancas a la izquierda y a la derecha en la ROI
    int lineaIzquierda = -1;
    int lineaDerecha = -1;

    // Buscar la primera línea blanca desde la izquierda
    for (int x = 0; x < roi.cols; ++x) {
        if (cv::countNonZero(roi.col(x)) > 0) {
            lineaIzquierda = x;
            break;
        }
    }

    // Buscar la primera línea blanca desde la derecha
    for (int x = roi.cols - 1; x >= 0; --x) {
        if (cv::countNonZero(roi.col(x)) > 0) {
            lineaDerecha = x;
            break;
        }
    }

    // Verificar si se encontraron ambas líneas
    if (lineaIzquierda == -1 || lineaDerecha == -1) {
        throw std::runtime_error("No se encontraron suficientes líneas blancas en la imagen.");
    }

    // Calcular la posición del centro entre las dos líneas detectadas
    int centroEntreLineas = (lineaIzquierda + lineaDerecha) / 2;

    // Crear una nueva imagen BGR de 640x480 con fondo negro
    cv::Mat imagenResultado = cv::Mat::zeros(480, 640, CV_8UC3);

    // Dibujar las líneas blancas en la posición detectada
    cv::line(imagenResultado, cv::Point(lineaIzquierda, yInicioROI), cv::Point(lineaIzquierda, yInicioROI + alturaROI), cv::Scalar(255, 255, 255), 2);
    cv::line(imagenResultado, cv::Point(lineaDerecha, yInicioROI), cv::Point(lineaDerecha, yInicioROI + alturaROI), cv::Scalar(255, 255, 255), 2);

    // Dibujar un círculo azul cuyo centro está en la mitad de la distancia entre las líneas
    int radioCirculo = 10;
    cv::circle(imagenResultado, cv::Point(centroEntreLineas, imagenBinaria.rows / 2), radioCirculo, cv::Scalar(255, 0, 0), -1);  // Círculo azul

    return imagenResultado;
}


cv::Mat MainWindow::combinarImagenes(const cv::Mat& imagenOriginal, const cv::Mat& imagenResultado) {
    // Verificar que ambas imágenes tengan el mismo tamaño y número de canales
    if (imagenOriginal.size() != imagenResultado.size() || imagenOriginal.type() != imagenResultado.type()) {
        throw std::invalid_argument("Las imágenes deben tener el mismo tamaño y tipo.");
    }

    // Crear una máscara donde los píxeles del círculo azul no son blancos (es decir, el círculo)
    cv::Mat mascara;
    cv::inRange(imagenResultado, cv::Scalar(255, 255, 255), cv::Scalar(255, 255, 255), mascara);
    cv::bitwise_not(mascara, mascara);  // Invertir la máscara para obtener solo el círculo azul

    // Crear una imagen de salida combinada
    cv::Mat imagenCombinada = imagenOriginal.clone();

    // Superponer la imagen resultado sobre la original usando la máscara
    imagenResultado.copyTo(imagenCombinada, mascara);

    return imagenCombinada;
}


cv::Mat MainWindow::detectarLineasBlancas(const cv::Mat& imagenBGR, int nivelDeteccion) {
    // Asegurar que el nivel de detección esté dentro del rango [0, 100]
    nivelDeteccion = std::clamp(nivelDeteccion, 0, 100);

    // Convertir la imagen BGR a escala de grises
    cv::Mat imagenGris;
    cv::cvtColor(imagenBGR, imagenGris, cv::COLOR_BGR2GRAY);

    // Aplicar un filtro de desenfoque para reducir el ruido
    cv::Mat imagenDesenfocada;
    cv::GaussianBlur(imagenGris, imagenDesenfocada, cv::Size(5, 5), 0);

    // Ajustar los umbrales de Canny en función del nivel de detección
    int umbralBajo = 50 + (nivelDeteccion * 1.5);   // Rango dinámico de 50 a 200
    int umbralAlto = 150 + (nivelDeteccion * 1.5);  // Rango dinámico de 150 a 300

    // Aplicar la detección de bordes Canny para encontrar los contornos
    cv::Mat bordes;
    cv::Canny(imagenDesenfocada, bordes, umbralBajo, umbralAlto);

    // Ajustar el umbral binario para las líneas blancas en función del nivel de detección
    int umbralBinario = 200 - (nivelDeteccion * 2);  // Disminuir el umbral con mayor detección
    cv::Mat lineasBlancas;
    cv::threshold(imagenGris, lineasBlancas, umbralBinario, 255, cv::THRESH_BINARY);

    // Combinar la detección de bordes con la máscara de líneas blancas
    cv::Mat resultado;
    cv::bitwise_and(bordes, lineasBlancas, resultado);

    return resultado;
}


//Paso #16. Crear la funcion del paso #15
void MainWindow::leerImagenesCamara(){
    //Paso #20. Leer la imagen de la camara
    camara >> imagenPrincipal;
    //Paso #22. Si la imagen esta vacia, termino el programa
    if(imagenPrincipal.empty()) return;

    //Ver resolucion
    qDebug() << "Ancho: " << imagenPrincipal.cols <<", Alto: "<< imagenPrincipal.rows;



    //Paso # TC9-2 - Decidir el espacio de color
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
        imagenNueva = detectarLineasBlancas(imagenPrincipal,ui->verticalScrollBar->value());
    }
    else if(ui->radioButton_5->isChecked()){

        cv::Mat imagenBN = detectarLineasBlancas(imagenPrincipal,ui->verticalScrollBar->value());
        cv::Mat imagenResultado = detectarCentroEntreLineas(imagenBN);
        //cv::Mat imagenResultado3C;
        //cv::cvtColor(imagenResultado,imagenResultado3C,cv::COLOR_GRAY2BGR);
        //imagenNueva = combinarImagenes(imagenPrincipal,imagenResultado);
        imagenResultado.copyTo(imagenNueva);
    }
    //Paso # TC9-3 - Comentar el paso #23.
    //Paso # TC9-4 - Copiar el paso 23 y cambiar la imagen de origen.
    cv::resize(imagenNueva, imagenChica, cv::Size(800,600));


    //Paso #23. Redimensionar la imagen
    //cv::resize(imagenPrincipal, imagenChica, cv::Size(800,600));

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
    cv::imwrite("imagen.jpg",imagenNueva);

}

