#include <opencv2/opencv.hpp>
#include <filesystem>
#include <iostream>
#include <vector>
#include <cstdlib>

namespace fs = std::filesystem;

static std::string find_haar_cascade_file(const std::string& filename)
{
    // 1) Si el usuario define un directorio por variable de entorno:
    //    export OPENCV_HAAR_DIR=/ruta/a/haarcascades
    if (const char* env = std::getenv("OPENCV_HAAR_DIR")) {
        fs::path p = fs::path(env) / filename;
        if (fs::exists(p)) return p.string();
    }

    // 2) Rutas típicas en Ubuntu (varían según paquete/opencv-data/libopencv-data)
    const std::vector<fs::path> dirs = {
        "/usr/share/opencv4/haarcascades",
        "/usr/share/opencv/haarcascades",
        "/usr/local/share/opencv4/haarcascades",
        "/usr/local/share/opencv/haarcascades",
        "/snap/opencv/current/usr/share/opencv4/haarcascades",
        "/var/lib/snapd/snap/opencv/current/usr/share/opencv4/haarcascades"
    };

    for (const auto& d : dirs) {
        fs::path p = d / filename;
        if (fs::exists(p)) return p.string();
    }

    // 3) Búsqueda “a ciegas” en algunos padres comunes (por si cambió la ruta)
    //    OJO: esto es más lento, pero útil si no está en rutas estándar.
    const std::vector<fs::path> roots = {
        "/usr/share",
        "/usr/local/share"
    };

    for (const auto& r : roots) {
        if (!fs::exists(r)) continue;
        try {
            for (auto const& entry : fs::recursive_directory_iterator(r)) {
                if (!entry.is_regular_file()) continue;
                if (entry.path().filename() == filename) {
                    return entry.path().string();
                }
            }
        } catch (...) {
            // Si alguna carpeta no tiene permisos, ignoramos y seguimos.
        }
    }

    return "";
}

int main()
{
    const std::string cascade_name = "haarcascade_frontalface_default.xml";
    const std::string cascade_path = find_haar_cascade_file(cascade_name);

    if (cascade_path.empty()) {
        std::cerr << "No encontre el archivo: " << cascade_name << "\n"
                  << "Sugerencias:\n"
                  << "  - Instala los datos de OpenCV (suele traer los haarcascades)\n"
                  << "    sudo apt update && sudo apt install opencv-data\n"
                  << "  - O define la variable OPENCV_HAAR_DIR apuntando a tu carpeta haarcascades.\n";
        return 1;
    }

    cv::CascadeClassifier face_cascade;
    if (!face_cascade.load(cascade_path)) {
        std::cerr << "Encontre el cascade pero no pude cargarlo:\n  " << cascade_path << "\n";
        return 1;
    }

    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "No pude abrir la camara (index 0).\n";
        return 1;
    }

    std::cout << "Cascade: " << cascade_path << "\n"
              << "Presiona 's' para salir.\n";

    cv::Mat frame, gray;
    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(gray, gray);

        std::vector<cv::Rect> faces;
        face_cascade.detectMultiScale(
            gray, faces,
            1.1,        // scaleFactor
            5,          // minNeighbors
            0,          // flags
            cv::Size(60, 60) // minSize (ajusta si detecta demasiado o muy poco)
        );

        for (const auto& r : faces) {
            cv::rectangle(frame, r, cv::Scalar(0, 0, 255), 2); // rojo (BGR)
        }

        cv::imshow("Face Detection (Haar) - presiona 's' para salir", frame);

        int key = cv::waitKey(10);
        if (key == 's' || key == 'S') {
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
