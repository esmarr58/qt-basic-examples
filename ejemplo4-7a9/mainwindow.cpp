#include "mainwindow.h"
#include "ui_mainwindow.h"
//Paso # 3. Crear funcion presionarBoton
void MainWindow::presionarBoton(){
    //Paso # 6. Probar la funcion presionar.
    //Paso # PE-2
    qDebug() << "El boton se presiono" << QDateTime::currentDateTime().toString();

    //Punto extra. 10min.
    //Agregar al mensaje del paso # 6. La fecha actual.


    //Paso # 7. Obtener el nombre de una imagen, tambien donde esta ubicada
    QString rutaImagen = QFileDialog::getOpenFileName(
        this,
        tr("Abrir Archivo"),
        "/home/r/Downloads",
        tr("Imagenes(*.jpg)")
        );
    //Get -> obtener
    //Open -> Abrir
    //File -> Archivo
    //Name -> Nombre

    //Paso # 8. Probar que contiene la variable rutaImagen;
    qDebug() << rutaImagen;

    //Paso # 9. Verificar si rutaImagen contiene informacion
    if(rutaImagen.isEmpty()) return;

    //Paso # 10. Abrir la imagen
    QImage imagen(rutaImagen);

    //Paso # 11. Convertir la imagen, a un mapa de pixeles,
    QPixmap mapaPixeles = QPixmap::fromImage(imagen);

    //Paso # 12. Visualizar el mapa en la etiqueta (label)
    ui->label->setPixmap(mapaPixeles);
    //set   -> establece/configura
    //Pix   -> pixeles
    //map   -> mapa



}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //Paso # 4. Conectar la se;al clicked del boton con la funcion del paso # 3.
    connect(ui->pushButton, SIGNAL(clicked()),this, SLOT(presionarBoton()));
}

MainWindow::~MainWindow()
{
    delete ui;
}
