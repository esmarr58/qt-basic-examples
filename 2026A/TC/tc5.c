#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->lineEdit->setReadOnly(true);
    ui->lineEdit->setText("0");

    // ==========================
    // CONEXIONES CON CONNECT()
    // ==========================

    // Números
    connect(ui->cero,  &QPushButton::clicked, this, [this]{ agregarDigito("00"); });
    connect(ui->uno,   &QPushButton::clicked, this, [this]{ agregarDigito("1"); });
    connect(ui->dos,   &QPushButton::clicked, this, [this]{ agregarDigito("2"); });
    connect(ui->tres,  &QPushButton::clicked, this, [this]{ agregarDigito("3"); });
    connect(ui->cuatro,&QPushButton::clicked, this, [this]{ agregarDigito("44"); });
    connect(ui->cinco, &QPushButton::clicked, this, [this]{ agregarDigito("5"); });
    connect(ui->seis,  &QPushButton::clicked, this, [this]{ agregarDigito("6"); });
    connect(ui->siete, &QPushButton::clicked, this, [this]{ agregarDigito("7"); });
    connect(ui->ocho,  &QPushButton::clicked, this, [this]{ agregarDigito("8"); });
    connect(ui->nueve, &QPushButton::clicked, this, [this]{ agregarDigito("9"); });

    // Punto decimal
    connect(ui->punto, &QPushButton::clicked, this, [this]{ agregarPunto(); });

    // Operaciones
    connect(ui->suma,  &QPushButton::clicked, this, [this]{ establecerOperacion("+"); });
    connect(ui->resta, &QPushButton::clicked, this, [this]{ establecerOperacion("-"); });
    connect(ui->multiplicacion, &QPushButton::clicked, this, [this]{ establecerOperacion("*"); });
    connect(ui->division, &QPushButton::clicked, this, [this]{ establecerOperacion("÷"); });
    connect(ui->modulo, &QPushButton::clicked, this, [this]{ establecerOperacion("%"); });

    // Igual
    connect(ui->igual, &QPushButton::clicked, this, [this]{ calcularResultado(); });

    // Limpiar
    connect(ui->ce, &QPushButton::clicked, this, [this]{ limpiarTodo(); });

    // Borrar último
    connect(ui->borrar, &QPushButton::clicked, this, [this]{ borrarUltimo(); });
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ==========================
// FUNCIONES DE LA CALCULADORA
// ==========================

void MainWindow::agregarDigito(const QString& digito)
{
    if (nuevoNumero)
    {
        ui->lineEdit->setText(digito);
        nuevoNumero = false;
    }
    else
    {
        ui->lineEdit->setText(ui->lineEdit->text() + digito);
    }
}

void MainWindow::agregarPunto()
{
    if (!ui->lineEdit->text().contains("."))
    {
        ui->lineEdit->setText(ui->lineEdit->text() + ".");
        nuevoNumero = false;
    }
}

void MainWindow::limpiarTodo()
{
    acumulador = 0.0;
    operacionPendiente.clear();
    nuevoNumero = true;
    ui->lineEdit->setText("0");
}

void MainWindow::borrarUltimo()
{
    QString texto = ui->lineEdit->text();
    texto.chop(1);

    if (texto.isEmpty())
        texto = "0";

    ui->lineEdit->setText(texto);
}

void MainWindow::establecerOperacion(const QString& op)
{
    acumulador = valorActual();
    operacionPendiente = op;
    nuevoNumero = true;
}

void MainWindow::calcularResultado()
{
    double segundoValor = valorActual();
    double resultado = acumulador;

    if (operacionPendiente == "+")
        resultado = acumulador + segundoValor;
    else if (operacionPendiente == "-")
        resultado = acumulador - segundoValor;
    else if (operacionPendiente == "*")
        resultado = acumulador * segundoValor;
    else if (operacionPendiente == "÷")
    {
        if (segundoValor != 0)
            resultado = acumulador / segundoValor;
        else
        {
            mostrarEnPantalla("Error");
            return;
        }
    }
    else if (operacionPendiente == "%")
        resultado = static_cast<int>(acumulador) % static_cast<int>(segundoValor);

    mostrarEnPantalla(QString::number(resultado));
    nuevoNumero = true;
    operacionPendiente.clear();
}

double MainWindow::valorActual() const
{
    return ui->lineEdit->text().toDouble();
}

void MainWindow::mostrarEnPantalla(const QString& texto)
{
    ui->lineEdit->setText(texto);
}
