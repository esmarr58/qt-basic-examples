/*
  Clase 16 - Diseño de Interfaces (I7262) 2026B
  Firmware de "contratos JSON" para graficar en Qt.

  NO necesita sensores: genera temperatura, humedad y luz por software
  (DHT22 y fotorresistencia "virtuales") y manda UNA LINEA JSON por muestra.

  Elige el contrato con CONTRATO:
    1 -> {"t":24.80}                          (un valor)
    2 -> {"t":24.80,"h":55.30}                (dos valores)
    3 -> {"t":24.80,"h":55.30,"lux":412}      (tres valores, para el ejercicio)

  Puerto serie: 115200 baudios, 8N1. Placa: ESP32S3 Dev Module.
*/

#define CONTRATO 1            // <-- cambia a 2 o 3

const unsigned long PERIODO_ENVIO_MS = 500;   // 2 muestras por segundo

// Señal simulada: senoidales lentas (1 ciclo por minuto)
const float TEMP_MEDIA = 25.0, TEMP_AMPLITUD = 1.0;   // 24 a 26 °C
const float HUM_MEDIA  = 55.0, HUM_AMPLITUD  = 5.0;   // 50 a 60 %
const float LUX_MEDIA  = 400.0, LUX_AMPLITUD = 150.0; // 250 a 550 lx
const unsigned long PERIODO_SENAL_MS = 60000UL;

unsigned long ultimoEnvio = 0;

void setup() {
  Serial.begin(115200);
}

void loop() {
  if (millis() - ultimoEnvio < PERIODO_ENVIO_MS) return;
  ultimoEnvio = millis();

  // 1) Medir (aqui: simular)
  float fase = 2.0 * PI * (float)(millis() % PERIODO_SENAL_MS) / (float)PERIODO_SENAL_MS;
  float temp = TEMP_MEDIA + TEMP_AMPLITUD * sin(fase);
  float hum  = HUM_MEDIA  + HUM_AMPLITUD  * sin(fase + PI);        // contrafase
  int   lux  = (int)(LUX_MEDIA + LUX_AMPLITUD * sin(3.0 * fase));  // 3 veces mas rapida

  // 2) Serializar el contrato y 3) enviarlo como UNA linea (el '\n' final la cierra)
#if CONTRATO == 1
  Serial.printf("{\"t\":%.2f}\n", temp);
#elif CONTRATO == 2
  Serial.printf("{\"t\":%.2f,\"h\":%.2f}\n", temp, hum);
#else
  Serial.printf("{\"t\":%.2f,\"h\":%.2f,\"lux\":%d}\n", temp, hum, lux);
#endif
}
