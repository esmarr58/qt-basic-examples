# Qt y MySQL #1 · Compila el driver QMYSQL de Qt 6.12 en Ubuntu con un script

Material del video. Probado en Ubuntu 24.04 con Qt 6.12.0 (instalado en `~/Qt` con el componente **Sources**) y MariaDB 10.11.

## Archivos

- `compilar-mysql-qt6.sh` — versión 2 del script (2026-10-27). **Línea 6:** `QT_VERSION="6.12.0"`; cámbiala si tu Qt es otro
  (por ejemplo `6.11.1`). Es la única línea que depende de tu versión.
- `prueba_qmysql/` — proyecto de consola de Qt Creator (CMake) que imprime los drivers disponibles y prueba la conexión.
  - `main.cpp` — el programa del video.
  - `variantes/main_reto_version.cpp` — solución del reto (`SELECT VERSION()`).
  - `variantes/main_password_mal.cpp` — para ver el error «Access denied».

## Pasos

1. Comprueba las fuentes de Qt: `ls ~/Qt/6.12.0/Src` (si no existen, MaintenanceTool → Add or remove components → cuenta de
   Qt → Qt 6.12.0 → Sources).
2. Ejecuta el script: `bash compilar-mysql-qt6.sh` (pide tu contraseña para `apt`). Al final debe decir
   `✅ QMYSQL disponible` y `🎉 Driver MySQL listo para Qt 6.12.0`.
3. Abre `prueba_qmysql` en Qt Creator (File → Open Project… → `CMakeLists.txt`), cambia usuario y contraseña por los de tu
   servidor y ejecuta con Ctrl+R. Salida esperada:

```
Drivers: QSQLITE, QPSQL, QMIMER, QIBASE, QODBC, QOCI, QMARIADB, QMYSQL
Conectado al servidor MySQL.
```

## Si algo falla

- `Driver not loaded` → el plugin no carga su biblioteca: vuelve a correr el script y revisa `ldd
  ~/Qt/6.12.0/gcc_64/plugins/sqldrivers/libqsqlmysql.so`.
- `Access denied for user …` → el driver ya funciona; revisa usuario y contraseña.
- `Can't connect to local server …` → el servidor MySQL/MariaDB no está corriendo (`sudo systemctl start mariadb`).

El usuario `curso` / `curso1234` del video se creó solo para el curso:
`CREATE USER 'curso'@'localhost' IDENTIFIED BY 'curso1234'; GRANT ALL PRIVILEGES ON tienda.* TO 'curso'@'localhost';`
