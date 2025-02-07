
/*
g++ -Wall programa4.cpp -o detectorCaras `pkg-config --cflags --libs opencv4` -lsfml-audio -lsfml-system

    */
#include <opencv2/opencv.hpp>
#include <SFML/Audio.hpp>
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

    // Cargar el sonido de alarma
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile("alarma.wav")) {
        cout << "Error al cargar el archivo de sonido 'alarma.wav'." << endl;
        return -1;
    }
    sf::Sound alarmSound;
    alarmSound.setBuffer(buffer);

    // Iniciar la captura de video desde la cámara web
    VideoCapture cap("http://10.109.228.65:8080/video");
    if (!cap.isOpened()) {
        cout << "No se pudo abrir la cámara." << endl;
        return -1;
    }

    Mat frame;
    bool faceDetected = false; // Variable para rastrear si se detectó una cara

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

        // Dibujar un rectángulo alrededor de cada cara detectada y mostrar el centro
        for (const Rect& face : faces) {
            // Dibujar el rectángulo alrededor de la cara
            rectangle(frame, face, Scalar(0, 255, 0), 2); // Color verde, grosor 2

            // Calcular el centro de la cara
            Point center(face.x + face.width / 2, face.y + face.height / 2);

            // Dibujar un círculo en el centro de la cara
            circle(frame, center, 5, Scalar(0, 0, 255), -1); // Color rojo, radio 5, relleno

            // Mostrar las coordenadas del centro en la imagen
            string text = "(" + to_string(center.x) + ", " + to_string(center.y) + ")";
            putText(frame, text, Point(face.x, face.y - 10), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1);

            // Reproducir el sonido de alarma si no se está reproduciendo ya
            if (!faceDetected) {
                alarmSound.play();
                faceDetected = true;
            }
        }

        // Si no se detectan caras, detener el sonido de alarma
        if (faces.empty() && faceDetected) {
            alarmSound.stop();
            faceDetected = false;
        }

        // Mostrar el frame con las caras detectadas y las coordenadas del centro
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
