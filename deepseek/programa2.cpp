#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    // Abre la cámara web (0 es la cámara predeterminada)
    cv::VideoCapture cap(0);

    // Verifica si la cámara se abrió correctamente
    if (!cap.isOpened()) {
        std::cerr << "Error: No se pudo abrir la cámara." << std::endl;
        return -1;
    }

    // Crea una ventana para mostrar la imagen
    cv::namedWindow("Camara Web", cv::WINDOW_AUTOSIZE);

    while (true) {
        cv::Mat frame;

        // Captura un frame de la cámara
        cap >> frame;

        // Verifica si el frame se capturó correctamente
        if (frame.empty()) {
            std::cerr << "Error: No se pudo capturar el frame." << std::endl;
            break;
        }

        // Muestra el frame en la ventana
        cv::imshow("Camara Web", frame);

        // Espera 30 ms y verifica si el usuario presionó la tecla 'q'
        if (cv::waitKey(30) == 'q') {
            break;
        }
    }

    // Libera la cámara y cierra la ventana
    cap.release();
    cv::destroyAllWindows();

    return 0;
}
