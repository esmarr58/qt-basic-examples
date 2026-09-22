/*
 * Ejemplo 2 (Clase 10) - Diagnostico de red por WebSocket + JSON.
 * Diseno de Interfaces (I7262) - Dr. Ruben Estrada Marmolejo, CUCEI-UDG.
 *
 * MISMO protocolo que la Practica 4 y el TC4 (WebSocket:81 + JSON con "tipo").
 * La app de Qt manda {"tipo":"leer_red"} y la placa responde con los datos de red.
 * Al conectarse un cliente, tambien se le manda el saludo y los datos.
 *
 *   PC  -> ESP32:  {"tipo":"leer_red"}
 *   ESP32 -> PC :  {"tipo":"saludo","mensaje":"...","puerto":81}
 *                  {"tipo":"datos_red","ssid":"..","ip":"..","mascara":"..",
 *                   "gateway":"..","dns":"..","mac":"..","rssi":-57}
 *
 * El LED RGB indica el estado (rojo=conectando, verde=conectada) y reintenta si se cae.
 * Librerias: WiFi (core) + WebSockets (Links2004) + ArduinoJson + Adafruit NeoPixel.
 * Placa: "ESP32S3 Dev Module".
 */
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

const char* nombreRed = "GWN571D04";
const char* claveRed  = "ESP32CUCEI$$";

const int pinLedRgb = 48;
WebSocketsServer socketServidor(81);
Adafruit_NeoPixel ledPlaca(1, pinLedRgb, NEO_GRB + NEO_KHZ800);

void color(uint8_t r, uint8_t g, uint8_t b) {
  ledPlaca.setPixelColor(0, ledPlaca.Color(r, g, b));
  ledPlaca.show();
}

void enviarJsonACliente(uint8_t idCliente, const JsonDocument& doc) {
  String salida; serializeJson(doc, salida);
  socketServidor.sendTXT(idCliente, salida);
}

void llenarDatosRed(JsonDocument& doc) {
  doc["tipo"]    = "datos_red";
  doc["ssid"]    = WiFi.SSID();
  doc["ip"]      = WiFi.localIP().toString();
  doc["mascara"] = WiFi.subnetMask().toString();
  doc["gateway"] = WiFi.gatewayIP().toString();
  doc["dns"]     = WiFi.dnsIP().toString();
  doc["mac"]     = WiFi.macAddress();
  doc["rssi"]    = WiFi.RSSI();
}

void imprimirDatosRed() {
  Serial.printf("IP %s  GW %s  RSSI %d dBm\n",
    WiFi.localIP().toString().c_str(), WiFi.gatewayIP().toString().c_str(), WiFi.RSSI());
}

void onEventoWS(uint8_t idCliente, WStype_t tipo, uint8_t *payload, size_t longitud) {
  if (tipo == WStype_CONNECTED) {
    JsonDocument saludo;
    saludo["tipo"]    = "saludo";
    saludo["mensaje"] = "Ejemplo 2: diagnostico de red";
    saludo["puerto"]  = 81;
    enviarJsonACliente(idCliente, saludo);
    JsonDocument datos; llenarDatosRed(datos);
    enviarJsonACliente(idCliente, datos);
  } else if (tipo == WStype_TEXT) {
    JsonDocument doc;
    if (deserializeJson(doc, payload, longitud)) return;
    const char* t = doc["tipo"] | "";
    if (strcmp(t, "leer_red") == 0) {
      JsonDocument datos; llenarDatosRed(datos);
      enviarJsonACliente(idCliente, datos);
    }
  }
}

void conectar() {
  color(60, 0, 0);
  Serial.print("Conectando a "); Serial.println(nombreRed);
  WiFi.mode(WIFI_STA);
  WiFi.begin(nombreRed, claveRed);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) { delay(300); Serial.print("."); }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) { color(0, 80, 0); imprimirDatosRed(); Serial.println("WebSocket en ws://<IP>:81"); }
  else { color(60, 0, 0); Serial.println("No se pudo conectar (revisa SSID/clave/senal)."); }
}

unsigned long ultimoReporte = 0;

void setup() {
  Serial.begin(115200);
  ledPlaca.begin(); ledPlaca.setBrightness(60);
  conectar();
  socketServidor.begin();
  socketServidor.onEvent(onEventoWS);
}

void loop() {
  socketServidor.loop();
  if (WiFi.status() != WL_CONNECTED) { color(60, 0, 0); conectar(); }
  if (millis() - ultimoReporte > 3000) {
    ultimoReporte = millis();
    if (WiFi.status() == WL_CONNECTED) { color(0, 80, 0); imprimirDatosRed(); }
  }
}
