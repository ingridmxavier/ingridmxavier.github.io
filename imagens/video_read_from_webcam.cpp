#include <iostream>
#include <sstream>
#include <ctime>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

string timestamp_filename() {
    time_t now = time(nullptr);
    tm t;
#ifdef _WIN32
    localtime_s(&t, &now);
#else
    localtime_r(&now, &t);
#endif
    char buf[64];
    strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &t);
    return string(buf);
}

int main()
{
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Erro: nao foi possivel abrir a webcam." << endl;
        return -1;
    }

    namedWindow("Filtragem Webcam", WINDOW_AUTOSIZE);

    int kernel_size = 5;
    bool usarFiltroMelhor = true;

    string nomeMelhor = "Bilateral";
    string nomePior   = "Media";

    cout << "Janela aberta. Clique na janela de imagem para dar foco." << endl;
    cout << "Teclas: [s] salvar frame, [f] alternar filtro, [q] sair (ou ESC)." << endl;

    Mat frame, filtered;
    while (true)
    {
        if (!cap.read(frame)) {
            cerr << "Erro ao capturar frame." << endl;
            break;
        }

        // aplica filtro atual
        if (usarFiltroMelhor) {
            int d = kernel_size; 
            double sigmaColor = kernel_size * 2.0;
            double sigmaSpace = kernel_size / 2.0 + 1.0;
            bilateralFilter(frame, filtered, d, sigmaColor, sigmaSpace);
        } else {
            // média (blur)
            blur(frame, filtered, Size(kernel_size, kernel_size));
        }

        // desenha uma caixa de texto para identificar o filtro
        string texto = "Filtro: " + (usarFiltroMelhor ? nomeMelhor : nomePior) + " (k=" + to_string(kernel_size) + ")";
        int font = FONT_HERSHEY_SIMPLEX;
        double fontScale = 0.8;
        int thickness = 2;
        int baseline = 0;
        Size textSize = getTextSize(texto, font, fontScale, thickness, &baseline);
        Point origin(10, 20 + textSize.height);

        // retangulo de fundo
        rectangle(filtered, Point(origin.x - 6, origin.y - textSize.height - 6),
                         Point(origin.x + textSize.width + 6, origin.y + 6),
                         Scalar(0, 0, 0), FILLED);
        // texto em branco
        putText(filtered, texto, origin, font, fontScale, Scalar(255, 255, 255), thickness, LINE_AA);

        imshow("Filtragem Webcam", filtered);

        // pega tecla (usa &0xFF para portabilidade)
        int rawKey = waitKey(30);
        if (rawKey == -1) continue; // nenhuma tecla pressionada

        int key = rawKey & 0xFF; // seguro para comparar ASCII
        char c = static_cast<char>(key);
        char cl = static_cast<char>(tolower((unsigned char)c));

        if (key == 27 || cl == 'q') { // ESC ou q
            cout << "Saindo..." << endl;
            break;
        } else if (cl == 's') {
            string fname = (usarFiltroMelhor ? "frame_melhor_" : "frame_pior_") + timestamp_filename() + ".jpg";
            if (imwrite(fname, filtered)) {
                cout << "Imagem salva: " << fname << endl;
            } else {
                cerr << "Falha ao salvar a imagem." << endl;
            }
        } else if (cl == 'f') {
            usarFiltroMelhor = !usarFiltroMelhor;
            cout << "Alternou filtro: agora = " << (usarFiltroMelhor ? nomeMelhor : nomePior) << endl;
        }
    }

    cap.release();
    destroyAllWindows();
    return 0;
}

