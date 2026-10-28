# Qt y MySQL #3 · Lee una tabla de MySQL en un QTableWidget desde cero

Proyecto `inventario` (Qt Widgets, CMake) tal como queda al final del video, compilado con Qt 6.12.0 en Ubuntu 24.04.

- `inventario/` — `CMakeLists.txt` (con el módulo `Sql`), `main.cpp`, `mainwindow.h`, `mainwindow.cpp`, `mainwindow.ui`.
- `inventario/variantes/` — lo que se muestra como error o como reto:
  - `CMakeLists_sin_sql.txt`: sin `Sql` → `fatal error: QSqlDatabase: No such file or directory`.
  - `mainwindow_password_mal.cpp`: «Sin conexión: Access denied for user …».
  - `mainwindow_tabla_mal.cpp`: «Error: Table 'tienda.producto' doesn't exist».
  - `mainwindow_reto_where.cpp`: reto, `SELECT … WHERE existencia < 50`.

Requisitos: el driver QMYSQL del video 1 y la base `tienda` del video 2. Cambia usuario y contraseña en `mainwindow.cpp` por
los de tu servidor. Abre `CMakeLists.txt` en Qt Creator (File → Open Project…), kit Desktop Qt 6.12.0, Ctrl+R y pulsa
«Cargar productos».
