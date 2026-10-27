# Curso de Qt Creator · Qt y MySQL (videos)

Material de la serie de videos «Qt y MySQL» (Ubuntu 24.04, Qt 6.12.0, MariaDB 10.11, phpMyAdmin 5.2). Una carpeta por episodio;
el código corresponde a lo que se ve en cada video.

| # | Carpeta | Tema |
|---|---|---|
| 1 | `01-driver-qmysql/` | Compilar el driver QMYSQL de Qt 6.12 con un script (`compilar-mysql-qt6.sh`, versión 2; cambia la línea 6 al actualizar Qt) y probarlo con un proyecto de consola |
| 2 | `02-phpmyadmin/` | Base `tienda` y tabla `productos` en phpMyAdmin; SQL usado en el video |
| 3 | `03-leer-tabla/` | Proyecto `inventario`: leer la tabla en un QTableWidget |
| 4 | `04-insertar/` | Insertar registros con prepare/bindValue |
| 5 | `05-actualizar/` | Actualizar la fila elegida (y reto: eliminar) |

Usuario de ejemplo del curso: `curso` / `curso1234` (`GRANT ALL PRIVILEGES ON tienda.* TO 'curso'@'localhost'`).
