/*
 * Ejemplo 2 (Clase 10) - Diagnostico de la conexion a la red del salon.
 * Diseno de Interfaces (I7262) - Dr. Ruben Estrada Marmolejo, CUCEI-UDG.
 *
 * Prueba la RED (sin botones): la placa se conecta al Access Point e imprime en el
 * Monitor Serial IP, mascara, gateway, DNS, MAC y RSSI. El LED RGB indica el estado
 * (rojo=conectando, verde=conectada) y reintenta si se cae.
 *
 * EMPAREJAMIENTO con Qt: ademas levanta un servidor web con
 *      GET /info  -> devuelve esos mismos datos en texto
 * para que la app de Qt (o un navegador) los CONSULTE por la red y confirme que la
 * PC y la ESP32 quedan en la MISMA red antes del TC4.
 *
 * Librerias: WiFi + WebServer (core ESP32) + Adafruit NeoPixel. Placa: "ESP32S3 Dev Module".
 */
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

const char* nombreRed = "GWN571D04";
const char* claveRed  = "ESP32CUCEI$$";

const int pinLedRgb = 48;
WebServer servidor(80);
Adafruit_NeoPixel ledPlaca(1, pinLedRgb, NEO_GRB + NEO_KHZ800);

void color(uint8_t r, uint8_t g, uint8_t b) {
  ledPlaca.setPixelColor(0, ledPlaca.Color(r, g, b));
  ledPlaca.show();
}

String textoDatosRed() {
  String s;
  s += "SSID:    " + WiFi.SSID() + "\n";
  s += "IP:      " + WiFi.localIP().toString() + "\n";
  s += "Mascara: " + WiFi.subnetMask().toString() + "\n";
  s += "Gateway: " + WiFi.gatewayIP().toString() + "\n";
  s += "DNS:     " + WiFi.dnsIP().toString() + "\n";
  s += "MAC:     " + WiFi.macAddress() + "\n";
  s += "RSSI:    " + String(WiFi.RSSI()) + " dBm\n";
  return s;
}

void imprimirDatosRed() {
  Serial.println("----- Datos de red -----");
  Serial.print(textoDatosRed());
  Serial.println("------------------------");
}

void manejarInfo() {
  servidor.send(200, "text/plain", textoDatosRed());
}

void conectar() {
  color(60, 0, 0);                 // rojo mientras conecta
  Serial.print("Conectando a ");
  Serial.println(nombreRed);
  WiFi.mode(WIFI_STA);
  WiFi.begin(nombreRed, claveRed);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 20000) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    color(0, 80, 0);               // verde: conectada
    imprimirDatosRed();
  } else {
    color(60, 0, 0);
    Serial.println("No se pudo conectar (revisa SSID, clave o senal).");
  }
}

void setup() {
  Serial.begin(115200);
  ledPlaca.begin();
  ledPlaca.setBrightness(60);
  conectar();
  servidor.on("/info", manejarInfo);
  servidor.begin();
  Serial.println("Servidor listo. La app de Qt puede consultar GET /info.");
}

unsigned long ultimoReporte = 0;

void loop() {
  servidor.handleClient();

  if (WiFi.status() != WL_CONNECTED) {
    color(60, 0, 0);
    conectar();                    // reintenta si se cayo
  }
  if (millis() - ultimoReporte > 3000) {   // reporte periodico al Serial
    ultimoReporte = millis();
    if (WiFi.status() == WL_CONNECTED) { color(0, 80, 0); imprimirDatosRed(); }
  }
}
