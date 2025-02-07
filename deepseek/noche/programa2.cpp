#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    // Abre la cámara web (0 es la cámara por defecto)
    cv::VideoCapture cap(0);

    if (!cap.isOpened()) {
        std::cerr << "Error: No se pudo abrir la cámara." << std::endl;
        return -1;
    }

    cv::Mat frame;
    std::cout << "Presiona 'ESC' para salir..." << std::endl;

    while (true) {
        // Captura un frame de la cámara
        cap >> frame;

        // Si el frame está vacío, sal del bucle
        if (frame.empty()) {
            std::cerr << "Error: Frame vacío." << std::endl;
            break;
        }

        // Muestra el frame en una ventana
        cv::imshow("Camara Web", frame);

        // Espera 30 ms y verifica si se presionó la tecla ESC
        if (cv::waitKey(30) == 27) {
            break;
        }
    }

    // Libera la cámara y cierra la ventana
    cap.release();
    cv::destroyAllWindows();

    return 0;
}
