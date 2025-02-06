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

    // Carga el clasificador en cascada para detección de caras
    cv::CascadeClassifier faceCascade;
    if (!faceCascade.load(cv::samples::findFile("haarcascade_frontalface_default.xml"))) {
        std::cerr << "Error: No se pudo cargar el clasificador de caras." << std::endl;
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

        // Convierte el frame a escala de grises (la detección de caras funciona mejor en grises)
        cv::Mat grayFrame;
        cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);

        // Detecta caras en el frame
        std::vector<cv::Rect> faces;
        faceCascade.detectMultiScale(grayFrame, faces, 1.1, 5);

        // Dibuja rectángulos alrededor de las caras detectadas
        for (const auto& face : faces) {
            cv::rectangle(frame, face, cv::Scalar(0, 255, 0), 2); // Rectángulo verde
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
