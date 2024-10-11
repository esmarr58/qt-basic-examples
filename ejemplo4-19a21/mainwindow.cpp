#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //Paso # 3. Enlazar la se;al clic del boton con la funcion buscarImagen
    connect(ui->pushButton, SIGNAL(clicked()),this, SLOT(buscarImagen()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

//Paso # 2. Crear la funcion buscarImagen
void MainWindow::buscarImagen(){
    //Paso # 5. Comprobar que la funcion corre al presionar el boton
    qDebug() << "Esta funcionando!";
    //Punto Extra 10min.
    //Quiero que se imprima en un qDebug la fecha y hora del sistema.
    //PE - Paso # 2.
    qDebug() << QDateTime::currentDateTime().toString("'Hoy es el dia:'dddd HH:mm:ss");

    //Paso # 6. Obtener la ruta, el nombre y la extension de la imagen.
    QString rutaImagen = QFileDialog::getOpenFileName(
            this,
        tr("Abrir imagenes"),
        "/home/r/Downloads",
        tr("Filtrar imagenes(*.jpg *.png)")
        );
    //get       -> obtener
    //Open      -> abrir
    //File      -> archivo
    //Name      -> nombre

    //Paso # 7 - Visualizar el contenido de la variable
    qDebug() << rutaImagen;

    //Paso # 8 - Verificar que el archivo este vacio
    if(rutaImagen.isEmpty()) return;

    //Paso # 9 - Abrimos la imagen
    QImage imagenQT(rutaImagen);

    //Paso # 10 - Convertir QImage -> QPixmap
    QPixmap mapaPixeles = QPixmap::fromImage(imagenQT);

    //Paso # 11 - Visualizar el resultado
    ui->label->setPixmap(mapaPixeles);





}
