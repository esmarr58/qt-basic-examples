/*
  Clase 17 - Diseño de Interfaces (I7262) 2026B
  Envio de datos por LOTES con marca de tiempo epoch (ms).

  Mide la distancia cada DT_MS y junta MUESTRAS_POR_LOTE muestras en UNA linea JSON:
    {"seq":7,"t0":1792069200123,"dt":60,"sinc":true,"dist":[37.4,37.9,null,38.6]}

    seq  : numero de lote (0,1,2,...) -> Qt detecta lotes perdidos
    t0   : epoch en MILISEGUNDOS de la primera muestra del lote
    dt   : milisegundos entre muestras  -> muestra i ocurrio en t0 + i*dt
    sinc : false mientras Qt no nos haya dicho la hora real
    dist : distancias en cm; null = lectura fallida (JSON no acepta NaN)

  La ESP32 NO sabe la hora real: solo cuenta desde que arranco.
  Qt se la dice con:  {"cmd":"sync","epoch_ms":1792069200000}
  y respondemos:      {"ack":"sync","epoch_ms":1792069200000}

  USAR_SENSOR_REAL 0 -> distancia simulada (sin hardware)
  USAR_SENSOR_REAL 1 -> HC-SR04 real (TRIG = GPIO 12, ECHO = GPIO 13, igual que la Practica #2)

  Puerto serie: 115200 baudios, 8N1. Placa: ESP32S3 Dev Module.
*/

#include "esp_timer.h"

#define USAR_SENSOR_REAL 0
#define TRIG_PIN 12
#define ECHO_PIN 13

const int DT_MS = 60;                 // el HC-SR04 pide >= 60 ms entre disparos
const int MUESTRAS_POR_LOTE = 10;     // 10 x 60 ms = un lote cada 0.6 s

float lote[MUESTRAS_POR_LOTE];
int   enLote = 0;
int64_t t0Lote = 0;
uint32_t seq = 0;

int64_t offsetEpochMs = 0;            // epoch = offset + ms desde el arranque
bool sincronizado = false;
int64_t proximaMuestraMs = 0;

char entrada[96];
int  largoEntrada = 0;

// Milisegundos desde el arranque en 64 bits: no se desborda como millis() (~49.7 dias)
int64_t msDesdeArranque() {
  return esp_timer_get_time() / 1000;
}

int64_t epochMs(int64_t msArranque) {
  return offsetEpochMs + msArranque;  // sin sincronizar, offset = 0 -> fechas de 1970
}

float medirDistancia() {
#if USAR_SENSOR_REAL
  digitalWrite(TRIG_PIN, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long us = pulseIn(ECHO_PIN, HIGH, 25000);   // timeout ~4 m
  if (us == 0) return NAN;                             // sin eco
  return us * 0.034 / 2.0;
#else
  float t = msDesdeArranque() / 1000.0;                // segundos
  float d = 60.0 + 40.0 * sin(2.0 * PI * t / 8.0);     // objeto que va y viene cada 8 s
  return d + random(-10, 11) / 10.0;                   // +-1 cm de ruido
#endif
}

void enviarLote() {
  Serial.printf("{\"seq\":%lu,\"t0\":%lld,\"dt\":%d,\"sinc\":%s,\"dist\":[",
                (unsigned long)seq, (long long)t0Lote, DT_MS, sincronizado ? "true" : "false");
  for (int i = 0; i < MUESTRAS_POR_LOTE; i++) {
    if (i > 0) Serial.print(',');
    if (isnan(lote[i])) Serial.print("null");
    else Serial.printf("%.1f", lote[i]);
  }
  Serial.print("]}\n");
  seq++;
}

void procesarComando(const char *linea) {
  // Comando esperado: {"cmd":"sync","epoch_ms":1792069200000}
  const char *p = strstr(linea, "\"epoch_ms\":");
  if (strstr(linea, "\"sync\"") == nullptr || p == nullptr) return;
  int64_t epochQt = strtoll(p + 11, nullptr, 10);

  offsetEpochMs = epochQt - msDesdeArranque();
  sincronizado = true;
  enLote = 0;                         // el lote a medias tenia la hora vieja: se descarta
  Serial.printf("{\"ack\":\"sync\",\"epoch_ms\":%lld}\n", (long long)epochQt);
}

void leerComandos() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      entrada[largoEntrada] = '\0';
      procesarComando(entrada);
      largoEntrada = 0;
    } else if (largoEntrada < (int)sizeof(entrada) - 1) {
      entrada[largoEntrada++] = c;
    }
  }
}

void setup() {
  Serial.begin(115200);
#if USAR_SENSOR_REAL
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
#endif
  proximaMuestraMs = msDesdeArranque();
}

void loop() {
  leerComandos();

  if (msDesdeArranque() < proximaMuestraMs) return;
  int64_t tMuestra = proximaMuestraMs;  // instante nominal de esta muestra
  proximaMuestraMs += DT_MS;            // ritmo fijo: dt constante entre muestras

  if (enLote == 0) t0Lote = epochMs(tMuestra);
  lote[enLote++] = medirDistancia();

  if (enLote == MUESTRAS_POR_LOTE) {
    enviarLote();
    enLote = 0;
  }
}
