#!/usr/bin/env bash
set -euo pipefail

# ============================================================
# Ubuntu 24.04 (Noble) - Dev tools + ESP-IDF deps + Web stack
# + Fix phpMyAdmin configuration storage (quita el warning)
#
# Uso:
#   chmod +x instalacion.sh
#   sudo ./instalacion.sh
#
# Opcional (NO instalar stack web):
#   sudo INSTALL_WEB=0 ./instalacion.sh
# ============================================================

if [[ "${EUID}" -ne 0 ]]; then
  echo "Este script debe ejecutarse como root. Usa: sudo ./instalacion.sh"
  exit 1
fi

INSTALL_WEB="${INSTALL_WEB:-1}"
export DEBIAN_FRONTEND=noninteractive

echo "==> Detectando Ubuntu..."
. /etc/os-release
echo "    $PRETTY_NAME"

echo "==> Actualizando repos y sistema..."
apt-get update -y
apt-get upgrade -y

echo "==> Instalando paquetes base (dev, Qt/XCB, OpenCV, redes, ESP-IDF deps)..."
apt-get install -y \
  build-essential \
  git wget curl ca-certificates \
  net-tools \
  cmake ninja-build ccache \
  flex bison gperf \
  python3 python3-pip python3-venv python3-serial \
  libffi-dev libssl-dev \
  dfu-util libusb-1.0-0 \
  libgl1-mesa-dev \
  libopencv-dev \
  libxcb-xinerama0 \
  libxcb-cursor0 libxcb-cursor-dev \
  libmysqlclient-dev \
  openssl \
  libfuse2

echo "==> Paquetes base OK."

# ------------------------------------------------------------------
# Web stack (opcional)
# ------------------------------------------------------------------
if [[ "$INSTALL_WEB" == "1" ]]; then
  echo "==> Instalando Nginx + MariaDB + PHP-FPM + phpMyAdmin..."

  apt-get install -y software-properties-common
  add-apt-repository -y universe || true
  apt-get update -y

  # Instalación
  apt-get install -y \
    nginx \
    mariadb-server \
    php-fpm php-mysql \
    phpmyadmin

  echo "==> Detectando version de PHP instalada..."
  PHP_VERSION="$(php -r "echo PHP_MAJOR_VERSION.'.'.PHP_MINOR_VERSION;" 2>/dev/null || true)"
  if [[ -z "${PHP_VERSION}" ]]; then
    PHP_VERSION="8.3"
  fi
  echo "    PHP_VERSION=${PHP_VERSION}"

  PHP_FPM_SOCK="/run/php/php${PHP_VERSION}-fpm.sock"
  if [[ ! -S "$PHP_FPM_SOCK" ]]; then
    echo "⚠️  No se encontró socket en $PHP_FPM_SOCK. Buscando sockets disponibles..."
    ls -la /run/php || true
    FOUND_SOCK="$(ls -1 /run/php/php*-fpm.sock 2>/dev/null | head -n 1 || true)"
    if [[ -n "$FOUND_SOCK" ]]; then
      PHP_FPM_SOCK="$FOUND_SOCK"
      echo "    Usando socket detectado: $PHP_FPM_SOCK"
    else
      echo "❌ No pude detectar el socket de php-fpm en /run/php. Revisa php-fpm."
      exit 1
    fi
  fi

  echo "==> Habilitando servicios..."
  systemctl enable --now mariadb
  systemctl enable --now nginx
  systemctl enable --now "php${PHP_VERSION}-fpm" || true

  # --------------------------
  # Config Nginx
  # --------------------------
  echo "==> Configurando Nginx + snippet de phpMyAdmin..."
  NGINX_DEFAULT="/etc/nginx/sites-available/default"
  MARKER="### HETPRO-PHPMYADMIN-AUTO ###"

  if [[ -f "$NGINX_DEFAULT" && ! -f "${NGINX_DEFAULT}.backup" ]]; then
    cp "$NGINX_DEFAULT" "${NGINX_DEFAULT}.backup"
    echo "    Backup creado: ${NGINX_DEFAULT}.backup"
  fi

  # Solo reescribimos si no existe el marker
  if ! grep -qF "$MARKER" "$NGINX_DEFAULT" 2>/dev/null; then
    cat > "$NGINX_DEFAULT" <<EOL
$MARKER
##
# Default server configuration
##
server {
    listen 80 default_server;
    listen [::]:80 default_server;

    include snippets/phpmyadmin.conf;

    root /var/www/html;

    index index.html index.htm index.nginx-debian.html index.php;

    server_name _;

    location / {
        try_files \$uri \$uri/ =404;
    }

    location ~ \.php$ {
        include snippets/fastcgi-php.conf;
        fastcgi_pass unix:${PHP_FPM_SOCK};
    }
}
EOL
    echo "    Nginx default configurado (con marker)."
  else
    echo "    Marker detectado, NO reescribo $NGINX_DEFAULT."
  fi

  mkdir -p /etc/nginx/snippets
  cat > /etc/nginx/snippets/phpmyadmin.conf <<EOL
# HETPRO phpMyAdmin snippet
location /phpmyadmin {
    root /usr/share/;
    index index.php index.html index.htm;

    location ~ ^/phpmyadmin/(.+\.php)\$ {
        try_files \$uri =404;
        root /usr/share/;
        fastcgi_pass unix:${PHP_FPM_SOCK};
        fastcgi_index index.php;
        fastcgi_param SCRIPT_FILENAME \$document_root\$fastcgi_script_name;
        include /etc/nginx/fastcgi_params;
    }

    location ~* ^/phpmyadmin/(.+\.(jpg|jpeg|gif|css|png|js|ico|html|xml|txt))\$ {
        root /usr/share/;
    }
}
EOL

  echo "==> Creando /var/www/html/info.php (si no existe)..."
  if [[ ! -f /var/www/html/info.php ]]; then
    echo "<?php phpinfo(); ?>" > /var/www/html/info.php
    chown www-data:www-data /var/www/html/info.php
  fi

  # --------------------------
  # Fix phpMyAdmin config storage (quita el warning)
  # --------------------------
  echo "==> Corrigiendo 'phpMyAdmin configuration storage' (pmadb)..."

  # Verifica si ya existe una tabla típica del config storage:
  TABLE_EXISTS="$(mysql -Nse "SELECT COUNT(*) FROM information_schema.tables WHERE table_schema='phpmyadmin' AND table_name='pma__bookmark';" 2>/dev/null || echo "0")"

  if [[ "${TABLE_EXISTS}" == "0" ]]; then
    CREATE_SQL="/usr/share/phpmyadmin/sql/create_tables.sql"
    if [[ -f "$CREATE_SQL" ]]; then
      echo "    Creando tablas internas (una sola vez)..."
      mysql < "$CREATE_SQL"
    else
      echo "⚠️  No encontré $CREATE_SQL. Saltando creación de tablas internas."
    fi
  else
    echo "    Tablas internas ya existen. OK."
  fi

  # Crear usuario control 'pma' (si no existe) y permisos
  PMA_PASS_FILE="/root/.pma_controlpass"
  if [[ -f "$PMA_PASS_FILE" ]]; then
    PMA_PASS="$(cat "$PMA_PASS_FILE")"
  else
    PMA_PASS="$(openssl rand -base64 24 | tr -d '\n' | tr -d '=' | head -c 32)"
    echo "$PMA_PASS" > "$PMA_PASS_FILE"
    chmod 600 "$PMA_PASS_FILE"
  fi

  mysql -e "CREATE USER IF NOT EXISTS 'pma'@'localhost' IDENTIFIED BY '${PMA_PASS}';" || true
  mysql -e "GRANT SELECT, INSERT, UPDATE, DELETE ON phpmyadmin.* TO 'pma'@'localhost';" || true
  mysql -e "FLUSH PRIVILEGES;" || true

  # Agregar config en /etc/phpmyadmin/conf.d para no tocar el archivo principal
  mkdir -p /etc/phpmyadmin/conf.d
  CONF_D="/etc/phpmyadmin/conf.d/99-hetpro-pma-storage.php"

  if [[ ! -f "$CONF_D" ]]; then
    cat > "$CONF_D" <<EOL
<?php
/**
 * HETPRO: phpMyAdmin configuration storage
 * Este archivo se genera automáticamente.
 */
\$i = 1;
\$cfg['Servers'][\$i]['pmadb'] = 'phpmyadmin';
\$cfg['Servers'][\$i]['controluser'] = 'pma';
\$cfg['Servers'][\$i]['controlpass'] = '${PMA_PASS}';
EOL
    chmod 640 "$CONF_D"
    chown root:www-data "$CONF_D" || true
    echo "    Config storage habilitado: $CONF_D"
  else
    echo "    Config storage ya estaba configurado: $CONF_D"
  fi

  # --------------------------
  # MariaDB admin user (opcional)
  # --------------------------
  echo "==> Usuario MariaDB 'admin' (opcional)"
  echo "    Si NO quieres cambiar/crear el usuario admin, deja la contraseña vacía y Enter."
  read -r -s -p "Introduce contraseña para 'admin' (vacío = no tocar): " PASSWORD1; echo
  if [[ -n "${PASSWORD1}" ]]; then
    read -r -s -p "Confirma la contraseña: " PASSWORD2; echo
    if [[ "$PASSWORD1" != "$PASSWORD2" ]]; then
      echo "❌ Las contraseñas no coinciden."
      exit 1
    fi

    USER_EXISTS="$(mysql -Nse "SELECT EXISTS(SELECT 1 FROM mysql.user WHERE user='admin' AND host='localhost');" 2>/dev/null || echo "0")"
    if [[ "${USER_EXISTS}" == "1" ]]; then
      mysql -e "ALTER USER 'admin'@'localhost' IDENTIFIED BY '${PASSWORD1}';"
      echo "    ✔ Contraseña de 'admin' actualizada."
    else
      mysql -e "CREATE USER 'admin'@'localhost' IDENTIFIED BY '${PASSWORD1}';"
      mysql -e "GRANT ALL PRIVILEGES ON *.* TO 'admin'@'localhost' WITH GRANT OPTION;"
      echo "    ✔ Usuario 'admin' creado con privilegios."
    fi
    mysql -e "FLUSH PRIVILEGES;"
  else
    echo "    OK, no se modificó/creó usuario 'admin'."
  fi

  echo "==> Probando configuración Nginx..."
  nginx -t

  echo "==> Reiniciando servicios..."
  systemctl restart "php${PHP_VERSION}-fpm" || true
  systemctl restart nginx
  systemctl restart mariadb || true

  echo "✅ Web stack listo."
  echo "   - Prueba PHP:  http://localhost/info.php"
  echo "   - phpMyAdmin:  http://localhost/phpmyadmin"
  echo "   - Nota: el password del usuario 'pma' quedó guardado en: /root/.pma_controlpass"

else
  echo "==> INSTALL_WEB=0 -> Omitiendo instalación/configuración de servidor web."
fi

echo "✅ Instalación completa."
