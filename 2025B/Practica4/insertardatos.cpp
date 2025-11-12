#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QTimer>
#include <QRandomGenerator>
#include <QDateTime>
#include <QDebug>

// =================== CONFIGURACIÓN ===================
// Si quieres fijarlo desde el .pro:  DEFINES += INSERT_INTERVAL_MS=2000
#ifndef INSERT_INTERVAL_MS
#define INSERT_INTERVAL_MS 2000   // intervalo default para insertar datos simulados
#endif

// Admin (capaz de crear BD/usuario). Puedes usar el mismo usuario si no separas roles.
static const QString ADMIN_HOST = "localhost";
static const QString ADMIN_USER = "admin";
static const QString ADMIN_PASS = "hola1234";

// Nombre de la BD que usará la app
static const QString DB_NAME    = "interfaces2025b";

// Usuario de aplicación (con permisos sobre DB_NAME)
static const QString APP_USER   = "app_user";
static const QString APP_PASS   = "app_password_seguro";

// Nombres de conexiones
static const char* CONN_ADMIN = "conexion_admin";
static const char* CONN_APP   = "conexion_app";
// =====================================================

static bool execOrWarn(QSqlQuery& q, const QString& sql) {
    if (!q.exec(sql)) {
        qWarning() << "❌ Error SQL:" << q.lastError().text() << "en:" << sql;
        return false;
    }
    return true;
}

static bool openConnection(const char* connName,
                           const QString& host,
                           const QString& user,
                           const QString& pass,
                           const QString& dbName = QString())
{
    if (QSqlDatabase::contains(connName)) {
        QSqlDatabase::removeDatabase(connName);
    }
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", connName);
    db.setHostName(host);
    db.setUserName(user);
    db.setPassword(pass);
    if (!dbName.isEmpty()) db.setDatabaseName(dbName);

    if (!db.open()) {
        qWarning() << "❌ No se pudo abrir la conexión" << connName << ":" << db.lastError().text();
        return false;
    }
    return true;
}

static bool ensureDbAndUser() {
    // 1) Conectar como ADMIN (sin seleccionar BD)
    if (!openConnection(CONN_ADMIN, ADMIN_HOST, ADMIN_USER, ADMIN_PASS)) {
        qWarning() << "❌ Error al conectar como admin.";
        return false;
    }
    QSqlDatabase dba = QSqlDatabase::database(CONN_ADMIN);
    QSqlQuery qa(dba);

    // 2) Crear BD si no existe
    if (!execOrWarn(qa, QString("CREATE DATABASE IF NOT EXISTS `%1`").arg(DB_NAME))) {
        dba.close();
        QSqlDatabase::removeDatabase(CONN_ADMIN);
        return false;
    }

    // 3) Crear usuario de aplicación (si no existe) y darle permisos sobre la BD
    if (!execOrWarn(qa, QString("CREATE USER IF NOT EXISTS '%1'@'localhost' IDENTIFIED BY '%2'")
                            .arg(APP_USER, APP_PASS))) {
        // Si tu MySQL no soporta IF NOT EXISTS, puedes ignorar el error si el usuario ya existe.
    }

    if (!execOrWarn(qa, QString("GRANT ALL PRIVILEGES ON `%1`.* TO '%2'@'localhost'")
                            .arg(DB_NAME, APP_USER))) {
        dba.close();
        QSqlDatabase::removeDatabase(CONN_ADMIN);
        return false;
    }

    execOrWarn(qa, "FLUSH PRIVILEGES");

    // 4) Cerrar admin y abrir conexión APP a la BD
    dba.close();
    QSqlDatabase::removeDatabase(CONN_ADMIN);

    if (!openConnection(CONN_APP, ADMIN_HOST, APP_USER, APP_PASS, DB_NAME)) {
        qWarning() << "❌ No se pudo abrir la base de datos como APP_USER.";
        return false;
    }

    // 5) Crear tabla si no existe
    QSqlDatabase db2 = QSqlDatabase::database(CONN_APP);
    QSqlQuery q2(db2);

    const QString crearTabla =
        "CREATE TABLE IF NOT EXISTS datos ("
        "id INT AUTO_INCREMENT PRIMARY KEY, "
        "timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
        "dato FLOAT, "
        "tipoDato VARCHAR(50), "
        "sensorID INT"
        ");";

    if (!execOrWarn(q2, crearTabla)) {
        db2.close();
        QSqlDatabase::removeDatabase(CONN_APP);
        return false;
    }

    qDebug() << "✅ Base de datos y tabla listas con usuario de aplicación.";
    return true;
}

static bool insertRandomRow() {
    if (!QSqlDatabase::contains(CONN_APP)) {
        qWarning() << "❌ Conexión APP no existe.";
        return false;
    }
    QSqlDatabase db = QSqlDatabase::database(CONN_APP);
    if (!db.isOpen()) {
        qWarning() << "❌ Conexión APP está cerrada.";
        return false;
    }

    // ✅ Usar generateDouble() para obtener número flotante [0, 100)
    const double dato = QRandomGenerator::global()->generateDouble() * 100.0;

    static const QStringList tipos = {"temperatura", "humedad", "voltaje"};
    const int tiposCount = static_cast<int>(tipos.size());
    const QString tipoDato = tipos.at(QRandomGenerator::global()->bounded(tiposCount));

    const int sensorID = QRandomGenerator::global()->bounded(1, 6);

    QSqlQuery q(db);
    q.prepare("INSERT INTO datos (dato, tipoDato, sensorID) VALUES (:dato, :tipo, :sid)");
    q.bindValue(":dato", dato);
    q.bindValue(":tipo", tipoDato);
    q.bindValue(":sid", sensorID);

    if (!q.exec()) {
        qWarning() << "❌ Error al insertar:" << q.lastError().text();
        return false;
    }

    qDebug().noquote() << QString("➕ Insertado: dato=%1, tipo=%2, sensorID=%3 @ %4")
                              .arg(dato, 0, 'f', 3)
                              .arg(tipoDato)
                              .arg(sensorID)
                              .arg(QDateTime::currentDateTime().toString(Qt::ISODate));
    return true;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    if (!QSqlDatabase::isDriverAvailable("QMYSQL")) {
        qCritical() << "❌ Driver QMYSQL no disponible. Instala el plugin de MySQL para Qt.";
        return 1;
    }

    if (!ensureDbAndUser()) {
        qCritical() << "❌ No se pudo preparar la base de datos.";
        return 1;
    }

    QTimer *timer = new QTimer(&app);
    QObject::connect(timer, &QTimer::timeout, &app, [](){
        insertRandomRow();
    });
    timer->start(INSERT_INTERVAL_MS);

    qInfo() << "⏱️  Insertando datos cada" << INSERT_INTERVAL_MS << "ms. Ctrl+C para salir.";

    QObject::connect(&app, &QCoreApplication::aboutToQuit, [](){
        if (QSqlDatabase::contains(CONN_APP)) {
            QSqlDatabase db = QSqlDatabase::database(CONN_APP);
            if (db.isOpen()) db.close();
            QSqlDatabase::removeDatabase(CONN_APP);
        }
        qInfo() << "👋 Cerrando...";
    });

    return app.exec();
}
