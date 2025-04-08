void MainWindow::readDataFromDatabase() {
    /*
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL"); // O usa "QSQLITE" según tu caso
    db.setHostName("localhost");
    db.setDatabaseName("basedatos");
    db.setUserName("usuario");
    db.setPassword("contraseña");

    if (!db.open()) {
        qDebug() << "Error al conectar con la base de datos:" << db.lastError().text();
        return;
    }
    */

    // Consultar las últimas 20 temperaturas
    QMap<qint64, double> tempMap;
    QMap<qint64, double> humMap;

    QSqlQuery tempQuery("SELECT timestamp, value FROM ejemplo1 WHERE type='temperatura' ORDER BY timestamp DESC LIMIT 20");
    while (tempQuery.next()) {
        qint64 timestamp = tempQuery.value(0).toLongLong();
        double temp = tempQuery.value(1).toDouble();
        tempMap[timestamp] = temp;
    }

    QSqlQuery humQuery("SELECT timestamp, value FROM ejemplo1 WHERE type='humedad' ORDER BY timestamp DESC LIMIT 20");
    while (humQuery.next()) {
        qint64 timestamp = humQuery.value(0).toLongLong();
        double hum = humQuery.value(1).toDouble();
        humMap[timestamp] = hum;
    }

    // Obtener todos los timestamps únicos en orden
    QSet<qint64> allTimestamps = tempMap.keys().toSet().unite(humMap.keys().toSet());
    QList<qint64> sortedTimestamps = allTimestamps.values();
    std::sort(sortedTimestamps.begin(), sortedTimestamps.end());

    // Limpiar datos anteriores
    dataPointsTemp.clear();
    dataPointsHum.clear();

    for (qint64 ts : sortedTimestamps) {
        double temp = tempMap.contains(ts) ? tempMap[ts] : 0;
        double hum = humMap.contains(ts) ? humMap[ts] : 0;
        updateChart(temp, hum, ts);
    }

    db.close();
}
