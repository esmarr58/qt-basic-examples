#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->label->setPixmap(QString(":/indicador.png"));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_horizontalSlider_sliderMoved(int position)
{
    QPixmap mapaPixeles(":/indicador.png");
    QTransform transformacion;
    transformacion.rotate(position);
    QPixmap imagenRotada = mapaPixeles.transformed(transformacion);
    ui->label->setPixmap(imagenRotada);
    ui->label->setAlignment(Qt::AlignCenter);

}

