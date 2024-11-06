#!/bin/bash

# Actualiza la lista de paquetes
sudo apt update -y

# Instala los paquetes necesarios
sudo apt install build-essential -y
sudo apt install libgl1-mesa-dev -y
sudo apt install libopencv-dev -y
sudo apt install libxcb-xinerama0 -y
sudo apt install nginx -y
sudo apt install net-tools -y
sudo apt install mariadb-server -y
sudo apt install php-fpm php-mysql -y
sudo apt install phpmyadmin -y
sudo apt install libmysqlclient-dev -y
sudo apt install libssl-dev -y
sudo apt install libxcb-cursor0 libxcb-cursor-dev -y
sudo apt install git wget flex bison gperf python3 python3-pip python3-venv -y
sudo apt install cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0 -y

# Instala el paquete Python con pip
pip install pyserial

# Mensaje de finalización
echo "Instalación completa de todos los paquetes."
