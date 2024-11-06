#!/bin/bash

# Define variables
QT_VERSION="6.8.0"
QT_PATH="/home/$USER/Qt/$QT_VERSION"
PLUGIN_PATH="$QT_PATH/gcc_64/plugins/sqldrivers"
SOURCE_PATH="$QT_PATH/Src/qtbase/src/plugins/sqldrivers/mysql"

# Update and install essential packages
echo "Updating package list..."
sudo apt update

# Remove MySQL libraries if installed
echo "Removing MySQL library (libmysqlclient-dev) if it exists..."
sudo apt remove -y libmysqlclient-dev

# Install MariaDB library
echo "Installing MariaDB library (libmariadb-dev)..."
sudo apt install -y libmariadb-dev

# Install other dependencies
echo "Installing remaining dependencies..."
sudo apt install -y build-essential cmake plocate python3-pip

# Load Qt environment
echo "Configuring Qt environment..."
source "$QT_PATH/gcc_64/bin/qt6-env.sh"

# Update locate database and search for libmariadb.so
echo "Updating locate database..."
sudo plocate -u

echo "Searching for MariaDB library..."
MYSQL_LIB_PATH=$(plocate libmariadb.so | grep -m 1 "libmariadb.so")

if [ -z "$MYSQL_LIB_PATH" ]; then
    echo "Error: MariaDB library not found. Ensure the client libraries are installed."
    exit 1
else
    echo "Library found at: $MYSQL_LIB_PATH"
    MYSQL_LIB_DIR=$(dirname "$MYSQL_LIB_PATH")
fi

# Navigate to MySQL plugin source directory
if [ -d "$SOURCE_PATH" ]; then
    cd "$SOURCE_PATH"
else
    echo "Error: MySQL plugin source directory not found at $SOURCE_PATH"
    exit 1
fi

# Configure the MySQL plugin with CMake and provide MariaDB paths
echo "Configuring the MySQL plugin with CMake..."
/home/$USER/Qt/$QT_VERSION/gcc_64/bin/qt-cmake -DMySQL_INCLUDE_DIR=/usr/include/mariadb -DMySQL_LIBRARY="$MYSQL_LIB_PATH" .

# Compile the plugin with verbose output
echo "Compiling the MySQL plugin with verbose output..."
make VERBOSE=1

# Check if the plugin was created and install it
if [ -f "libqsqlmysql.so" ]; then
    echo "Installing MySQL plugin to $PLUGIN_PATH..."
    sudo cp libqsqlmysql.so "$PLUGIN_PATH"
    echo "MySQL plugin installed successfully."
else
    echo "Error: libqsqlmysql.so was not created."
    
    # Search for any generated file in the build tree as a backup check
    echo "Attempting to locate libqsqlmysql.so within the Qt directory..."
    LIB_PATH_FOUND=$(find "$QT_PATH" -name "libqsqlmysql.so")
    
    if [ -n "$LIB_PATH_FOUND" ]; then
        echo "libqsqlmysql.so found at an alternative location: $LIB_PATH_FOUND"
        sudo cp "$LIB_PATH_FOUND" "$PLUGIN_PATH"
        echo "Plugin copied to plugins folder."
    else
        echo "libqsqlmysql.so not found in any Qt directories. Exiting."
        exit 1
    fi
fi

# Install PySide6 if not installed
echo "Checking for PySide6 module in Python..."
python3 -c "import PySide6" 2>/dev/null
if [ $? -ne 0 ]; then
    echo "PySide6 not found. Installing PySide6..."
    python3 -m pip install PySide6
fi

# Verify installation with QCoreApplication to avoid QSqlDatabase error
echo "Verifying MySQL plugin installation..."
python3 - << END
from PySide6.QtCore import QCoreApplication
from PySide6.QtSql import QSqlDatabase
import sys

app = QCoreApplication(sys.argv)
if "QMYSQL" in QSqlDatabase.drivers():
    print("QMYSQL driver is successfully installed and available.")
else:
    print("QMYSQL driver installation failed.")
END

echo "Installation script completed."
