# Qt y MySQL #2 · Crea tu base de datos y tu tabla en phpMyAdmin

Material del video. phpMyAdmin 5.2 sobre MariaDB 10.11 (Ubuntu 24.04), en `http://localhost/phpmyadmin`, con el usuario del
curso `curso` / `curso1234` (`GRANT ALL PRIVILEGES ON tienda.* TO 'curso'@'localhost'`).

- `tienda.sql` — todo lo que se hace con clics en el video, como SQL equivalente (base, tabla, filas, columna `categoria`,
  UPDATE/SELECT con WHERE, comparación FLOAT/DECIMAL y el reto con la columna `codigo`).

Estado final de la tabla `productos` (es el punto de partida de los videos 3, 4 y 5):

| id | nombre | precio | existencia | categoria | codigo |
|---|---|---|---|---|---|
| 1 | Lápiz | 5.50 | 100 | Papelería | NULL |
| 2 | Cuaderno | 32.00 | 40 | Papelería | NULL |
| 3 | Mochila | 450.00 | 8 | Accesorios | 7501234567890 |
| 4 | Pluma | 12.90 | 60 | Papelería | NULL |

Para empezar de cero: `DROP DATABASE tienda;` y ejecutar `tienda.sql` en la pestaña SQL.

## Punto de partida automático: `preparar-base.sh`

Si no quieres repetir los clics del video (o quieres reiniciar la base), este script crea el usuario `curso`, la base `tienda`,
la tabla `productos` y **100 productos** de ejemplo (los cuatro primeros son los de la tabla de arriba; el resto tiene
`categoria` y un `codigo` de 13 dígitos). Es el punto de partida común de los videos 3, 4 y 5.

```bash
bash preparar-base.sh      # pide tu contraseña de sudo (usa «sudo mysql»); MariaDB o MySQL en la misma máquina
```

Aviso: si la tabla `productos` ya existe, la borra y la vuelve a crear. Los 100 INSERT están también en `productos_100.sql`
(sirve para pegarlo en la pestaña SQL de phpMyAdmin).
