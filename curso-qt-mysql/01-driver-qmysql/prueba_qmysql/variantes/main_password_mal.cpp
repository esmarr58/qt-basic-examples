#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlError>
#include <QTextStream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTextStream salida(stdout);

    salida << "Drivers: " << QSqlDatabase::drivers().join(", ") << Qt::endl;

    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setUserName("curso");
    db.setPassword("otra");

    if (db.open()) {
        salida << "Conectado al servidor MySQL." << Qt::endl;
    } else {
        salida << "Error: " << db.lastError().text() << Qt::endl;
    }
    return 0;
}
