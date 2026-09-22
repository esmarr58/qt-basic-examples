/*
 * Ejemplo 3 (Clase 10) - Controla el LED desde el NAVEGADOR (mini-preview del TC4).
 * Diseno de Interfaces (I7262) - Dr. Ruben Estrada Marmolejo, CUCEI-UDG.
 *
 * La placa se conecta a la red y levanta un pequeno SERVIDOR WEB. Abre en el
 * navegador de tu PC (en la MISMA red) la direccion http://<IP> que aparece en el
 * Monitor Serial y usa los botones ENCENDER / APAGAR para prender el LED. La pagina
 * tambien muestra el estado del pulsador fisico.
 *
 * Es el mismo "ida y vuelta" PC <-> ESP32 que en el TC4 haremos con Qt y WebSockets,
 * pero aqui con el navegador y HTTP, para que primero pruebes la red y el hardware.
 *
 * Cableado: LED + R 330 en GPIO4, pulsador entre GPIO15 y GND, LED RGB en GPIO48.
 * Librerias: WiFi + WebServer (core ESP32) + Adafruit NeoPixel.
 * Placa: "ESP32S3 Dev Module".
 */
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

const char* nombreRed = "GWN571D04";
const char* claveRed  = "ESP32CUCEI$$";

const int pinLed    = 4;    // LED externo + R 330 ohm
const int pinLedRgb = 48;   // LED RGB de la placa
const int pinBoton  = 15;   // pulsador -> GND (INPUT_PULLUP)

WebServer servidor(80);
Adafruit_NeoPixel ledPlaca(1, pinLedRgb, NEO_GRB + NEO_KHZ800);
bool ledEncendido = false;

void aplicarLed() {
  digitalWrite(pinLed, ledEncendido ? HIGH : LOW);
  ledPlaca.setPixelColor(0, ledEncendido ? ledPlaca.Color(0, 80, 0) : 0);
  ledPlaca.show();
}

String paginaHtml() {
  bool botonPresionado = (digitalRead(pinBoton) == LOW);
  String estado = ledEncendido ? "ENCENDIDO" : "APAGADO";
  String html = "<!doctype html><html lang='es'><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<title>ESP32 - Clase 10</title></head>";
  html += "<body style='font-family:sans-serif;text-align:center;margin-top:40px'>";
  html += "<h2>Control del LED (Clase 10)</h2>";
  html += "<p>Estado del LED: <b>" + estado + "</b></p>";
  html += "<p><a href='/encender'><button style='font-size:22px;padding:10px 20px'>ENCENDER</button></a> ";
  html += "<a href='/apagar'><button style='font-size:22px;padding:10px 20px'>APAGAR</button></a></p>";
  html += "<p>Boton fisico: " + String(botonPresionado ? "PRESIONADO" : "en reposo") + "</p>";
  html += "<p style='color:#888'>Recarga la pagina para actualizar el estado del boton.</p>";
  html += "</body></html>";
  return html;
}

void manejarRaiz()     { servidor.send(200, "text/html", paginaHtml()); }
void manejarEncender() { ledEncendido = true;  aplicarLed(); servidor.sendHeader("Location", "/"); servidor.send(303); }
void manejarApagar()   { ledEncendido = false; aplicarLed(); servidor.sendHeader("Location", "/"); servidor.send(303); }

void setup() {
  Serial.begin(115200);
  pinMode(pinLed, OUTPUT);
  pinMode(pinBoton, INPUT_PULLUP);
  ledPlaca.begin();
  ledPlaca.setBrightness(60);
  aplicarLed();

  WiFi.mode(WIFI_STA);
  WiFi.begin(nombreRed, claveRed);
  Serial.print("Conectando a ");
  Serial.print(nombreRed);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Abre esta direccion en tu navegador: http://");
  Serial.println(WiFi.localIP());

  servidor.on("/", manejarRaiz);
  servidor.on("/encender", manejarEncender);
  servidor.on("/apagar", manejarApagar);
  servidor.begin();
  Serial.println("Servidor web iniciado.");
}

void loop() {
  servidor.handleClient();
}
