#include "mainwindow.h"
#include "ui_mainwindow.h"
bool habilitador = false;

void MainWindow::loop(){
    if(habilitador){
        //actualizarGrafico(true,1,1);
        if(serialActual == nullptr || !serialActual->isOpen()){
            QMessageBox::warning(this, "Error", "El puerto serial no esta abierto");
            return;
        }
        serialActual->write("a\n");
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Inicialización de variables privadas a nullptr
    ejeX = nullptr;
    ejeYTemperatura = nullptr;
    ejeYHumedad = nullptr;

    serieTemperatura = new QLineSeries();
    serieHumedad = new QLineSeries();    // Crear el objeto del gráfico y añadir la serie

    grafico = new QChart();

    // Añadir la serie al gráfico
    grafico->addSeries(serieTemperatura);
    grafico->addSeries(serieHumedad);


    grafico->setTitle("Proyecto Final - Diseño de Interfaces TSU-JABIL");
    // Establecer el título del gráfico

    vistaGrafico = ui->widget;
    vistaGrafico->setChart(grafico);
    vistaGrafico->setRenderHint(QPainter::Antialiasing);

    QTimer *cronos = new QTimer(this);
    connect(cronos, &QTimer::timeout, this, &MainWindow::loop);
    cronos->start(1000);

    manejarPuertosSeriales(ui->comboBox);
    vistaGrafico->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setupDatabase();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::actualizarGrafico(bool prueba, double temperatura, double humedad)
{
    // Verificar y reiniciar las series si tienen más de 1000 puntos
    if (serieTemperatura->count() >= 1000 || serieHumedad->count() >= 1000) {
        serieTemperatura->clear();
        serieHumedad->clear();
    }

    // Determinar los valores de temperatura y humedad
    double nuevoTemperatura;
    double nuevoHumedad;

    if (prueba) {
        // Generar valores aleatorios si prueba es true
        nuevoTemperatura = QRandomGenerator::global()->bounded(30);  // 0 a 30 grados
        nuevoHumedad = QRandomGenerator::global()->bounded(100);    // 0 a 100%
    } else {
        // Usar los valores proporcionados
        nuevoTemperatura = temperatura;
        nuevoHumedad = humedad;
    }

    // Determinar la posición X (índice actual de la serie)
    int x = serieTemperatura->count();

    // Agregar los nuevos puntos a las series
    serieTemperatura->append(x, nuevoTemperatura);
    serieHumedad->append(x, nuevoHumedad);

    // Crear y configurar los ejes si no están añadidos
    if (grafico->axes(Qt::Horizontal).isEmpty()) {
        ejeX = new QValueAxis;
        grafico->addAxis(ejeX, Qt::AlignBottom);
        serieTemperatura->attachAxis(ejeX);
        serieHumedad->attachAxis(ejeX);
    }

    if (!ejeYTemperatura) {
        ejeYTemperatura = new QValueAxis;
        ejeYTemperatura->setTitleText("Temperatura (°C)");
        grafico->addAxis(ejeYTemperatura, Qt::AlignRight);
        serieTemperatura->attachAxis(ejeYTemperatura);
    }

    if (!ejeYHumedad) {
        ejeYHumedad = new QValueAxis;
        ejeYHumedad->setTitleText("Humedad (%)");
        grafico->addAxis(ejeYHumedad, Qt::AlignLeft);
        serieHumedad->attachAxis(ejeYHumedad);
    }

    // Configurar los rangos de los ejes
    int visibleCount = 20;
    int minX = (x < visibleCount) ? 0 : x - visibleCount + 1;
    ejeX->setRange(minX, x);  // Mostrar solo los últimos 20 puntos
    ejeYTemperatura->setRange(0, 30);  // Rango para temperatura
    ejeYHumedad->setRange(0, 100);     // Rango para humedad

    // Configurar la leyenda para las series
    if (serieTemperatura->name().isEmpty()) {
        serieTemperatura->setName("Temperatura");
    }
    if (serieHumedad->name().isEmpty()) {
        serieHumedad->setName("Humedad");
    }

    grafico->legend()->setVisible(true); // Asegurar que la leyenda sea visible
    grafico->legend()->setAlignment(Qt::AlignBottom); // Ubicar la leyenda en la parte inferior
    // Actualizar o crear la etiqueta dinámica dentro de la gráfica
    QString textoEtiqueta = QString("Temperatura: %1 °C\nHumedad: %2 %")
                                .arg(nuevoTemperatura, 0, 'f', 2)
                                .arg(nuevoHumedad, 0, 'f', 2);

    if (!etiquetaDinamica) {
        etiquetaDinamica = new QGraphicsTextItem(textoEtiqueta);
        grafico->scene()->addItem(etiquetaDinamica);
        etiquetaDinamica->setPos(100, 55);  // Posición inicial dentro de la gráfica
        etiquetaDinamica->setDefaultTextColor(Qt::black);
        QFont font = etiquetaDinamica->font();
        font.setPointSize(10);
        etiquetaDinamica->setFont(font);
    } else {
        etiquetaDinamica->setPlainText(textoEtiqueta);
    }
    // Redibujar la vista del gráfico
    vistaGrafico->repaint();
}



void MainWindow::manejarPuertosSeriales(QComboBox *comboBox)
{
    // Limpiar cualquier elemento previo en el ComboBox
    comboBox->clear();
    // Añadir la opción por defecto "Seleccionar puerto"
    comboBox->addItem("Seleccionar puerto");
    // Obtener la lista de puertos seriales disponibles
    const QList<QSerialPortInfo> puertosDisponibles = QSerialPortInfo::availablePorts();
    // Verificar si se encontraron puertos seriales
    if (puertosDisponibles.isEmpty()) {
        QMessageBox::critical(this, "Error", "No se encontraron puertos seriales.");
    } else {
        // Agregar los puertos seriales al ComboBox con el nombre y la descripción
        for (const QSerialPortInfo &info : puertosDisponibles) {
            QString textoCombo = QString("%1 - %2")
            .arg(info.portName())
                .arg(info.description().isEmpty() ? "Sin descripción" : info.description());
            comboBox->addItem(textoCombo);
            qDebug() << "Puerto encontrado:" << info.portName();
            qDebug() << "Descripción:" << info.description();
        }
    }
    // Conectar el evento de selección del ComboBox para manejar la conexión y lectura de datos
    QObject::connect(comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, comboBox]() {
        // Verificar si el usuario ha seleccionado algo distinto de "Seleccionar puerto"
        if (comboBox->currentIndex() == 0) {
            QMessageBox::warning(this, "Advertencia", "Por favor selecciona un puerto.");
            return;
        }
        // Obtener el texto seleccionado del ComboBox
        QString seleccion = comboBox->currentText();
        // Separar el nombre del puerto de la descripción
        QStringList partes = seleccion.split(" - ");
        if (partes.size() < 1) {
            QMessageBox::critical(this, "Error", "Formato de selección no válido.");
            return;
        }
        QString nombrePuerto = partes[0]; // Esto obtiene "COM3"
        qDebug() << "Conectando al puerto:" << nombrePuerto;

        // Si hay una conexión activa en otro puerto, cerrarla primero
        if (serialActual != nullptr && serialActual->isOpen()) {
            qDebug() << "Desconectando del puerto" << serialActual->portName();
            serialActual->close();
            delete serialActual;
            serialActual = nullptr;
        }

        // Crear un nuevo objeto QSerialPort y configurar el puerto seleccionado
        serialActual = new QSerialPort();
        serialActual->setPortName(nombrePuerto);
        serialActual->setBaudRate(QSerialPort::Baud115200);
        serialActual->setDataBits(QSerialPort::Data8);
        serialActual->setParity(QSerialPort::NoParity);
        serialActual->setStopBits(QSerialPort::OneStop);
        serialActual->setFlowControl(QSerialPort::NoFlowControl);

        // Intentar abrir el puerto
        if (!serialActual->open(QIODevice::ReadWrite)) {
            QMessageBox::critical(this, "Error", "No se pudo abrir el puerto serial: " +
                                                     serialActual->errorString());
            delete serialActual;
            serialActual = nullptr;
            return;
        }
        QMessageBox::information(this, "Conexión exitosa", "Conexión establecida con éxito en el puerto " +
                                                                                                                     nombrePuerto);
        qDebug() << "Conexión establecida con éxito en el puerto" << nombrePuerto;
        habilitador = true;
        // Conectar la señal readyRead del puerto serial para leer los datos
        QObject::connect(serialActual, &QSerialPort::readyRead, this, [this]() {
            // Verificar que el puerto serial esté abierto
            if (serialActual == nullptr || !serialActual->isOpen()) {
                QMessageBox::warning(this, "Error", "El puerto serial no está abierto.");
                return;
            }
            // Mientras haya una línea completa disponible en el puerto serial
            while (serialActual->canReadLine()) {
                // Leer la línea completa del puerto serial
                QByteArray datosRecibidos = serialActual->readLine().trimmed();
                QString textoRecibido = QString::fromUtf8(datosRecibidos);

                // Depuración en consola
                qDebug() << "Línea completa recibida:" << textoRecibido;

                // Intentar decodificar el mensaje JSON
                QJsonParseError parseError;
                QJsonDocument doc = QJsonDocument::fromJson(datosRecibidos, &parseError);

                if (parseError.error != QJsonParseError::NoError) {
                    qDebug() << "Error al analizar JSON:" << parseError.errorString();
                    continue; // Pasar a la siguiente línea si el JSON no es válido
                }

                if (!doc.isObject()) {
                    qDebug() << "El mensaje JSON no es un objeto.";
                    continue; // Pasar a la siguiente línea si no es un objeto JSON
                }

                // Extraer los valores de "temp" y "humedad"
                QJsonObject jsonObject = doc.object();
                if (jsonObject.contains("temp") && jsonObject.contains("humedad")) {
                    double temp = jsonObject.value("temp").toDouble();
                    double humedad = jsonObject.value("humedad").toDouble();

                    // Mostrar los valores decodificados en la consola
                    qDebug() << "Temperatura:" << temp;
                    qDebug() << "Humedad:" << humedad;
                    actualizarGrafico(false,temp, humedad);
                    saveSensorData(temp,humedad);
                } else {
                    qDebug() << "El JSON no contiene las claves 'temp' o 'humedad'.";
                }
            }

        });
    });
}

void MainWindow::setupDatabase() {
    // Configurar la conexión con MySQL
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost");
    db.setDatabaseName("ejemplo1"); // Cambiar por el nombre de tu base de datos
    db.setUserName("admin"); // Cambiar por tu usuario
    db.setPassword("hola1234"); // Cambiar por tu contraseña

    if (!db.open()) {
        qCritical() << "Error al conectar a la base de datos:" << db.lastError().text();
        return;
    }

    // Verificar si la tabla ya existe
    QSqlQuery query(db);
    QString tableName = "datos_Sensores22";

    if (query.exec(QString("SHOW TABLES LIKE '%1'").arg(tableName)) && query.next()) {
        qDebug() << "La tabla ya existe. No se realizará ninguna acción.";
    } else {
        // Crear la tabla
        QString createTableQuery = QString(
                                       "CREATE TABLE %1 ("
                                       "id INT AUTO_INCREMENT PRIMARY KEY, "
                                       "epoch INT NOT NULL, "
                                       "temperatura FLOAT NOT NULL, "
                                       "humedad FLOAT NOT NULL"
                                       ")"
                                       ).arg(tableName);

        if (query.exec(createTableQuery)) {
            qDebug() << "Tabla creada exitosamente.";
        } else {
            qCritical() << "Error al crear la tabla:" << query.lastError().text();
        }
    }

    db.close();
}

void MainWindow::saveSensorData(float temperatura, float humedad) {
    // Configurar la conexión con MySQL
    QSqlDatabase db = QSqlDatabase::database(); // Usar la conexión existente
    if (!db.isOpen()) {
        qCritical() << "La base de datos no está abierta.";
        return;
    }

    // Obtener el epoch actual
    int epoch = QDateTime::currentDateTime().toSecsSinceEpoch();

    // Insertar los datos en la tabla
    QSqlQuery query(db);
    QString tableName = "datos_Sensores22";

    // Crear la consulta SQL para insertar los datos
    QString insertQuery = QString(
                              "INSERT INTO %1 (epoch, temperatura, humedad) "
                              "VALUES (:epoch, :temperatura, :humedad)"
                              ).arg(tableName);

    query.prepare(insertQuery);
    query.bindValue(":epoch", epoch);
    query.bindValue(":temperatura", temperatura);
    query.bindValue(":humedad", humedad);

    if (query.exec()) {
        qDebug() << "Datos insertados correctamente en la base de datos.";
    } else {
        qCritical() << "Error al insertar los datos:" << query.lastError().text();
    }
}


