/*
 * Ejemplo 1 (Clase 10) - Boton con RETENCION -> LED, ESPEJO por WebSocket + JSON.
 * Diseno de Interfaces (I7262) - Dr. Ruben Estrada Marmolejo, CUCEI-UDG.
 *
 * MISMO protocolo que la Practica 4 y el TC4: servidor WebSocket (puerto 81) con
 * mensajes JSON que llevan un campo "tipo". Helpers: onEventoWS, enviarJsonACliente,
 * difundirJson. Al conectarse un cliente se le manda un {"tipo":"saludo"} y el estado.
 *
 *   ESP32 -> PC (JSON):
 *     {"tipo":"saludo","mensaje":"...","puerto":81}
 *     {"tipo":"estado_led","encendido":true|false}   // al cambiar el boton
 *
 * (1) Hardware: pulsador con retencion -> LED (como Practica 1), con antirrebote.
 * (2) Red: se conecta al Access Point del salon e imprime su IP.
 * (3) Empareja con la app de Qt por ws://<IP>:81 (la ventana refleja el LED).
 *
 * Cableado: pulsador GPIO15->GND (INPUT_PULLUP), LED GPIO4 + R 330, LED RGB GPIO48.
 * Librerias: WiFi (core) + WebSockets (Links2004) + ArduinoJson + Adafruit NeoPixel.
 * Placa: "ESP32S3 Dev Module".
 */
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

const char* nombreRed = "GWN571D04";
const char* claveRed  = "ESP32CUCEI$$";

const int pinBoton  = 15;
const int pinLed    = 4;
const int pinLedRgb = 48;

WebSocketsServer socketServidor(81);
Adafruit_NeoPixel ledPlaca(1, pinLedRgb, NEO_GRB + NEO_KHZ800);

bool ledEncendido = false;

int  ultimaLectura = HIGH, estadoEstable = HIGH;
unsigned long tiempoCambio = 0;
const unsigned long msAntirrebote = 30;

// ---- Helpers JSON (igual que la Practica 4) ----
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
  (void)payload; (void)longitud;
  if (tipo == WStype_CONNECTED) {
    JsonDocument saludo;
    saludo["tipo"]    = "saludo";
    saludo["mensaje"] = "Ejemplo 1: espejo del boton";
    saludo["puerto"]  = 81;
    enviarJsonACliente(idCliente, saludo);

    JsonDocument est;
    est["tipo"]      = "estado_led";
    est["encendido"] = ledEncendido;
    enviarJsonACliente(idCliente, est);
  }
}

void conectarRed() {
  Serial.print("Conectando a "); Serial.print(nombreRed);
  WiFi.mode(WIFI_STA);
  WiFi.begin(nombreRed, claveRed);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300); Serial.print(".");
    ledPlaca.setPixelColor(0, ledPlaca.Color(0, 0, 60)); ledPlaca.show(); delay(120);
    ledPlaca.setPixelColor(0, 0);                        ledPlaca.show();
  }
  Serial.println();
  Serial.print("Conectado. IP: "); Serial.println(WiFi.localIP());
  Serial.println("WebSocket en ws://<IP>:81");
}

void setup() {
  Serial.begin(115200);
  pinMode(pinBoton, INPUT_PULLUP);
  pinMode(pinLed, OUTPUT);
  ledPlaca.begin(); ledPlaca.setBrightness(60); ledPlaca.show();
  aplicarLed();
  conectarRed();
  socketServidor.begin();
  socketServidor.onEvent(onEventoWS);
  Serial.println("Presiona el boton para ENCENDER/APAGAR el LED (retencion).");
}

void loop() {
  socketServidor.loop();
  int lectura = digitalRead(pinBoton);
  if (lectura != ultimaLectura) tiempoCambio = millis();
  if (millis() - tiempoCambio > msAntirrebote) {
    if (lectura != estadoEstable) {
      estadoEstable = lectura;
      if (estadoEstable == LOW) {
        ledEncendido = !ledEncendido;
        aplicarLed();
        difundirEstado();
        Serial.print("LED -> "); Serial.println(ledEncendido ? "ENCENDIDO" : "APAGADO");
      }
    }
  }
  ultimaLectura = lectura;
}
