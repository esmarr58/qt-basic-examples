#include <ArduinoJson.h>

const uint8_t LED_PIN = 13;

// Tamaño máximo de línea recibida (incluye '\n' y '\0')
constexpr size_t RX_BUF_SIZE = 128;
char rxBuf[RX_BUF_SIZE];
size_t rxLen = 0;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(9600);
  while (!Serial) { /* en UNO no es necesario, pero no estorba */ }

  Serial.println(F("Listo. Envia JSON y presiona Enter (ej: {\"pin\":13,\"estado\":1})"));
  Serial.println(F("Asegurate de tener 'Nueva linea' en el Monitor Serial."));
}

void loop() {
  // Leer caracteres disponibles sin bloquear
  while (Serial.available() > 0) {
    char c = (char)Serial.read();

    // Si tenemos espacio, guardamos
    if (rxLen < RX_BUF_SIZE - 1) {
      rxBuf[rxLen++] = c;
    }

    // ¿Fin de línea?
    if (c == '\n') {
      rxBuf[rxLen] = '\0'; // Terminar cadena C

      // Opcional: quitar '\r' final si viene CR+LF
      if (rxLen > 1 && rxBuf[rxLen - 2] == '\r') {
        rxBuf[rxLen - 2] = '\0';
      }

      // ---- Parsear JSON ----
      // Ajusta la capacidad según tu JSON real (usa ArduinoJson Assistant si gustas)
      StaticJsonDocument<128> doc;
      DeserializationError err = deserializeJson(doc, rxBuf);

      if (err) {
        Serial.print(F("Error al analizar JSON: "));
        Serial.println(err.f_str()); // muestra p.ej. NoMemory, InvalidInput, etc.
      } else {
        // Leer campos (si faltan, usan defaults)
        int pin = doc["pin"] | -1;
        int estado = doc["estado"] | 0;

        // En UNO evitar 0 y 1 (Serial). Rango válido típico 2..13
        if (pin >= 2 && pin <= 13) {
          pinMode(pin, OUTPUT);
          // Pequeña pausa no necesaria, pero segura en AVR
          delay(2);
          digitalWrite(pin, estado ? HIGH : LOW);
          Serial.print(F("Pin "));
          Serial.print(pin);
          Serial.println(estado ? F(" -> HIGH") : F(" -> LOW"));
        } else {
          Serial.println(F("Pin invalido para Arduino UNO (usa 2..13)."));
        }
      }

      // Resetear buffer para la siguiente línea
      rxLen = 0;
    }

    // Si se llenó el buffer sin encontrar '\n', limpia para evitar overflow
    if (rxLen >= RX_BUF_SIZE - 1) {
      rxLen = 0;
      Serial.println(F("Linea demasiado larga. Ajusta RX_BUF_SIZE o envia menos datos."));
    }
  }
}
