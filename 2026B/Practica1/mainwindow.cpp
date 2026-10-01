#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setFocusPolicy(Qt::StrongFocus);
    if (centralWidget()) centralWidget()->setFocusPolicy(Qt::StrongFocus);

    socket.open(QUrl("ws://192.168.1.101:81"));
    connect(&socket, &QWebSocket::connected, this, &MainWindow::alConectar);
    connect(&socket, &QWebSocket::disconnected, this, &MainWindow::alDesconectar);
    connect(&socket, &QWebSocket::textMessageReceived, this, &MainWindow::alRecibirMensaje);


    QTimer *cronometro = new QTimer(this);
    connect(cronometro, &QTimer::timeout, this, &MainWindow::loop);
    cronometro->start(1000);



}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loop(){
    /*
    static int contador = 0;
    ui->statusbar->showMessage(QString::number(contador));
    contador = contador+1;
    */
    if(socket.state() != QAbstractSocket::ConnectedState) return;
    socket.sendTextMessage("{\"tipo\":\"latido\"}");

}

void MainWindow::alConectar(){
    ui->statusbar->showMessage("Conectado");


}
void MainWindow::alDesconectar(){
    ui->statusbar->showMessage("Desconectado");

}
void MainWindow::enviarJSON(const QString &mensaje){
    if(socket.state() != QAbstractSocket::ConnectedState)
        return;
    socket.sendTextMessage(mensaje);

}

void MainWindow::alRecibirMensaje(const QString &mensaje){
    ui->statusbar->showMessage(mensaje);
}


void MainWindow::on_pushButton_3_clicked()
{
    if(socket.state() != QAbstractSocket::ConnectedState) return;
    socket.sendTextMessage("{\"tipo\":\"mover\", \"accion\":\"retroceder\", \"ms\": 100}");
}







void MainWindow::on_pushButton_4_clicked()
{

    if(socket.state() != QAbstractSocket::ConnectedState) return;
    socket.sendTextMessage("{\"tipo\":\"mover\", \"accion\":\"avanzar\", \"ms\": 100}");
}


void MainWindow::on_pushButton_2_clicked()
{

    if(socket.state() != QAbstractSocket::ConnectedState) return;
    socket.sendTextMessage("{\"tipo\":\"mover\", \"accion\":\"izquierda\", \"ms\": 100}");
}

void MainWindow::on_pushButton_clicked()
{

    if(socket.state() != QAbstractSocket::ConnectedState) return;
    socket.sendTextMessage("{\"tipo\":\"mover\", \"accion\":\"derecha\", \"ms\": 100}");
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
    case Qt::Key_Up:
        on_pushButton_3_clicked();

        break;
    case Qt::Key_Down:
        on_pushButton_4_clicked();

        break;
    case Qt::Key_Left:
        on_pushButton_clicked();

        break;
    case Qt::Key_Right:
        on_pushButton_2_clicked();

        break;
    default:
        QWidget::keyPressEvent(event); // Pass other keys to base class
        break;
    }
}


