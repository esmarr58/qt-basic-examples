#include <WiFi.h>
#include <WebSocketsServer.h>   // arduinoWebSockets (Links2004)
#include <ArduinoJson.h>        // Benoit Blanchon
#include <ESPmDNS.h>            // mDNS / DNS-SD
#include <Adafruit_NeoPixel.h>
#include <DHT.h>                // Adafruit DHT

#ifndef PIN_RGB
#define PIN_RGB 48   // <- cámbialo si tu placa usa otro pin
#endif

Adafruit_NeoPixel rgb(1, PIN_RGB, NEO_GRB + NEO_KHZ800);

// ====== CONFIG WiFi ======
 const char* ssid     = "GWN571D04";
 const char* password = "ESP32CUCEI$$s";


// ====== Servidor WebSocket (puerto 81) ======
WebSocketsServer servidorWS(81);
const int PIN_LED = 44;   // Pin donde está el LED

// ====== Flags y temporizadores ======
volatile bool latidoRecibido = false;
unsigned long ultimoEnvioMs  = 0;
unsigned long CADA_MS        = 2000;   // cadencia de telemetría en ms (>= 2000 para DHT22)

// ====== DHT22 ======
#define PIN_DHT   6      // Tu configuración previa indicaba GPIO 6 para DHT
#define DHTTYPE   DHT22
DHT dht(PIN_DHT, DHTTYPE);

// Control de cadencia del DHT (mínimo 2s entre lecturas)
const unsigned long DHT_MIN_MS = 2000;
unsigned long ultimoDhtMs = 0;
float tempC_cache = NAN;
float hum_cache   = NAN;
bool  dht_ok      = false;

// ====== Lectura ADC (opcional) ======
const int PIN_ADC = 4;

// ====== Variable global para el hostname ======
String nombreHost;

// ====== RGB ======
void setColor(uint8_t r, uint8_t g, uint8_t b) {
  rgb.setPixelColor(0, rgb.Color(r, g, b));
  rgb.show();
}

// ====== Utilidades ======
String macSinDosPuntos() {
  String mac = WiFi.macAddress();
  mac.toLowerCase();
  String out;
  out.reserve(12);
  for (char c : mac) {
    if (c != ':') out += c;
  }
  return out;
}

String sufijoMAC6() {
  String m = macSinDosPuntos();
  if (m.length() >= 6) return m.substring(m.length() - 6);
  return m;
}

// ---- Enviar JSON ----
void enviarJsonACliente(uint8_t idCliente, const JsonDocument& doc) {
  String out;
  serializeJson(doc, out);
  servidorWS.sendTXT(idCliente, out);
}

void difundirJson(const JsonDocument& doc) {
  String out;
  serializeJson(doc, out);
  servidorWS.broadcastTXT(out);
}

// ---- Lectura del DHT con cache y control de tiempo ----
void actualizarLecturaDHT(bool forzar = false) {
  unsigned long ahora = millis();
  if (!forzar && (ahora - ultimoDhtMs < DHT_MIN_MS)) {
    // Aún no toca leer; mantiene cache
    return;
  }

  float h = dht.readHumidity();
  float t = dht.readTemperature(); // °C

  if (isnan(h) || isnan(t)) {
    dht_ok = false;  // conserva cache previo pero marca fallo
  } else {
    hum_cache   = h;
    tempC_cache = t;
    dht_ok      = true;
    ultimoDhtMs = ahora;
  }
}

// ---- Eventos WebSocket ----
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
      if (err) {
        StaticJsonDocument<128> resp;
        resp["tipo"]  = "error";
        resp["razon"] = "json_invalido";
        enviarJsonACliente(idCliente, resp);
        break;
      }

      const char* tipoMsg = doc["tipo"] | "";

      if (strcmp(tipoMsg, "latido") == 0) {
        latidoRecibido = true;
        StaticJsonDocument<160> ack;
        ack["tipo"]       = "latido_ok";
        ack["tiempo_ms"]  = (uint32_t)millis();
        enviarJsonACliente(idCliente, ack);
      }
      else if (strcmp(tipoMsg, "eco") == 0) {
        StaticJsonDocument<256> eco;
        eco["tipo"]  = "eco_respuesta";
        eco["datos"] = doc["datos"];
        enviarJsonACliente(idCliente, eco);
      }
      else if (strcmp(tipoMsg, "tasa") == 0) {
        unsigned long nueva = doc["ms"] | CADA_MS;
        if (nueva < 100)  nueva = 100;
        if (nueva < DHT_MIN_MS) nueva = DHT_MIN_MS; // no bajar de 2s si usas DHT22
        if (nueva > 5000) nueva = 5000;
        CADA_MS = nueva;

        StaticJsonDocument<128> r;
        r["tipo"]  = "tasa_ok";
        r["ms"]    = (uint32_t)CADA_MS;
        enviarJsonACliente(idCliente, r);
      }
      else if (strcmp(tipoMsg, "led") == 0) {
        int estado = doc["estado"] | 0; // {"tipo":"led","estado":1}
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
      else if (strcmp(tipoMsg, "leer_dht") == 0) {
        // Fuerza lectura inmediata si ya pasó el mínimo (evita saturar)
        actualizarLecturaDHT(true);
        StaticJsonDocument<256> resp;
        resp["tipo"]        = "dht_lectura";
        resp["dht_ok"]      = dht_ok;
        resp["temperatura"] = tempC_cache; // puede ser NaN si nunca hubo una válida
        resp["humedad"]     = hum_cache;
        enviarJsonACliente(idCliente, resp);
      }
      else {
        StaticJsonDocument<128> resp;
        resp["tipo"]  = "aviso";
        resp["razon"] = "tipo_desconocido";
        enviarJsonACliente(idCliente, resp);
      }
    } break;

    default:
      break;
  }
}

void iniciarMDNSyDNSSD() {
  // Hostname único: esp32-XXXXXX
  nombreHost = "esp32-" + sufijoMAC6();
  nombreHost.toLowerCase();

  if (!MDNS.begin(nombreHost.c_str())) {
    Serial.println("[mDNS] Error iniciando mDNS");
  } else {
    Serial.println("[mDNS] Iniciado: " + nombreHost + ".local");

    bool ok = MDNS.addService("ws", "tcp", 81);
    if (ok) {
      MDNS.addServiceTxt("ws", "tcp", "proto", "json");
      Serial.println("[mDNS] Servicio _ws._tcp anunciado en puerto 81");
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW); // LED apagado al inicio

  rgb.begin();
  rgb.clear();
  rgb.show();   // apaga al inicio

  dht.begin();  // Inicializa DHT22

  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(400);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Conectado. IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  analogReadResolution(12);
  pinMode(PIN_ADC, INPUT);

  iniciarMDNSyDNSSD();

  servidorWS.begin();
  servidorWS.onEvent(onEventoWS);
  Serial.println("Servidor WebSocket en ws://" + nombreHost + ".local:81");
}

void loop() {
  servidorWS.loop();

  const unsigned long ahora = millis();

  // Actualiza lectura del DHT solo cuando toca (cacheado)
  actualizarLecturaDHT(false);

  if (latidoRecibido && (ahora - ultimoEnvioMs >= CADA_MS)) {
    ultimoEnvioMs = ahora;

    int lecturaADC = analogRead(PIN_ADC);

    StaticJsonDocument<256> msg;
    msg["tipo"]         = "telemetria";
    msg["adc"]          = lecturaADC;

    // Envía últimos valores de DHT (reales o NaN si nunca fue válido)
    msg["temperatura"]  = tempC_cache;  // °C
    msg["humedad"]      = hum_cache;    // %
    msg["dht_ok"]       = dht_ok;       // true si la última lectura fue válida

    msg["tiempo_ms"]    = (uint32_t)ahora;

    difundirJson(msg);
  }
}
