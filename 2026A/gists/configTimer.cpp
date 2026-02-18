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
