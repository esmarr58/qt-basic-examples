/*
 * Ejemplo 1 (Clase 10) - Boton con RETENCION que enciende un LED de la ESP32-S3,
 *                         conexion a la red y ESPEJO por la red (para la app Qt).
 * Diseno de Interfaces (I7262) - Dr. Ruben Estrada Marmolejo, CUCEI-UDG.
 *
 * Prueba TRES cosas antes del TC4:
 *   (1) HARDWARE: un pulsador con "retencion" -> cada pulsacion ENCIENDE o APAGA
 *       el LED (igual que en la Practica 1), con antirrebote.
 *   (2) RED: la placa se conecta al Access Point del salon e imprime su IP.
 *   (3) EMPAREJAMIENTO: levanta un pequeno servidor web para que la app de Qt
 *       (o un navegador) LEA el estado del LED por la red:
 *          GET /estado  -> responde "ON" o "OFF"
 *          GET /        -> pagina HTML que se auto-actualiza
 *       Asi la ventana de Qt refleja lo que hace el boton fisico.
 *
 * Cableado (el mismo del TC4):
 *   - Pulsador externo entre GPIO15 y GND  (INPUT_PULLUP: reposo=HIGH, presionado=LOW)
 *   - LED externo + R 330 ohm en GPIO4
 *   - LED RGB de la placa (WS2812) en GPIO48: verde=encendido, negro=apagado
 *
 * Librerias: WiFi + WebServer (core ESP32) + Adafruit NeoPixel.
 * Placa (Arduino IDE): "ESP32S3 Dev Module".
 */
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

// ----- Red del salon (misma del TC4) -----
const char* nombreRed = "GWN571D04";
const char* claveRed  = "ESP32CUCEI$$";

// ----- Pines -----
const int pinBoton  = 15;   // pulsador -> GND (INPUT_PULLUP)
const int pinLed    = 4;    // LED externo + R 330 ohm
const int pinLedRgb = 48;   // LED RGB integrado en la placa

WebServer servidor(80);
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

// ----- Servidor web: la app de Qt lee el estado por aqui -----
void manejarEstado() {
  servidor.send(200, "text/plain", ledEncendido ? "ON" : "OFF");
}
void manejarRaiz() {
  String estado = ledEncendido ? "ENCENDIDO" : "APAGADO";
  String html = "<!doctype html><html lang='es'><head><meta charset='utf-8'>";
  html += "<meta http-equiv='refresh' content='1'>";   // se auto-actualiza cada 1 s
  html += "<meta name='viewport' content='width=device-width,initial-scale=1'>";
  html += "<title>Ejemplo 1 - boton con retencion</title></head>";
  html += "<body style='font-family:sans-serif;text-align:center;margin-top:40px'>";
  html += "<h2>LED de la ESP32</h2><p>Estado: <b>" + estado + "</b></p>";
  html += "<p style='color:#888'>Presiona el boton fisico para alternar.</p></body></html>";
  servidor.send(200, "text/html", html);
}

void conectarRed() {
  Serial.print("Conectando a ");
  Serial.print(nombreRed);
  WiFi.mode(WIFI_STA);
  WiFi.begin(nombreRed, claveRed);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
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

  servidor.on("/", manejarRaiz);
  servidor.on("/estado", manejarEstado);
  servidor.begin();
  Serial.println("Servidor listo. La app de Qt puede leer GET /estado.");
  Serial.println("Presiona el boton para ENCENDER/APAGAR el LED (retencion).");
}

void loop() {
  servidor.handleClient();

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
