#!/bin/bash

# ===============================
# CONFIGURACIÓN PRINCIPAL
# ===============================
QT_VERSION="6.12.0"   # <-- Cambia solo esta línea cuando actualices Qt

# compilar-mysql-qt6.sh · versión 2 (2026-10-27) para Qt 6.12 en Ubuntu 24.04.
# Compila el driver QMYSQL de Qt 6 (plugin libqsqlmysql.so) con la biblioteca de MariaDB
# y lo instala en tu Qt. Requisitos: Qt 6 en ~/Qt instalado con el componente «Sources»,
# permisos de sudo e internet. Uso:  bash compilar-mysql-qt6.sh
# Versión 1 (Qt 6.9.2): github.com/esmarr58/qt-basic-examples/blob/main/scripts/compilar-mysq-qt6.sh

QT_PATH="$HOME/Qt/$QT_VERSION"
QT_BIN="$QT_PATH/gcc_64/bin"
QT_PLUGIN_PATH="$QT_PATH/gcc_64/plugins/sqldrivers"
QT_SRC_PATH="$QT_PATH/Src/qtbase/src/plugins/sqldrivers"
BUILD_DIR="$HOME/build-sqldrivers-qt$QT_VERSION"

# ===============================
# COMPROBAR QT Y SUS FUENTES
# ===============================
if [ ! -x "$QT_BIN/qt-cmake" ]; then
    echo "❌ No existe $QT_BIN/qt-cmake. Revisa QT_VERSION (línea 6) y tu instalación de Qt."
    exit 1
fi
if [ ! -f "$QT_SRC_PATH/CMakeLists.txt" ]; then
    echo "❌ No están las fuentes de Qt en $QT_SRC_PATH."
    echo "   Abre ~/Qt/MaintenanceTool → Add or remove components → Qt $QT_VERSION → marca «Sources»."
    exit 1
fi
echo "✅ Qt $QT_VERSION con fuentes en $QT_PATH"

# ===============================
# INSTALAR DEPENDENCIAS
# ===============================
echo "🔄 Actualizando la lista de paquetes..."
sudo apt update -y

if dpkg -s libmysqlclient-dev >/dev/null 2>&1; then
    echo "🧹 Quitando libmysqlclient-dev (usaremos la biblioteca de MariaDB)..."
    sudo apt remove -y libmysqlclient-dev
fi

echo "📦 Instalando libmariadb-dev, build-essential, cmake y ninja-build..."
sudo apt install -y libmariadb-dev build-essential cmake ninja-build

# ===============================
# LOCALIZAR LA BIBLIOTECA DE MARIADB
# ===============================
MYSQL_INCLUDE_DIR="/usr/include/mariadb"
MYSQL_LIB_PATH="/usr/lib/x86_64-linux-gnu/libmariadb.so"
if [ ! -f "$MYSQL_LIB_PATH" ]; then
    MYSQL_LIB_PATH=$(find /usr/lib -name "libmariadb.so" 2>/dev/null | head -n 1)
fi
if [ -z "$MYSQL_LIB_PATH" ] || [ ! -f "$MYSQL_INCLUDE_DIR/mysql.h" ]; then
    echo "❌ No se encontró libmariadb.so o $MYSQL_INCLUDE_DIR/mysql.h. Instala libmariadb-dev."
    exit 1
fi
echo "✅ Biblioteca: $MYSQL_LIB_PATH"
echo "✅ Cabeceras:  $MYSQL_INCLUDE_DIR"

# ===============================
# COMPILAR EL PLUGIN QMYSQL
# ===============================
echo "🏗️ Configurando con qt-cmake en $BUILD_DIR..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || exit 1
"$QT_BIN/qt-cmake" -G Ninja "$QT_SRC_PATH" \
    -DMySQL_INCLUDE_DIR="$MYSQL_INCLUDE_DIR" \
    -DMySQL_LIBRARY="$MYSQL_LIB_PATH" \
    -DCMAKE_INSTALL_PREFIX="$QT_PATH/gcc_64" || { echo "❌ Falló la configuración."; exit 1; }

echo "⚙️ Compilando el plugin MySQL..."
cmake --build . --target QMYSQLDriverPlugin || { echo "❌ Falló la compilación."; exit 1; }

# ===============================
# INSTALAR EL PLUGIN COMPILADO
# ===============================
PLUGIN=$(find "$BUILD_DIR" -name "libqsqlmysql.so" | head -n 1)
if [ -z "$PLUGIN" ]; then
    echo "❌ No se encontró libqsqlmysql.so en $BUILD_DIR."
    exit 1
fi
if [ -f "$QT_PLUGIN_PATH/libqsqlmysql.so" ]; then
    echo "📂 Respaldando el plugin anterior como libqsqlmysql.so.anterior"
    mv "$QT_PLUGIN_PATH/libqsqlmysql.so" "$QT_PLUGIN_PATH/libqsqlmysql.so.anterior"
fi
echo "📂 Copiando $PLUGIN a $QT_PLUGIN_PATH..."
cp "$PLUGIN" "$QT_PLUGIN_PATH/" || exit 1
echo "✅ Plugin instalado."

# ===============================
# VERIFICAR LA INSTALACIÓN
# ===============================
echo "🧪 Verificando que Qt $QT_VERSION cargue el driver QMYSQL..."
PRUEBA="$BUILD_DIR/prueba_qmysql"
mkdir -p "$PRUEBA" && cd "$PRUEBA" || exit 1
cat > main.cpp << 'FIN'
#include <QCoreApplication>
#include <QSqlDatabase>
#include <QTextStream>
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);
    out << "Drivers SQL disponibles: " << QSqlDatabase::drivers().join(", ") << Qt::endl;
    return QSqlDatabase::isDriverAvailable("QMYSQL") ? 0 : 1;
}
FIN
cat > CMakeLists.txt << 'FIN'
cmake_minimum_required(VERSION 3.16)
project(prueba_qmysql LANGUAGES CXX)
find_package(Qt6 REQUIRED COMPONENTS Core Sql)
add_executable(prueba_qmysql main.cpp)
target_link_libraries(prueba_qmysql PRIVATE Qt6::Core Qt6::Sql)
FIN
"$QT_BIN/qt-cmake" -G Ninja . > /dev/null && cmake --build . > /dev/null || { echo "❌ No compiló la prueba."; exit 1; }
if ./prueba_qmysql; then
    echo "✅ QMYSQL disponible. Biblioteca enlazada:"
    ldd "$QT_PLUGIN_PATH/libqsqlmysql.so" | grep -i maria
    echo "🎉 Driver MySQL listo para Qt $QT_VERSION."
else
    echo "❌ Qt no carga QMYSQL. Revisa con: QT_DEBUG_PLUGINS=1 ./prueba_qmysql"
    exit 1
fi
