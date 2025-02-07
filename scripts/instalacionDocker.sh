#!/bin/bash

# Script para instalar Docker en Ubuntu usando el método moderno y recomendado

# 1. Actualiza el sistema
echo "Actualizando el sistema..."
sudo apt update && sudo apt upgrade -y

# 2. Instala dependencias necesarias
echo "Instalando dependencias necesarias..."
sudo apt install -y apt-transport-https ca-certificates curl

# 3. Descarga y agrega la clave GPG de Docker
echo "Descargando y agregando la clave GPG de Docker..."
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg

# 4. Agrega el repositorio de Docker
echo "Agregando el repositorio de Docker..."
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

# 5. Instala Docker
echo "Instalando Docker..."
sudo apt update
sudo apt install -y docker-ce docker-ce-cli containerd.io

# 6. Verifica la instalación
echo "Verificando la instalación de Docker..."
sudo docker --version

# 7. Agrega el usuario actual al grupo Docker (para no usar sudo)
echo "Agregando el usuario actual al grupo Docker..."
sudo usermod -aG docker $USER
echo "Por favor, reinicia la computadora para que los cambios surtan efecto."

# 8. (Opcional) Configura Docker para que se inicie automáticamente
echo "Configurando Docker para que se inicie automáticamente..."
sudo systemctl enable docker

echo "¡Instalación de Docker completada con éxito!"
