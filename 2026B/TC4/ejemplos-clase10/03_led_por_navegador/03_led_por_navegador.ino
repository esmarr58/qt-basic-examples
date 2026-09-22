/*
 * Ejemplo 3 (Clase 10) - Control del LED por WebSocket + JSON (precalienta el TC4).
 * Diseno de Interfaces (I7262) - Dr. Ruben Estrada Marmolejo, CUCEI-UDG.
 *
 * MISMO protocolo que la Practica 4 y el TC4 (WebSocket:81 + JSON con "tipo").
 * La app de Qt manda {"tipo":"led","encendido":true|false}; la placa prende/apaga el
 * LED y DIFUNDE {"tipo":"estado_led","encendido":..}. Es exactamente el ida y vuelta
 * del TC4 (ahi se agregan mas comandos como salidas del L298 y el RGB).
 *
 *   PC  -> ESP32:  {"tipo":"led","encendido":true|false}
 *   ESP32 -> PC :  {"tipo":"saludo",...}  y  {"tipo":"estado_led","encendido":..}
 *
 * Cableado: LED GPIO4 + R 330, pulsador GPIO15->GND, LED RGB GPIO48.
 * Librerias: WiFi (core) + WebSockets (Links2004) + ArduinoJson + Adafruit NeoPixel.
 * Placa: "ESP32S3 Dev Module".
 */
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

const char* nombreRed = "GWN571D04";
const char* claveRed  = "ESP32CUCEI$$";

const int pinLed    = 4;
const int pinLedRgb = 48;
const int pinBoton  = 15;

WebSocketsServer socketServidor(81);
Adafruit_NeoPixel ledPlaca(1, pinLedRgb, NEO_GRB + NEO_KHZ800);
bool ledEncendido = false;

void enviarJsonACliente(uint8_t idCliente, const JsonDocument& doc) {
  String salida; serializeJson(doc, salida);
  socketServidor.sendTXT(idCliente, salida);
}
void difundirJson(const JsonDocument& doc) {
  String salida; serializeJson(doc, salida);
  socketServidor.broadcastTXT(salida);
}

void aplicarLed() {
  digitalWrite(pinLed, ledEncendido ? HIGH : LOW);
  ledPlaca.setPixelColor(0, ledEncendido ? ledPlaca.Color(0, 80, 0) : 0);
  ledPlaca.show();
}

void difundirEstado() {
  JsonDocument doc;
  doc["tipo"]      = "estado_led";
  doc["encendido"] = ledEncendido;
  difundirJson(doc);
}

void onEventoWS(uint8_t idCliente, WStype_t tipo, uint8_t *payload, size_t longitud) {
  if (tipo == WStype_CONNECTED) {
    JsonDocument saludo;
    saludo["tipo"]    = "saludo";
    saludo["mensaje"] = "Ejemplo 3: control del LED";
    saludo["puerto"]  = 81;
    enviarJsonACliente(idCliente, saludo);
    JsonDocument est;
    est["tipo"]      = "estado_led";
    est["encendido"] = ledEncendido;
    enviarJsonACliente(idCliente, est);
  } else if (tipo == WStype_TEXT) {
    JsonDocument doc;
    if (deserializeJson(doc, payload, longitud)) return;
    const char* t = doc["tipo"] | "";
    if (strcmp(t, "led") == 0) {
      ledEncendido = doc["encendido"] | false;
      aplicarLed();
      difundirEstado();
      Serial.print("LED -> "); Serial.println(ledEncendido ? "ENCENDIDO" : "APAGADO");
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(pinLed, OUTPUT);
  pinMode(pinBoton, INPUT_PULLUP);
  ledPlaca.begin(); ledPlaca.setBrightness(60);
  aplicarLed();
  WiFi.mode(WIFI_STA);
  WiFi.begin(nombreRed, claveRed);
  Serial.print("Conectando a "); Serial.print(nombreRed);
  while (WiFi.status() != WL_CONNECTED) { delay(300); Serial.print("."); }
  Serial.println();
  Serial.print("Conectado. WebSocket en ws://"); Serial.print(WiFi.localIP()); Serial.println(":81");
  socketServidor.begin();
  socketServidor.onEvent(onEventoWS);
}

void loop() {
  socketServidor.loop();
}
