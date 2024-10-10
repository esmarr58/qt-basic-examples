#include "mainwindow.h"
#include "ui_mainwindow.h"

//Paso # 3. Crear la funcion abrirImagen
void MainWindow::abrirImagen(){
    //Paso #5. Probar el funcionamiento de abrirImagen
    qDebug() << "se ejecuto la funcion abrirImagen ";

    //Punto extra 1pt final, 10min.
    //Agregar que se imprima la fecha del evento clicked en un qdebug.
    //PE-Paso#2.
    qDebug() << QDateTime::currentDateTime().toString();

    //Paso # 6. Guardar en una variable QString la ruta de la imagen que queremos Abrir
    QString rutaImagen = QFileDialog::getOpenFileName(
        this,
        tr("Abrir archivo de imagen"),
        "/home/r/Downloads",
        tr("Imagenes(*.png *.jpg)")
        );

    //get   -> Obtener
    //Open  -> Abrir
    //File  -> Archivo
    //Name  -> Nombre

    //Paso # 7. Visualizar que hay dentro de la variable rutaImagen
    qDebug() << rutaImagen;

    //Paso # 8. Verificar que la variable anterior , #6 tenga informacion
    if(rutaImagen.isEmpty()) return;

    //Paso # 9. Abrir la imagen
    QImage imagenQT(rutaImagen);

    //Paso # 10. Convertir la QImagen a un QPixmap
    QPixmap mapaPixeles = QPixmap::fromImage(imagenQT);

    //Paso # 11. Visualizar la imagen qlabel
    ui->label->setPixmap(mapaPixeles);
    //set   -> configurar/establecer
    //Pix   -> pixeles
    //map   -> mapa

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Paso # 4. Conectar la se;al clicked del boton con la funcion abrirImagen
    connect(ui->pushButton, SIGNAL(clicked()),this, SLOT(abrirImagen()));
}

MainWindow::~MainWindow()
{
    delete ui;
}
