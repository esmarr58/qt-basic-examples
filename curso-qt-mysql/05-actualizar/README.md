# Qt y MySQL #5 · Actualiza una fila de MySQL desde Qt

Proyecto `inventario` al final del video 5 (parte del final del video 4), Qt 6.12.0 en Ubuntu 24.04.

- `inventario/` — fila «Id:» con `etiquetaIdValor`, botón Actualizar, `int idSeleccionado = -1` en `mainwindow.h`,
  `on_tablaProductos_cellClicked(int, int)` (pasa la fila al formulario) y `on_botonActualizar_clicked()` (`UPDATE … WHERE id = :id`,
  `numRowsAffected()`).
- `inventario/variantes/reto_eliminar/` — reto: botón Eliminar con `DELETE FROM productos WHERE id = :id`.

Prueba del video: Cargar → clic en Mochila → precio 399 → Actualizar → «Filas actualizadas: 1.»; otra vez sin cambios → «0.».
