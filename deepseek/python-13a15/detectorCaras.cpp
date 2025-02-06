/*
g++ -Wall detectorCaras.cpp -o detectorCaras `pkg-config --cflags --libs opencv4`

  */

#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main() {
    // Cargar el clasificador preentrenado para detección de caras
    CascadeClassifier face_cascade;
    if (!face_cascade.load(cv::samples::findFile("haarcascade_frontalface_default.xml"))) {
        cout << "Error al cargar el clasificador de caras." << endl;
        return -1;
    }

    // Iniciar la captura de video desde la cámara web
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cout << "No se pudo abrir la cámara." << endl;
        return -1;
    }

    Mat frame;
    while (true) {
        // Capturar frame por frame
        cap >> frame;

        // Verificar si el frame se capturó correctamente
        if (frame.empty()) {
            cout << "No se pudo recibir el frame. Saliendo..." << endl;
            break;
        }

        // Convertir el frame a escala de grises
        Mat gray;
        cvtColor(frame, gray, COLOR_BGR2GRAY);

        // Detectar caras en el frame
        vector<Rect> faces;
        face_cascade.detectMultiScale(gray, faces, 1.1, 5, 0, Size(30, 30));

        // Dibujar un rectángulo alrededor de cada cara detectada
        for (const Rect& face : faces) {
            rectangle(frame, face, Scalar(0, 255, 0), 2); // Color verde, grosor 2
        }

        // Mostrar el frame con las caras detectadas
        imshow("Detección de Caras", frame);

        // Esperar 1 milisegundo y verificar si se presionó la tecla 'q'
        if (waitKey(1) == 'q') {
            break;
        }
    }

    // Liberar la cámara y cerrar todas las ventanas
    cap.release();
    destroyAllWindows();

    return 0;
}
