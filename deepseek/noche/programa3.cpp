#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    // Abre la cámara web (0 es la cámara por defecto)
    cv::VideoCapture cap(0);

    if (!cap.isOpened()) {
        std::cerr << "Error: No se pudo abrir la cámara." << std::endl;
        return -1;
    }

    // Carga el clasificador preentrenado para la detección de caras
    cv::CascadeClassifier faceCascade;
    if (!faceCascade.load("/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml")) {
        std::cerr << "Error: No se pudo cargar el clasificador de caras." << std::endl;
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

        // Convierte el frame a escala de grises (la detección de caras funciona mejor en escala de grises)
        cv::Mat grayFrame;
        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);

        // Detecta caras en el frame
        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(grayFrame, faces, 1.1, 5);

        // Dibuja rectángulos alrededor de las caras detectadas
        for (const auto& face : faces) {
            cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2); // Rectángulo verde con grosor 2
        }

        // Muestra el frame con las caras detectadas
        cv::imshow("Detección de Caras", frame);

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
