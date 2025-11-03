#!/bin/bash

# ===============================
# CONFIGURACIÓN PRINCIPAL
# ===============================
QT_VERSION="6.9.2"   # <-- Cambia solo esta línea cuando actualices Qt
USER_HOME="/home/$USER"
QT_PATH="$USER_HOME/Qt/$QT_VERSION"
QT_BIN="$QT_PATH/gcc_64/bin"
QT_PLUGIN_PATH="$QT_PATH/gcc_64/plugins/sqldrivers"
QT_SRC_PATH="$QT_PATH/Src/qtbase/src/plugins/sqldrivers/mysql"

# ===============================
# INSTALACIÓN DE DEPENDENCIAS
# ===============================
echo "🔄 Actualizando lista de paquetes..."
sudo apt update -y && sudo apt upgrade -y

echo "🧹 Eliminando librerías previas de MySQL..."
sudo apt remove -y libmysqlclient-dev

echo "📦 Instalando dependencias necesarias..."
sudo apt install -y libmariadb-dev build-essential cmake plocate python3-pip

# ===============================
# CONFIGURAR ENTORNO DE QT
# ===============================
echo "⚙️ Configurando entorno de Qt $QT_VERSION..."
if [ -f "$QT_BIN/qt6-env.sh" ]; then
    source "$QT_BIN/qt6-env.sh"
else
    echo "Advertencia: No se encontró qt6-env.sh en $QT_BIN"
fi

# ===============================
# BUSCAR LIBRERÍA MARIADB
# ===============================
echo "🔍 Actualizando base de datos de plocate..."
sudo plocate -u

echo "🔍 Buscando libmariadb.so..."
MYSQL_LIB_PATH=$(plocate libmariadb.so | grep -m 1 "libmariadb.so")

if [ -z "$MYSQL_LIB_PATH" ]; then
    echo "❌ Error: No se encontró la librería libmariadb.so. Instala libmariadb-dev."
    exit 1
else
    echo "✅ Librería encontrada en: $MYSQL_LIB_PATH"
fi

# ===============================
# COMPILAR PLUGIN DE MYSQL
# ===============================
if [ -d "$QT_SRC_PATH" ]; then
    cd "$QT_SRC_PATH" || exit 1
else
    echo "❌ Error: No se encontró el directorio $QT_SRC_PATH"
    exit 1
fi

echo "🏗️ Configurando plugin con CMake..."
"$QT_BIN/qt-cmake" -DMySQL_INCLUDE_DIR=/usr/include/mariadb -DMySQL_LIBRARY="$MYSQL_LIB_PATH" .

echo "⚙️ Compilando plugin MySQL..."
make -j$(nproc) VERBOSE=1

# ===============================
# INSTALAR PLUGIN COMPILADO
# ===============================
if [ -f "libqsqlmysql.so" ]; then
    echo "📂 Copiando plugin a $QT_PLUGIN_PATH..."
    sudo cp libqsqlmysql.so "$QT_PLUGIN_PATH"
    echo "✅ Plugin instalado correctamente."
else
    echo "⚠️ No se encontró libqsqlmysql.so, buscando en el árbol de Qt..."
    ALT_PATH=$(find "$QT_PATH" -name "libqsqlmysql.so" 2>/dev/null | head -n 1)
    if [ -n "$ALT_PATH" ]; then
        echo "📍 Encontrado en $ALT_PATH, copiando..."
        sudo cp "$ALT_PATH" "$QT_PLUGIN_PATH"
        echo "✅ Plugin copiado correctamente."
    else
        echo "❌ No se encontró libqsqlmysql.so. Falló la compilación."
        exit 1
    fi
fi

# ===============================
# INSTALAR PYSIDE6 (OPCIONAL)
# ===============================
echo "🐍 Verificando PySide6..."
python3 -c "import PySide6" 2>/dev/null
if [ $? -ne 0 ]; then
    echo "📦 Instalando PySide6..."
    python3 -m pip install PySide6
fi

# ===============================
# VERIFICAR INSTALACIÓN
# ===============================
echo "🧪 Verificando instalación de QMYSQL..."
python3 - << END
from PySide6.QtCore import QCoreApplication
from PySide6.QtSql import QSqlDatabase
import sys

app = QCoreApplication(sys.argv)
if "QMYSQL" in QSqlDatabase.drivers():
    print("✅ QMYSQL driver disponible.")
else:
    print("❌ Error: QMYSQL no se encuentra instalado.")
END

echo "🎉 Instalación completada para Qt $QT_VERSION."
