#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    // Variables de estado de la calculadora
    double acumulador = 0.0;        // Guarda el resultado parcial
    QString operacionPendiente;     // "+", "-", "*", "÷", "%"
    bool nuevoNumero = true;        // Indica si inicia un nuevo número

    // Funciones auxiliares
    void agregarDigito(const QString& digito);
    void agregarPunto();
    void limpiarTodo1();
    void borrarUltimo();
    void establecerOperacion(const QString& op);
    void calcularResultado();
    double valorActual() const;
    void mostrarEnPantalla(const QString& texto);

};
#endif // MAINWINDOW_H
