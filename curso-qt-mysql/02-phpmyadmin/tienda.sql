-- Qt y MySQL #2 · Lo que se hace en phpMyAdmin durante el video, como SQL equivalente (MariaDB 10.11 / MySQL 8).
-- El video lo hace con clics; este archivo sirve para reproducirlo o repararlo desde la pestaña SQL.

CREATE DATABASE tienda CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci;
USE tienda;

CREATE TABLE productos (
  id INT NOT NULL AUTO_INCREMENT,
  nombre VARCHAR(50) NOT NULL,
  precio DECIMAL(8,2) NOT NULL,
  existencia INT NOT NULL,
  PRIMARY KEY (id)
) ENGINE = InnoDB;

-- Dos filas con el formulario «Insertar» (phpMyAdmin manda NULL en id para que el AUTO_INCREMENT lo asigne)
INSERT INTO productos (id, nombre, precio, existencia) VALUES (NULL, 'Lápiz', '5.50', '100'), (NULL, 'Cuaderno', '32.00', '40');

-- Ejercicio: dos filas desde la pestaña SQL
INSERT INTO productos (nombre, precio, existencia) VALUES ('Mochila', 450.00, 8), ('Pluma', 12.90, 60);

-- Estructura → Agregar 1 columna después de existencia
ALTER TABLE productos ADD categoria VARCHAR(30) NOT NULL AFTER existencia;

-- UPDATE sin WHERE (phpMyAdmin pide confirmación: cambia todas las filas) y con WHERE
UPDATE productos SET categoria = 'Papelería';
UPDATE productos SET categoria = 'Accesorios' WHERE id = 3;

-- SELECT con WHERE
SELECT nombre, precio FROM productos WHERE precio > 20;

-- FLOAT frente a DECIMAL (para dinero, DECIMAL)
SELECT CAST(12.90 AS FLOAT) * 100, 12.90 * 100;

-- Reto: columna codigo que admite NULL y el código solo para la Mochila
ALTER TABLE productos ADD codigo VARCHAR(13) NULL AFTER categoria;
UPDATE productos SET codigo = '7501234567890' WHERE id = 3;
