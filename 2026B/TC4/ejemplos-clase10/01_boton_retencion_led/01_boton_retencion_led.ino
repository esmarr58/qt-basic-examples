/*
 * Ejemplo 1 (Clase 10) - Boton con RETENCION que enciende un LED de la ESP32-S3,
 *                         y conexion a la red del salon.
 * Diseno de Interfaces (I7262) - Dr. Ruben Estrada Marmolejo, CUCEI-UDG.
 *
 * Prueba DOS cosas antes de empezar el TC4:
 *   (1) HARDWARE: un pulsador con "retencion" -> cada pulsacion ENCIENDE o APAGA
 *       el LED (igual que en la Practica 1 del semestre pasado). Antirrebote por
 *       software para que no titile.
 *   (2) RED: la placa se conecta al Access Point del salon e imprime su IP en el
 *       Monitor Serial (esa IP la usaras en el TC4 desde Qt).
 *
 * Cableado (el mismo del TC4, para no recablear):
 *   - Pulsador externo entre GPIO15 y GND  (pull-up interna: reposo=HIGH, presionado=LOW)
 *   - LED externo + R 330 ohm en GPIO4      (una de las salidas IN1..IN4 del TC4)
 *   - LED RGB de la placa (WS2812) en GPIO48: verde=encendido, negro=apagado
 *
 * Librerias: WiFi (incluida en el core ESP32) + Adafruit NeoPixel.
 * Placa en Arduino IDE: "ESP32S3 Dev Module".
 */
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>

// ----- Red del salon (misma del TC4) -----
const char* nombreRed = "GWN571D04";
const char* claveRed  = "ESP32CUCEI$$";

// ----- Pines -----
const int pinBoton  = 15;   // pulsador -> GND (INPUT_PULLUP)
const int pinLed    = 4;    // LED externo + R 330 ohm
const int pinLedRgb = 48;   // LED RGB integrado en la placa

Adafruit_NeoPixel ledPlaca(1, pinLedRgb, NEO_GRB + NEO_KHZ800);

// ----- Estado de la retencion (memoria del LED) -----
bool ledEncendido = false;

// ----- Variables del antirrebote -----
int  ultimaLectura   = HIGH;
int  estadoEstable   = HIGH;
unsigned long tiempoCambio = 0;
const unsigned long msAntirrebote = 30;

void aplicarLed() {
  digitalWrite(pinLed, ledEncendido ? HIGH : LOW);
  if (ledEncendido) ledPlaca.setPixelColor(0, ledPlaca.Color(0, 80, 0)); // verde
  else              ledPlaca.setPixelColor(0, 0);                        // apagado
  ledPlaca.show();
}

void conectarRed() {
  Serial.print("Conectando a ");
  Serial.print(nombreRed);
  WiFi.mode(WIFI_STA);
  WiFi.begin(nombreRed, claveRed);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
    // parpadeo azul mientras conecta
    ledPlaca.setPixelColor(0, ledPlaca.Color(0, 0, 60)); ledPlaca.show(); delay(120);
    ledPlaca.setPixelColor(0, 0);                        ledPlaca.show();
  }
  Serial.println();
  Serial.print("Conectado. IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("Senal (RSSI): ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
}

void setup() {
  Serial.begin(115200);
  pinMode(pinBoton, INPUT_PULLUP);
  pinMode(pinLed, OUTPUT);
  ledPlaca.begin();
  ledPlaca.setBrightness(60);
  ledPlaca.show();
  aplicarLed();
  conectarRed();
  Serial.println("Listo: presiona el boton para ENCENDER/APAGAR el LED (retencion).");
}

void loop() {
  int lectura = digitalRead(pinBoton);
  if (lectura != ultimaLectura) {
    tiempoCambio = millis();          // hubo un cambio: reinicia el reloj del rebote
  }
  if (millis() - tiempoCambio > msAntirrebote) {
    if (lectura != estadoEstable) {
      estadoEstable = lectura;
      if (estadoEstable == LOW) {     // flanco de bajada = pulsacion confirmada
        ledEncendido = !ledEncendido; // RETENCION: alterna el estado
        aplicarLed();
        Serial.print("LED -> ");
        Serial.println(ledEncendido ? "ENCENDIDO" : "APAGADO");
      }
    }
  }
  ultimaLectura = lectura;
}
