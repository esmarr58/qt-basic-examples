#include "mainwindow.h"
#include "ui_mainwindow.h"

//Paso # 3. Crear la funcion abrirImagen
void MainWindow::abrirImagen(){
    //Paso # 5. Mandar mensaje a la consola
    //Si el mensaje aparece, significa que la funcion se ejecuta
    qDebug() << "Se ejecuto la funcion abrir Imagen";

    //Punto Extra, 10min.
    //Se envie la fecha y hora en un qdebug
    //PE - Paso # 2.
    qDebug() << QDateTime::currentDateTime().toString();

    //Paso # 6. Crear variable QString para guardar la ruta del archivo a abrir
    QString rutaImagen = QFileDialog::getOpenFileName(
            this,
        tr("Abrir imagenes"),
        "/home/r/Downloads",
        tr("Filtro de imagenes(*.jpg *.png *.jpeg)")
        );
    //get   ->  obtener
    //Open  ->  abrir
    //File  ->  archivo
    //Name  ->  Nombre
    //tr    ->  try ->  intentar

    //Paso # 6.2 no puedo abrir una imagen cuya ruta ""
    if(rutaImagen.isEmpty()) return;


    //Paso # 7. Ver el contenido de la variable anterior
    qDebug() << rutaImagen;

    //Paso # 8. Abrir la imagen
    QImage imagenQT(rutaImagen);

    //Paso # 9. Convertir QImage -> QPixmap
    QPixmap mapaPixeles = QPixmap::fromImage(imagenQT);

    //Paso # 10. Visualizar la imagen
    ui->label->setPixmap(mapaPixeles);
    //set   ->  configurar/establece
    //Pix   ->  pixeles
    //map   ->  mapa



}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //Paso # 4. Conectar la se;al clicked del boton, con la funcion abrirImagen
    connect(ui->pushButton, SIGNAL(clicked()),this, SLOT(abrirImagen()));
}

MainWindow::~MainWindow()
{
    delete ui;
}
