# Clase 16 — Graficar en Qt con contratos JSON básicos

**Diseño de Interfaces (I7262)** — Sesión 16 (Semana 9).
La ESP32-S3 manda **una línea JSON por muestra** por el puerto serie y Qt la grafica con **Qt Charts**.
No se necesita ningún sensor: el firmware **simula** temperatura, humedad y luz.

## Contenido

```
Clase16-Graficar-JSON/
├── firmware_contratos/        # Firmware Arduino (ESP32-S3): contratos v1, v2 y v3
│   └── firmware_contratos.ino
├── qt_ej1_un_valor/           # Ejemplo 1: contrato v1 -> una curva
├── qt_ej2_dos_valores/        # Ejemplo 2: contrato v2 -> dos curvas, doble eje Y
├── diagrama-bloques.pdf       # El camino del dato, de la ESP32 a la gráfica
└── README.md
```

## Los contratos

Un **contrato** es el acuerdo de qué manda la ESP32 y qué espera Qt. Una línea por muestra, terminada en `\n`:

| Contrato | Línea que viaja por el cable | Lo usa |
|---|---|---|
| v1 | `{"t":24.80}` | `qt_ej1_un_valor` |
| v2 | `{"t":24.80,"h":55.30}` | `qt_ej2_dos_valores` |
| v3 | `{"t":24.80,"h":55.30,"lux":412}` | **ejercicio**: modifica el ejemplo 2 para graficar `lux` |

En el ejemplo 2, las líneas marcadas con `// [v2]` son las únicas que cambian respecto al ejemplo 1.

## Requisitos

- **Arduino IDE** con soporte **ESP32-S3** (placa: *ESP32S3 Dev Module*). No usa librerías externas.
- **Qt 6** con los módulos **serialport** y **charts** (Qt Creator).

## Cómo se usa

1. En `firmware_contratos.ino` elige `#define CONTRATO 1` (o 2, o 3) y carga el firmware.
   Puedes comprobarlo en el Monitor Serie (115200): aparece una línea JSON cada 0.5 s.
2. **Cierra el Monitor Serie** (si no, Qt no puede abrir el puerto).
3. Abre `qt_ej1_un_valor/qt_ej1_un_valor.pro` en Qt Creator, compila y ejecuta.
4. Elige el puerto (`COMx` o `/dev/ttyUSB0`) y presiona **Conectar**.
5. Para el ejemplo 2 cambia el firmware a `CONTRATO 2` y abre `qt_ej2_dos_valores`.

## El camino del dato

```
ESP32: medir -> serializar JSON -> enviar línea (\n)
   -> USB/serie -> buffer de QSerialPort
Qt:    readyRead -> canReadLine/readLine -> QJsonDocument::fromJson -> QLineSeries::append -> QChart
```

`readyRead` **no** equivale a un mensaje: el serie llega en pedazos. Por eso se lee solo con
`canReadLine()` y `readLine()`, nunca con `readAll()`.
