#!/bin/bash

# Script para instalar Docker en Ubuntu y ejecutar DeepSeek localmente

# 1. Actualiza el sistema
echo "Actualizando el sistema..."
sudo apt update && sudo apt upgrade -y

# 2. Instala dependencias necesarias
echo "Instalando dependencias necesarias..."
sudo apt install -y apt-transport-https ca-certificates curl software-properties-common

# 3. Agrega la clave GPG oficial de Docker
echo "Agregando la clave GPG oficial de Docker..."
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg

# 4. Agrega el repositorio de Docker a las fuentes de APT
echo "Agregando el repositorio de Docker a las fuentes de APT..."
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null

# 5. Instala Docker
echo "Instalando Docker..."
sudo apt update
sudo apt install -y docker-ce docker-ce-cli containerd.io

# 6. Verifica la instalación
echo "Verificando la instalación de Docker..."
sudo docker --version

# 7. Ejecuta Docker sin `sudo` (opcional)
echo "Agregando el usuario actual al grupo Docker..."
sudo usermod -aG docker $USER
echo "Por favor, cierra la sesión y vuelve a iniciarla para que los cambios surtan efecto."

# 8. Descargar y ejecutar DeepSeek
echo "Descargando y ejecutando DeepSeek..."
docker pull deepseek/deepseek:latest
docker run -it deepseek/deepseek:latest

# 9. Verifica que DeepSeek esté corriendo
echo "Verificando que DeepSeek esté corriendo..."
# Aquí puedes agregar comandos adicionales para verificar que DeepSeek esté funcionando correctamente

# 10. (Opcional) Configura Docker para que se inicie automáticamente
echo "Configurando Docker para que se inicie automáticamente..."
sudo systemctl enable docker

echo "¡Instalación y configuración completadas!"
