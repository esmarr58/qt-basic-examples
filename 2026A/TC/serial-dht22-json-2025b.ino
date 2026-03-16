#include <ArduinoJson.h>
#include <math.h>

// ========================
// Selección de modo
// ========================
// 1 = usar DHT22 real
// 0 = usar señal simulada
#define USAR_SENSOR_REAL 0

#if USAR_SENSOR_REAL
  #include "DHT.h"
#endif

// Prototipo
void processLine(const char* line);

// ========================
// Configuración general
// ========================
const uint8_t ledPinDefault = 13;

const size_t BUF_LEN = 160;
char inBuf[BUF_LEN];
size_t inLen = 0;

// ========================
// Configuración DHT22
// ========================
#if USAR_SENSOR_REAL
static const uint8_t DHT_TYPE = DHT22;
#endif

// ========================
// Configuración simulación
// ========================
const float TEMP_MEDIA     = 25.0;
const float TEMP_AMPLITUD  = 1.0;   // 24 a 26 °C

const float HUM_MEDIA      = 55.0;
const float HUM_AMPLITUD   = 5.0;   // 50 a 60 %

const unsigned long PERIODO_MS = 60000UL;

// ========================
// Lectura real
// ========================
#if USAR_SENSOR_REAL
void leerDHT22(uint8_t pinDHT) {
  DHT dht(pinDHT, DHT_TYPE);
  dht.begin();

  delay(30);

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    StaticJsonDocument<128> out;
    out["ok"]    = false;
    out["error"] = "Fallo al leer DHT22";
    serializeJson(out, Serial);
    Serial.println();
    return;
  }

  StaticJsonDocument<128> out;
  out["ok"]      = true;
  out["modo"]    = "real";
  out["temp"]    = t;
  out["humedad"] = h;
  serializeJson(out, Serial);
  Serial.println();
}
#endif

// ========================
// Lectura simulada
// ========================
void leerSensorSimulado() {
  unsigned long tMs = millis();

  float fase = 2.0 * PI * ((float)(tMs % PERIODO_MS) / (float)PERIODO_MS);

  float temp = TEMP_MEDIA + TEMP_AMPLITUD * sin(fase);

  // Desfase 180 grados
  float humedad = HUM_MEDIA + HUM_AMPLITUD * sin(fase + PI);

  StaticJsonDocument<128> out;
  out["ok"]      = true;
  out["modo"]    = "simulado";
  out["temp"]    = temp;
  out["humedad"] = humedad;
  serializeJson(out, Serial);
  Serial.println();
}

// ========================
// Función unificada
// ========================
void leerTemperaturaHumedad(uint8_t pinDHT) {
#if USAR_SENSOR_REAL
  leerDHT22(pinDHT);
#else
  (void)pinDHT; // evita warning por variable no usada
  leerSensorSimulado();
#endif
}

// ========================
// Setup / Loop
// ========================
void setup() {
  pinMode(ledPinDefault, OUTPUT);
  digitalWrite(ledPinDefault, LOW);

  Serial.begin(115200);
  Serial.setTimeout(50);

#if USAR_SENSOR_REAL
  Serial.println(F("Modo activo: SENSOR DHT22"));
#else
  Serial.println(F("Modo activo: SENSOR SIMULADO"));
#endif

  Serial.println(F("Listo. Envia JSON por linea:"));
  Serial.println(F("{\"pin\":13,\"estado\":1}"));
  Serial.println(F("{\"sensor\":\"th\",\"pin\":4}"));
}

void loop() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();

    if (c == '\r') continue;

    if (c == '\n') {
      inBuf[inLen] = '\0';
      processLine(inBuf);
      inLen = 0;
    } else {
      if (inLen < (BUF_LEN - 1)) {
        inBuf[inLen++] = c;
      } else {
        inLen = 0;
        Serial.println(F("Linea demasiado larga"));
      }
    }
  }
}

// ========================
// Procesamiento JSON
// ========================
void processLine(const char* line) {
  StaticJsonDocument<256> doc;

  DeserializationError err = deserializeJson(doc, line);
  if (err) {
    Serial.print(F("Error al analizar JSON: "));
    Serial.println(err.f_str());
    return;
  }

  // Rama LED
  if (doc.containsKey("pin") && doc.containsKey("estado")) {
    int pin = doc["pin"].as<int>();
    int estado = doc["estado"].as<int>();

#if defined(ARDUINO_AVR_UNO)
    if (pin < 0 || pin > 13) {
      Serial.println(F("Pin fuera de rango (0..13 para Arduino UNO)"));
      return;
    }
#endif

    pinMode(pin, OUTPUT);
    delay(2);
    digitalWrite(pin, (estado == 1) ? HIGH : LOW);
    Serial.println((estado == 1) ? F("LED encendido") : F("LED apagado"));
    return;
  }

  // Rama temperatura/humedad
  bool esTH = false;
  int thPin = 4; // pin por defecto

  if (doc.containsKey("sensor")) {
    const char* s = doc["sensor"];
    if (s && (strcmp(s, "th") == 0 || strcmp(s, "dht22") == 0)) {
      esTH = true;
      if (doc.containsKey("pin")) thPin = doc["pin"].as<int>();
    }
  }

  if (!esTH && doc.containsKey("cmd")) {
    const char* c = doc["cmd"];
    if (c && strcmp(c, "leer_th") == 0) {
      esTH = true;
      if (doc.containsKey("pin")) thPin = doc["pin"].as<int>();
    }
  }

  if (esTH) {
    leerTemperaturaHumedad((uint8_t)thPin);
    return;
  }

  Serial.println(F("JSON no reconocido. Usa {\"pin\",\"estado\"} o {\"sensor\":\"th\",\"pin\":4}"));
}
