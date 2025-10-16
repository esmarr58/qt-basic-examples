#include <WiFi.h>
#include <WebSocketsServer.h>   // arduinoWebSockets (Links2004)
#include <ArduinoJson.h>        // Benoit Blanchon
#include <ESPmDNS.h>            // mDNS / DNS-SD
#include <Adafruit_NeoPixel.h>
#include <DHT.h>

// ==== FreeRTOS (tareas/colas) ====
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// ----------------------------------------------------------------------
//                   PROTOTIPOS (usados en varias secciones)
// ----------------------------------------------------------------------
void enviarJsonACliente(uint8_t idCliente, const JsonDocument& doc);
void difundirJson(const JsonDocument& doc);

// ====== CONFIGURACIÓN GENERAL ======
#define SIMULAR_DHT true   // true = datos simulados / false = leer DHT22 real

#ifndef PIN_RGB
#define PIN_RGB 48         // Pin para LED RGB (NeoPixel)
#endif

Adafruit_NeoPixel rgb(1, PIN_RGB, NEO_GRB + NEO_KHZ800);

// ====== CONFIG WiFi ======
const char* ssid     = "Totalplay-2.4G-8200";
const char* password = "SfyUAMkFb2L7DQDU";

// ====== Servidor WebSocket ======
WebSocketsServer servidorWS(81);
const int PIN_LED = 44;

// ====== Temporizadores y variables ======
volatile bool latidoRecibido = false;
unsigned long ultimoEnvioMs  = 0;
unsigned long CADA_MS        = 2000;

// ====== DHT22 ======
#define PIN_DHT   6
#define DHTTYPE   DHT22
DHT dht(PIN_DHT, DHTTYPE);

const unsigned long DHT_MIN_MS_REAL = 2000;
const unsigned long DHT_MIN_MS_SIM  = 500;
float tempC_cache = NAN;
float hum_cache   = NAN;
bool  dht_ok      = false;
unsigned long ultimoDhtMs = 0;

// ====== ADC opcional ======
const int PIN_ADC = 4;

// ====== Hostname ======
String nombreHost;

// ======================================================================
//              MAPEO DE PINES – 2 MÓDULOS L298N (ASIGNACIÓN FINAL)
// ======================================================================
// Módulo 1 (Motor A completo): 4,5,6,7
#define M1_IN1 4
#define M1_IN2 5
#define M1_IN3 6
#define M1_IN4 7

// Módulo 2 (Motor B completo): 11,12,13,14
#define M2_IN1 11
#define M2_IN2 12
#define M2_IN3 13
#define M2_IN4 14

// Estructura para manejar pares (igual que en tu tester serial)
struct PairPins {
  const char* name;
  uint8_t p1;
  uint8_t p2;
};

// Pares: q=P0, w=P1, e=P2, r=P3 (coincide con tu diagnóstico)
PairPins pairs[4] = {
  {"P0 M1_IN1/M1_IN2", M1_IN1, M1_IN2}, // q
  {"P1 M1_IN3/M1_IN4", M1_IN3, M1_IN4}, // w
  {"P2 M2_IN1/M2_IN2", M2_IN1, M2_IN2}, // e
  {"P3 M2_IN3/M2_IN4", M2_IN3, M2_IN4}  // r
};

// ======================================================================
//                     INICIALIZACIÓN DE PINES (motores)
// ======================================================================
static inline void motores_init_pines() {
  for (int i = 0; i < 4; ++i) {
    pinMode(pairs[i].p1, OUTPUT);
    pinMode(pairs[i].p2, OUTPUT);
    digitalWrite(pairs[i].p1, LOW);
    digitalWrite(pairs[i].p2, LOW);
  }
}

// ======================================================================
//                 HELPERS DE MOVIMIENTO (SIN PWM, ON/OFF)
//  Códigos de estado por par (como en tu tester):
//   0 -> 00 (LOW, LOW)
//   1 -> 01 (LOW, HIGH)
//   2 -> 10 (HIGH, LOW)
//   3 -> 11 (HIGH, HIGH)
// Tu diagnóstico para atajos:
//   Avanzar:     todos = 1
//   Retroceder:  todos = 2
//   Derecha:     e,r = 2 ; q,w = 1
//   Izquierda:   e,r = 1 ; q,w = 2
// ======================================================================
static inline void setPairState(int idx, int code) {
  if (idx < 0 || idx > 3) return;
  uint8_t p1 = pairs[idx].p1;
  uint8_t p2 = pairs[idx].p2;
  switch (code) {
    case 0: digitalWrite(p1, LOW);  digitalWrite(p2, LOW);  break;
    case 1: digitalWrite(p1, LOW);  digitalWrite(p2, HIGH); break;
    case 2: digitalWrite(p1, HIGH); digitalWrite(p2, LOW);  break;
    case 3: digitalWrite(p1, HIGH); digitalWrite(p2, HIGH); break;
    default: break;
  }
}

static inline void motores_parar_total() {
  for (int i = 0; i < 4; ++i) setPairState(i, 0);
}

// Atajos usando tu mapeo final
static inline void mover_avanzar() {
  setPairState(0, 1); // q
  setPairState(1, 1); // w
  setPairState(2, 1); // e
  setPairState(3, 1); // r
}
static inline void mover_retroceder() {
  setPairState(0, 2);
  setPairState(1, 2);
  setPairState(2, 2);
  setPairState(3, 2);
}
static inline void mover_izquierda() {
  setPairState(2, 1); // e
  setPairState(3, 1); // r
  setPairState(0, 2); // q
  setPairState(1, 2); // w
}
static inline void mover_derecha() {
  setPairState(2, 2); // e
  setPairState(3, 2); // r
  setPairState(0, 1); // q
  setPairState(1, 1); // w
}

// ======================================================================
//        COLA / TASK DE MOTORES (CORE 1) - CONTROL POR millis()
// ======================================================================
enum class DirMov : uint8_t { Parado=0, Avanzar, Retroceder, Izquierda, Derecha, Parar };

struct CmdMotor {
  DirMov accion;
  uint32_t dur_ms;   // duración solicitada
  uint32_t t0_ms;    // (lo rellena el task)
};

static QueueHandle_t motorCmdQ = nullptr;
static TaskHandle_t motorTaskHandle = nullptr;

void MotorTask(void* arg) {
  (void)arg;
  CmdMotor cmd{DirMov::Parado, 0, 0};

  bool enMovimiento = false;
  uint32_t t_fin = 0;

  for (;;) {
    // Preempción inmediata si llega nuevo comando
    CmdMotor nuevo;
    if (xQueueReceive(motorCmdQ, &nuevo, (TickType_t)0) == pdTRUE) {
      // Parar cualquier movimiento actual antes de aplicar el nuevo
      enMovimiento = false;
      motores_parar_total();

      if (nuevo.accion == DirMov::Parar || nuevo.accion == DirMov::Parado || nuevo.dur_ms == 0) {
        enMovimiento = false;
      } else {
        // Aplicar movimiento según tu mapeo final (pares q,w,e,r)
        switch (nuevo.accion) {
          case DirMov::Avanzar:     mover_avanzar();    break;
          case DirMov::Retroceder:  mover_retroceder(); break;
          case DirMov::Izquierda:   mover_izquierda();  break;
          case DirMov::Derecha:     mover_derecha();    break;
          default:                  motores_parar_total(); break;
        }
        nuevo.t0_ms = millis();
        t_fin = nuevo.t0_ms + nuevo.dur_ms;
        enMovimiento = true;

        StaticJsonDocument<128> ack;
        ack["tipo"] = "mover_inicio";
        difundirJson(ack);
      }
      cmd = nuevo;
    }

    // Verificar timeout por millis
    if (enMovimiento) {
      uint32_t ahora = millis();
      if ((int32_t)(ahora - t_fin) >= 0) {
        enMovimiento = false;
        motores_parar_total();

        StaticJsonDocument<128> fin;
        fin["tipo"] = "mover_fin";
        difundirJson(fin);
      }
    }

    vTaskDelay(1); // ~1 ms
  }
}

// ======================================================================
//                        AUXILIARES GENERALES
// ======================================================================
void setColor(uint8_t r, uint8_t g, uint8_t b) {
  rgb.setPixelColor(0, rgb.Color(r, g, b));
  rgb.show();
}

String macSinDosPuntos() {
  String mac = WiFi.macAddress();
  mac.toLowerCase();
  String out;
  for (char c : mac) if (c != ':') out += c;
  return out;
}
String sufijoMAC6() {
  String m = macSinDosPuntos();
  return (m.length() >= 6) ? m.substring(m.length() - 6) : m;
}

void enviarJsonACliente(uint8_t idCliente, const JsonDocument& doc) {
  String out; serializeJson(doc, out);
  servidorWS.sendTXT(idCliente, out);
}
void difundirJson(const JsonDocument& doc) {
  String out; serializeJson(doc, out);
  servidorWS.broadcastTXT(out);
}

// ======================================================================
//                         LECTURA DEL DHT
// ======================================================================
void actualizarLecturaDHT(bool forzar = false) {
  unsigned long ahora = millis();
  unsigned long intervalo = SIMULAR_DHT ? DHT_MIN_MS_SIM : DHT_MIN_MS_REAL;
  if (!forzar && (ahora - ultimoDhtMs < intervalo)) return;

  if (SIMULAR_DHT) {
    tempC_cache = 25.0 + (random(-300, 300) / 100.0);    // 22°C..28°C
    hum_cache   = 45.0 + (random(-1000, 1000) / 100.0);  // 35%..55%
    dht_ok = true;
    ultimoDhtMs = ahora;
  } else {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (isnan(h) || isnan(t)) {
      dht_ok = false;
    } else {
      hum_cache = h;
      tempC_cache = t;
      dht_ok = true;
      ultimoDhtMs = ahora;
    }
  }
}

// ======================================================================
//                      EVENTOS WEBSOCKET
// ======================================================================
void onEventoWS(uint8_t idCliente, WStype_t tipo, uint8_t * payload, size_t longitud) {
  switch (tipo) {
    case WStype_CONNECTED: {
      IPAddress ip = servidorWS.remoteIP(idCliente);
      Serial.printf("[WS] Cliente %u conectado desde %s\n", idCliente, ip.toString().c_str());

      StaticJsonDocument<256> msg;
      msg["tipo"]        = "saludo";
      msg["mensaje"]     = "Servidor ESP32-S3 listo";
      msg["puerto"]      = 81;
      msg["nombre_host"] = nombreHost;
      msg["mac"]         = WiFi.macAddress();
      enviarJsonACliente(idCliente, msg);
    } break;

    case WStype_DISCONNECTED:
      Serial.printf("[WS] Cliente %u desconectado\n", idCliente);
      break;

    case WStype_TEXT: {
      String recibido((char*)payload, longitud);
      Serial.printf("[WS] RX (%u bytes): %s\n", (unsigned)longitud, recibido.c_str());

      StaticJsonDocument<1024> doc;
      DeserializationError err = deserializeJson(doc, recibido);
      if (err) return;

      const char* tipoMsg = doc["tipo"] | "";

      if (strcmp(tipoMsg, "latido") == 0) {
        latidoRecibido = true;
        StaticJsonDocument<160> ack;
        ack["tipo"]       = "latido_ok";
        ack["tiempo_ms"]  = (uint32_t)millis();
        enviarJsonACliente(idCliente, ack);
      }
      else if (strcmp(tipoMsg, "leer_dht") == 0) {
        actualizarLecturaDHT(true);
        StaticJsonDocument<256> resp;
        resp["tipo"]        = "dht_lectura";
        resp["dht_ok"]      = dht_ok;
        resp["temperatura"] = tempC_cache;
        resp["humedad"]     = hum_cache;
        enviarJsonACliente(idCliente, resp);
      }
      else if (strcmp(tipoMsg, "led") == 0) {
        int estado = doc["estado"] | 0;
        digitalWrite(PIN_LED, estado ? HIGH : LOW);
        StaticJsonDocument<128> resp;
        resp["tipo"] = "led_ack";
        resp["estado"] = estado;
        enviarJsonACliente(idCliente, resp);
      }
      else if (strcmp(tipoMsg, "rgb") == 0) {
        uint8_t r = doc["r"] | 0;
        uint8_t g = doc["g"] | 0;
        uint8_t b = doc["b"] | 0;
        setColor(r, g, b);
        StaticJsonDocument<128> ack;
        ack["tipo"] = "rgb_ack";
        ack["r"] = r; ack["g"] = g; ack["b"] = b;
        enviarJsonACliente(idCliente, ack);
      }
      else if (strcmp(tipoMsg, "mover") == 0) {
        // Esquema:
        // { "tipo":"mover", "accion":"avanzar|retroceder|izquierda|derecha|parar", "ms": 800 }
        const char* accionStr = doc["accion"] | "avanzar";
        uint32_t ms = doc["ms"] | 600;
        if (ms > 8000) ms = 8000; // límite de seguridad

        DirMov a = DirMov::Avanzar;
        if      (strcmp(accionStr, "retroceder") == 0) a = DirMov::Retroceder;
        else if (strcmp(accionStr, "izquierda")  == 0) a = DirMov::Izquierda;
        else if (strcmp(accionStr, "derecha")    == 0) a = DirMov::Derecha;
        else if (strcmp(accionStr, "parar")      == 0) a = DirMov::Parar;

        CmdMotor cmd{ a, ms, 0 };
        bool ok = (motorCmdQ && (xQueueSend(motorCmdQ, &cmd, 0) == pdTRUE));

        StaticJsonDocument<192> ack2;
        ack2["tipo"]   = ok ? "mover_ack" : "mover_err";
        ack2["accion"] = accionStr;
        ack2["ms"]     = ms;
        enviarJsonACliente(idCliente, ack2);
      }
    } break;

    default:
      break;
  }
}

// ======================================================================
//                           mDNS
// ======================================================================
void iniciarMDNSyDNSSD() {
  nombreHost = "esp32-" + sufijoMAC6();
  nombreHost.toLowerCase();

  if (MDNS.begin(nombreHost.c_str())) {
    MDNS.addService("ws", "tcp", 81);
    MDNS.addServiceTxt("ws", "tcp", "proto", "json");
    Serial.println("[mDNS] Iniciado: " + nombreHost + ".local");
  } else {
    Serial.println("[mDNS] Error iniciando mDNS");
  }
}

// ======================================================================
//                             SETUP
// ======================================================================
void setup() {
  Serial.begin(115200);
  delay(200);

  // Semilla para random (mejor variación en modo simulado)
  pinMode(PIN_ADC, INPUT);
  analogReadResolution(12);
  randomSeed(analogRead(PIN_ADC));

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);

  rgb.begin();
  rgb.clear();
  rgb.show();

  if (!SIMULAR_DHT) dht.begin();

  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP: "); Serial.println(WiFi.localIP());

  iniciarMDNSyDNSSD();

  // === Motores: pines e iniciar apagados por defecto ===
  motores_init_pines();
  motores_parar_total();

  // === Cola y Task de motores (core 1) ===
  motorCmdQ = xQueueCreate(5, sizeof(CmdMotor));
  if (!motorCmdQ) {
    Serial.println("[MOTOR] ERROR: no se pudo crear la cola de comandos");
  } else {
    BaseType_t ok = xTaskCreatePinnedToCore(
      MotorTask,
      "MotorTask",
      4096,       // stack
      nullptr,
      2,          // prioridad
      &motorTaskHandle,
      1           // core 1 (el otro CPU)
    );
    if (ok != pdPASS) {
      Serial.println("[MOTOR] ERROR: no se pudo crear el task de motores");
    } else {
      Serial.println("[MOTOR] Task de motores iniciado en core 1");
    }
  }

  // === WebSocket ===
  servidorWS.begin();
  servidorWS.onEvent(onEventoWS);
  Serial.println("Servidor WebSocket en ws://" + nombreHost + ".local:81");
}

// ======================================================================
//                              LOOP
// ======================================================================
void loop() {
  servidorWS.loop();

  unsigned long ahora = millis();
  actualizarLecturaDHT(false);

  if (latidoRecibido && (ahora - ultimoEnvioMs >= CADA_MS)) {
    ultimoEnvioMs = ahora;

    int lecturaADC = analogRead(PIN_ADC);

    StaticJsonDocument<256> msg;
    msg["tipo"]         = "telemetria";
    msg["adc"]          = lecturaADC;
    msg["temperatura"]  = tempC_cache;
    msg["humedad"]      = hum_cache;
    msg["dht_ok"]       = dht_ok;
    msg["tiempo_ms"]    = (uint32_t)ahora;

    difundirJson(msg);
  }
}
