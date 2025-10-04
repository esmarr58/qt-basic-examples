#include <WiFi.h>
#include <WebSocketsServer.h>   // arduinoWebSockets (Links2004)
#include <ArduinoJson.h>        // Benoit Blanchon
#include <ESPmDNS.h>            // mDNS / DNS-SD

// ====== CONFIG WiFi ======
//const char* ssid     = "GWN571D04";
//const char* password = "ESP32CUCEI$$s";

const char* ssid     = "Totalplay-2.4G-8200";
const char* password = "SfyUAMkFb2L7DQDU";
// ====== Servidor WebSocket (puerto 81) ======
WebSocketsServer servidorWS(81);

// ====== Flags y temporizadores ======
volatile bool latidoRecibido = false;
unsigned long ultimoEnvioMs  = 0;
unsigned long CADA_MS        = 1000;   // cadencia de telemetría en ms

// ====== Lectura ADC (opcional) ======
const int PIN_ADC = 4;

// ====== Variable global para el hostname ======
String nombreHost;

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
        if (nueva > 5000) nueva = 5000;
        CADA_MS = nueva;

        StaticJsonDocument<128> r;
        r["tipo"]  = "tasa_ok";
        r["ms"]    = (uint32_t)CADA_MS;
        enviarJsonACliente(idCliente, r);
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
  if (latidoRecibido && (ahora - ultimoEnvioMs >= CADA_MS)) {
    ultimoEnvioMs = ahora;

    int lecturaADC     = analogRead(PIN_ADC);
    float temperatura  = 25.0f + (lecturaADC % 50) * 0.02f;
    float humedad      = 50.0f + (lecturaADC % 50) * 0.10f;

    StaticJsonDocument<256> msg;
    msg["tipo"]         = "telemetria";
    msg["adc"]          = lecturaADC;
    msg["temperatura"]  = temperatura;
    msg["humedad"]      = humedad;
    msg["tiempo_ms"]    = (uint32_t)ahora;

    difundirJson(msg);
  }
}
