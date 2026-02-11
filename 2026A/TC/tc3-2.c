#include <opencv2/opencv.hpp>
#include <filesystem>
#include <iostream>
#include <vector>
#include <cstdlib>

namespace fs = std::filesystem;

static std::string find_cascade_file(const std::string& filename)
{
    if (const char* env = std::getenv("OPENCV_HAAR_DIR")) {
        fs::path p = fs::path(env) / filename;
        if (fs::exists(p)) return p.string();
    }

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

    const std::vector<fs::path> roots = {"/usr/share", "/usr/local/share"};
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
            // Ignorar carpetas sin permisos
        }
    }

    return "";
}

static bool load_cascade(cv::CascadeClassifier& c, const std::string& fname)
{
    const std::string path = find_cascade_file(fname);
    if (path.empty()) {
        std::cerr << "No encontre: " << fname << "\n";
        return false;
    }
    if (!c.load(path)) {
        std::cerr << "Encontre pero no pude cargar: " << path << "\n";
        return false;
    }
    std::cout << "OK cascade: " << fname << " -> " << path << "\n";
    return true;
}

int main()
{
    cv::CascadeClassifier face_cascade, eyes_cascade, mouth_cascade;

    // Cara
    if (!load_cascade(face_cascade, "haarcascade_frontalface_default.xml")) {
        std::cerr << "Sugerencia: sudo apt install opencv-data\n";
        return 1;
    }

    // Ojos: puedes usar cualquiera de estos (depende qué tengas instalado)
    // - haarcascade_eye.xml
    // - haarcascade_eye_tree_eyeglasses.xml (a veces funciona mejor con lentes)
    if (!load_cascade(eyes_cascade, "haarcascade_eye_tree_eyeglasses.xml")) {
        // fallback
        if (!load_cascade(eyes_cascade, "haarcascade_eye.xml")) {
            std::cerr << "No pude cargar un cascade de ojos.\n";
            return 1;
        }
    }

    // Boca: normalmente viene como "haarcascade_smile.xml" (sonrisas), o "haarcascade_mcs_mouth.xml"
    // Probamos primero mcs_mouth (si existe), si no, smile como fallback.
    bool mouth_ok = load_cascade(mouth_cascade, "haarcascade_mcs_mouth.xml");
    if (!mouth_ok) {
        mouth_ok = load_cascade(mouth_cascade, "haarcascade_smile.xml");
    }
    if (!mouth_ok) {
        std::cerr << "No pude cargar cascade de boca (mcs_mouth o smile).\n"
                  << "Sugerencia: sudo apt install opencv-data\n";
        return 1;
    }

    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "No pude abrir la camara (index 0).\n";
        return 1;
    }

    std::cout << "Presiona 's' para salir.\n";

    cv::Mat frame, gray;
    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(gray, gray);

        // 1) Detectar caras
        std::vector<cv::Rect> faces;
        face_cascade.detectMultiScale(
            gray, faces,
            1.1, 5, 0,
            cv::Size(80, 80)
        );

        // Dibujar y detectar partes dentro de cada cara
        for (const auto& f : faces) {
            // Cara (rojo)
            cv::rectangle(frame, f, cv::Scalar(0, 0, 255), 2);

            cv::Mat faceROI = gray(f);

            // 2) Detectar ojos dentro de la cara (mejor en mitad superior)
            cv::Rect upperFace(0, 0, f.width, f.height / 2);
            cv::Mat upperROI = faceROI(upperFace);

            std::vector<cv::Rect> eyes;
            eyes_cascade.detectMultiScale(
                upperROI, eyes,
                1.1, 5, 0,
                cv::Size(20, 20)
            );

            // Elegir hasta 2 ojos: izquierda y derecha por coordenada x
            // (si detecta más, tomamos los 2 más “grandes” y luego ordenamos por x)
            if (eyes.size() > 2) {
                std::sort(eyes.begin(), eyes.end(),
                          [](const cv::Rect& a, const cv::Rect& b) {
                              return a.area() > b.area();
                          });
                eyes.resize(2);
            }
            std::sort(eyes.begin(), eyes.end(),
                      [](const cv::Rect& a, const cv::Rect& b) {
                          return a.x < b.x;
                      });

            for (const auto& e : eyes) {
                // Ojo (rojo), ojo está en upperROI, hay que sumar offsets:
                cv::Rect eAbs(f.x + e.x, f.y + e.y, e.width, e.height);
                cv::rectangle(frame, eAbs, cv::Scalar(0, 0, 255), 2);
            }

            // 3) Detectar “boca” dentro de la cara (mejor en parte inferior)
            // Tomamos la mitad inferior (o 55%-100% para evitar nariz)
            int y0 = static_cast<int>(f.height * 0.55);
            int h  = f.height - y0;
            if (h > 0) {
                cv::Rect lowerFace(0, y0, f.width, h);
                cv::Mat lowerROI = faceROI(lowerFace);

                std::vector<cv::Rect> mouths;
                mouth_cascade.detectMultiScale(
                    lowerROI, mouths,
                    1.1, 8, 0,
                    cv::Size(30, 20)
                );

                // Si hay varias detecciones, tomar la más grande (suele ser la más razonable)
                if (!mouths.empty()) {
                    auto best = *std::max_element(
                        mouths.begin(), mouths.end(),
                        [](const cv::Rect& a, const cv::Rect& b) {
                            return a.area() < b.area();
                        }
                    );

                    cv::Rect mAbs(
                        f.x + best.x,
                        f.y + y0 + best.y,
                        best.width,
                        best.height
                    );
                    cv::rectangle(frame, mAbs, cv::Scalar(0, 0, 255), 2);
                }
            }
        }

        cv::imshow("Cara + Ojos + Boca (Haar) | 's' para salir", frame);

        int key = cv::waitKey(10);
        if (key == 's' || key == 'S') break;
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}
