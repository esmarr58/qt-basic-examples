#include <WiFi.h>
#include <WebSocketsServer.h>   // arduinoWebSockets (Links2004)
#include <ArduinoJson.h>        // Benoit Blanchon
#include <ESPmDNS.h>            // mDNS / DNS-SD
#include <Adafruit_NeoPixel.h>
#include <DHT.h>

// ====== CONFIGURACIÓN GENERAL ======
#define SIMULAR_DHT true   // <-- true = datos simulados / false = leer DHT22 real

#ifndef PIN_RGB
#define PIN_RGB 48         // Pin para LED RGB
#endif

Adafruit_NeoPixel rgb(1, PIN_RGB, NEO_GRB + NEO_KHZ800);

// ====== CONFIG WiFi ======
 const char* ssid     = "GWN571D04";
 const char* password = "ESP32CUCEI$$s";

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

// ====== Funciones auxiliares ======
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

// ====== LECTURA DEL DHT ======
void actualizarLecturaDHT(bool forzar = false) {
  unsigned long ahora = millis();
  unsigned long intervalo = SIMULAR_DHT ? DHT_MIN_MS_SIM : DHT_MIN_MS_REAL;

  if (!forzar && (ahora - ultimoDhtMs < intervalo)) return;

  if (SIMULAR_DHT) {
    // ====== Datos simulados ======
    tempC_cache = 25.0 + (random(-300, 300) / 100.0); // entre 22°C y 28°C
    hum_cache   = 45.0 + (random(-1000, 1000) / 100.0); // entre 35% y 55%
    dht_ok = true;
    ultimoDhtMs = ahora;
  } else {
    // ====== Lectura real ======
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

// ====== EVENTOS WEBSOCKET ======
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
    } break;

    default:
      break;
  }
}

// ====== mDNS ======
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

// ====== SETUP ======
void setup() {
  Serial.begin(115200);
  delay(200);

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

  analogReadResolution(12);
  pinMode(PIN_ADC, INPUT);

  iniciarMDNSyDNSSD();

  servidorWS.begin();
  servidorWS.onEvent(onEventoWS);
  Serial.println("Servidor WebSocket en ws://" + nombreHost + ".local:81");
}

// ====== LOOP ======
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
