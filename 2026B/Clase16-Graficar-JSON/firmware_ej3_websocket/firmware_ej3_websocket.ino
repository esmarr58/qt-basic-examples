/*
 * ============================================================================
 *  Clase 16 - Ejemplo 3 - Diseño de Interfaces (I7262) - ESP32-S3-DevKitC-1
 * ----------------------------------------------------------------------------
 *  El MISMO contrato v2 del ejemplo 2 (temperatura + humedad), pero ahora por
 *  WebSocket en lugar del puerto serie. Igual que en el TC4, la ESP32 es el
 *  SERVIDOR WebSocket (puerto 81) y la interfaz de Qt es el CLIENTE.
 *
 *  No necesita sensores: temperatura y humedad se SIMULAN (DHT22 virtual).
 *
 *  Librerias (las mismas del TC4):
 *   - WebSockets  (Markus Sattler / Links2004)  -> servidor WebSocket
 *   - ArduinoJson (Benoit Blanchon, v7)          -> armar JSON
 *
 *  Placa (Arduino IDE 1.8.19): "ESP32S3 Dev Module".
 *
 *  PROTOCOLO JSON  (mismo esquema del TC4: campo "tipo")
 *  --------------
 *  ESP32  ->  Computadora:
 *     {"tipo":"saludo",   "mensaje":"...", "puerto":81}   // al conectar
 *     {"tipo":"medicion", "t":24.8, "h":55.3}             // cada 500 ms, a TODOS
 *
 *  Diferencia con el puerto serie: cada mensaje WebSocket llega COMPLETO a Qt
 *  (el protocolo ya separa un mensaje del siguiente); no hace falta el '\n'.
 * ============================================================================
 */

#include <WiFi.h>
#include <WebSocketsServer.h>   // WebSockets (Links2004)
#include <ArduinoJson.h>        // ArduinoJson v7 (Benoit Blanchon)

// ---------------------------------------------------------------------------
// 1) Credenciales del punto de acceso (Access Point del salon)
// ---------------------------------------------------------------------------
const char* nombreRed  = "GWN571D04";      // SSID del Access Point
const char* claveRed    = "ESP32CUCEI$$";   // Contraseña del Access Point

// ---------------------------------------------------------------------------
// 2) Servidor WebSocket en el puerto 81
// ---------------------------------------------------------------------------
WebSocketsServer servidorSocket = WebSocketsServer(81);

// ---------------------------------------------------------------------------
// 3) Señal simulada: senoidales lentas (1 ciclo por minuto)
// ---------------------------------------------------------------------------
const float temperaturaMedia = 25.0, temperaturaAmplitud = 1.0;   // 24 a 26 °C
const float humedadMedia     = 55.0, humedadAmplitud     = 5.0;   // 50 a 60 %
const unsigned long periodoSenal = 60000;   // milisegundos

// Cada cuanto se manda una medicion
const unsigned long periodoEnvio = 500;     // milisegundos (2 por segundo)
unsigned long marcaTiempoEnvio = 0;

// ---------------------------------------------------------------------------
// (1) Medir + (2) Serializar + (3) Enviar a TODOS los clientes conectados
// ---------------------------------------------------------------------------
void enviarMedicion() {
  float fase = 2.0 * PI * (float)(millis() % periodoSenal) / (float)periodoSenal;
  float temperatura = temperaturaMedia + temperaturaAmplitud * sin(fase);
  float humedad     = humedadMedia     + humedadAmplitud     * sin(fase + PI);  // contrafase

  JsonDocument documento;
  documento["tipo"] = "medicion";
  documento["t"]    = round(temperatura * 100) / 100.0;   // 2 decimales
  documento["h"]    = round(humedad * 100) / 100.0;

  String texto;
  serializeJson(documento, texto);
  servidorSocket.broadcastTXT(texto);     // un mensaje = una medicion
}

// ---------------------------------------------------------------------------
// Callback del servidor WebSocket (nombre de funcion fijado por la libreria)
// ---------------------------------------------------------------------------
void alEventoWebSocket(uint8_t numCliente, WStype_t tipo, uint8_t* carga, size_t longitud) {
  switch (tipo) {
    case WStype_CONNECTED: {
      IPAddress ip = servidorSocket.remoteIP(numCliente);
      Serial.print("Cliente conectado: ");
      Serial.println(ip.toString());
      JsonDocument saludo;
      saludo["tipo"]    = "saludo";
      saludo["mensaje"] = "Ejemplo 3: contrato v2 por WebSocket";
      saludo["puerto"]  = 81;
      String textoSaludo;
      serializeJson(saludo, textoSaludo);
      servidorSocket.sendTXT(numCliente, textoSaludo);
      break;
    }
    case WStype_DISCONNECTED:
      Serial.printf("Cliente %u desconectado\n", numCliente);
      break;
    default:
      break;   // este ejemplo no recibe comandos
  }
}

// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  // Conectar al Access Point
  WiFi.begin(nombreRed, claveRed);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Conectado. IP de la ESP32: ");
  Serial.println(WiFi.localIP());   // <-- esta IP se escribe en la interfaz de Qt

  // Iniciar el servidor WebSocket
  servidorSocket.begin();
  servidorSocket.onEvent(alEventoWebSocket);
  Serial.println("Servidor WebSocket iniciado en el puerto 81");
}

// ---------------------------------------------------------------------------
void loop() {
  servidorSocket.loop();   // atiende a los clientes WebSocket

  if (millis() - marcaTiempoEnvio >= periodoEnvio) {
    marcaTiempoEnvio = millis();
    enviarMedicion();
  }
}
