/*
 * Ejemplo 2 (Clase 10) - Diagnostico de la conexion a la red del salon.
 * Diseno de Interfaces (I7262) - Dr. Ruben Estrada Marmolejo, CUCEI-UDG.
 *
 * Solo prueba la RED (sin botones): la placa se conecta al Access Point del salon
 * e imprime TODOS los datos de red en el Monitor Serial (IP, mascara, gateway, DNS,
 * MAC y potencia de senal). El LED RGB indica el estado:
 *     rojo = conectando / sin red      verde = conectada
 * Si la red se cae, reintenta solo. Sirve para confirmar que la PC y la ESP32
 * quedaran en la MISMA red antes del TC4.
 *
 * Librerias: WiFi (core ESP32) + Adafruit NeoPixel. Placa: "ESP32S3 Dev Module".
 */
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>

const char* nombreRed = "GWN571D04";
const char* claveRed  = "ESP32CUCEI$$";

const int pinLedRgb = 48;
Adafruit_NeoPixel ledPlaca(1, pinLedRgb, NEO_GRB + NEO_KHZ800);

void color(uint8_t r, uint8_t g, uint8_t b) {
  ledPlaca.setPixelColor(0, ledPlaca.Color(r, g, b));
  ledPlaca.show();
}

void imprimirDatosRed() {
  Serial.println("----- Datos de red -----");
  Serial.print("SSID:    "); Serial.println(WiFi.SSID());
  Serial.print("IP:      "); Serial.println(WiFi.localIP());
  Serial.print("Mascara: "); Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: "); Serial.println(WiFi.gatewayIP());
  Serial.print("DNS:     "); Serial.println(WiFi.dnsIP());
  Serial.print("MAC:     "); Serial.println(WiFi.macAddress());
  Serial.print("RSSI:    "); Serial.print(WiFi.RSSI()); Serial.println(" dBm");
  Serial.println("------------------------");
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
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    color(60, 0, 0);
    conectar();                    // reintenta si se cayo
  } else {
    color(0, 80, 0);
    imprimirDatosRed();
  }
  delay(3000);
}
