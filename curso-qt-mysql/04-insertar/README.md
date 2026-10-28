# Qt y MySQL #4 · Inserta registros desde Qt con prepare y bindValue

Proyecto `inventario` al final del video 4 (parte del final del video 3), Qt 6.12.0 en Ubuntu 24.04.

- `inventario/` — formulario (Nombre, Precio, Existencia, Categoría, Guardar) y el slot `on_botonGuardar_clicked()` con
  `prepare` / `bindValue` / `exec` / `lastInsertId`.
- `inventario/variantes/`:
  - `mainwindow_texto_pegado.cpp`: el INSERT armado con `+` (no lo uses): con «Lápiz D'Angelo» falla con el error 1064 y abre la
    puerta a la inyección SQL.
  - `mainwindow_reto_precio.cpp`: reto, no guardar con precio 0.

Prueba del video: Regla · 15 · 25 · Papelería → «Guardado con id 5.» y la fila aparece también en phpMyAdmin.
