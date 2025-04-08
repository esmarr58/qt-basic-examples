<?php
// Conexión a la base de datos
$mysqli = new mysqli("localhost", "usuario", "contraseña", "basedatos");
if ($mysqli->connect_error) {
    die("Error de conexión: " . $mysqli->connect_error);
}

// Obtener últimos 20 datos de temperatura
$tempQuery = "SELECT FROM_UNIXTIME(timestamp) as fecha, value FROM ejemplo1 WHERE type='temperatura' ORDER BY timestamp DESC LIMIT 20";
$tempResult = $mysqli->query($tempQuery);

// Obtener últimos 20 datos de humedad
$humQuery = "SELECT FROM_UNIXTIME(timestamp) as fecha, value FROM ejemplo1 WHERE type='humedad' ORDER BY timestamp DESC LIMIT 20";
$humResult = $mysqli->query($humQuery);

// Guardar los datos en arrays
$fechas = [];
$temperaturas = [];
$humedades = [];

while ($row = $tempResult->fetch_assoc()) {
    $fechas[] = $row['fecha'];
    $temperaturas[] = $row['value'];
}

while ($row = $humResult->fetch_assoc()) {
    $humedades[] = $row['value'];
}

$mysqli->close();
?>
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <title>Gráficas de Temperatura y Humedad</title>
    <meta http-equiv="refresh" content="5"> <!-- Actualiza cada 5 segundos -->
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        body { font-family: Arial; text-align: center; margin: 40px; }
        canvas { max-width: 700px; margin: 20px auto; }
        h2 { margin-top: 40px; }
    </style>
</head>
<body>
    <h1>Gráfica de Temperatura y Humedad</h1>
    <h3>Actualizado automáticamente cada 5 segundos</h3>

    <h2>Temperatura</h2>
    <canvas id="tempChart"></canvas>

    <h2>Humedad</h2>
    <canvas id="humChart"></canvas>

    <script>
        const etiquetas = <?php echo json_encode(array_reverse($fechas)); ?>;
        const datosTemperatura = <?php echo json_encode(array_reverse($temperaturas)); ?>;
        const datosHumedad = <?php echo json_encode(array_reverse($humedades)); ?>;

        const configTemp = {
            type: 'line',
            data: {
                labels: etiquetas,
                datasets: [{
                    label: 'Temperatura (°C)',
                    data: datosTemperatura,
                    borderWidth: 2,
                    fill: false,
                    borderColor: 'red'
                }]
            }
        };

        const configHum = {
            type: 'line',
            data: {
                labels: etiquetas,
                datasets: [{
                    label: 'Humedad (%)',
                    data: datosHumedad,
                    borderWidth: 2,
                    fill: false,
                    borderColor: 'blue'
                }]
            }
        };

        new Chart(document.getElementById('tempChart'), configTemp);
        new Chart(document.getElementById('humChart'), configHum);
    </script>
</body>
</html>
