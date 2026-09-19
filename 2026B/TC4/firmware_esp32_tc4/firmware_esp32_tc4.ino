/*
 * ============================================================================
 *  TC4 - Diseño de Interfaces (I7262) - Firmware para ESP32-S3-DevKitC-1
 * ----------------------------------------------------------------------------
 *  La ESP32-S3 funciona como SERVIDOR WebSocket. La computadora (interfaz en Qt)
 *  se conecta como CLIENTE y ambos intercambian mensajes en formato JSON.
 *
 *  Objetivos del TC4:
 *   1) Controlar SALIDAS DIGITALES de la ESP32 de forma remota (comando JSON).
 *      Las 4 salidas representan las entradas IN1..IN4 del puente H L298
 *      (que en la Practica 1 moveran los motores del robot). Aqui son LEDs.
 *      Ademas se controla el LED RGB de la placa (GPIO 48).
 *   2) Leer BOTONES fisicos de la ESP32 y avisar a la interfaz (evento JSON)
 *      para disparar acciones en Qt (reproducir / pausar un sonido).
 *
 *  Librerias (las mismas que se instalaron en el video de preparacion):
 *   - WebSockets  (Markus Sattler / Links2004)  -> servidor WebSocket
 *   - ArduinoJson (Benoit Blanchon, v7)          -> armar/leer JSON
 *   - Adafruit NeoPixel                          -> LED RGB de la placa
 *
 *  Placa (Arduino IDE 1.8.19): "ESP32S3 Dev Module".
 *
 *  PROTOCOLO JSON
 *  --------------
 *  Computadora  ->  ESP32 (comandos):
 *     {"comando":"salida", "canal":1, "estado":true}   // canal 1..4 (IN1..IN4)
 *     {"comando":"rgb",    "estado":true}              // LED RGB de la placa
 *     {"comando":"consulta"}                           // pedir el estado actual
 *
 *  ESP32  ->  Computadora (eventos y confirmaciones):
 *     {"evento":"boton", "boton":"reproducir"}
 *     {"evento":"boton", "boton":"pausar"}
 *     {"evento":"boton", "boton":"boot"}
 *     {"evento":"estado","salidas":[false,false,false,false],"rgb":false}
 * ============================================================================
 */

#include <WiFi.h>
#include <WebSocketsServer.h>   // WebSockets (Links2004)
#include <ArduinoJson.h>        // ArduinoJson v7 (Benoit Blanchon)
#include <Adafruit_NeoPixel.h>  // LED RGB de la placa

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
// 3) Mapa de pines
//    Salidas (representan IN1..IN4 del L298). Elegimos GPIO seguros del DevKitC.
// ---------------------------------------------------------------------------
const int pinesSalida[4] = { 4, 5, 6, 7 };   // canal 1..4  ->  IN1..IN4
const int cantidadSalidas = 4;

// LED RGB integrado en la placa (WS2812 en el GPIO 48)
const int pinLedRgb = 48;
Adafruit_NeoPixel ledPlaca(1, pinLedRgb, NEO_GRB + NEO_KHZ800);

// Botones de entrada (se leen con resistencia PULL-UP interna: reposo = HIGH,
// presionado = LOW porque el boton conecta el pin a GND).
const int pinBotonBoot        = 0;    // boton BOOT de la placa
const int pinBotonReproducir  = 15;   // pulsador externo -> "reproducir"
const int pinBotonPausar      = 16;   // pulsador externo -> "pausar"

// ---------------------------------------------------------------------------
// 4) Estado actual de las salidas (para poder informarlo cuando lo pidan)
// ---------------------------------------------------------------------------
bool estadoSalidas[4] = { false, false, false, false };
bool estadoRgb = false;

// Antirrebote: recordamos el ultimo nivel leido y el momento del cambio.
int  nivelPrevioBoot        = HIGH;
int  nivelPrevioReproducir  = HIGH;
int  nivelPrevioPausar      = HIGH;
unsigned long marcaTiempoRebote = 0;
const unsigned long tiempoAntirrebote = 40;  // milisegundos

// ---------------------------------------------------------------------------
// Enciende o apaga el LED RGB de la placa (verde = encendido, apagado = negro)
// ---------------------------------------------------------------------------
void aplicarLedRgb(bool encendido) {
  if (encendido) {
    ledPlaca.setPixelColor(0, ledPlaca.Color(0, 150, 0));  // verde
  } else {
    ledPlaca.setPixelColor(0, ledPlaca.Color(0, 0, 0));    // apagado
  }
  ledPlaca.show();
  estadoRgb = encendido;
}

// ---------------------------------------------------------------------------
// Aplica un estado (encendido/apagado) a una salida (canal 1..4)
// ---------------------------------------------------------------------------
void aplicarSalida(int canal, bool encendido) {
  if (canal < 1 || canal > cantidadSalidas) return;   // canal invalido
  int indice = canal - 1;
  digitalWrite(pinesSalida[indice], encendido ? HIGH : LOW);
  estadoSalidas[indice] = encendido;
}

// ---------------------------------------------------------------------------
// Arma un JSON con el estado actual y lo envia al cliente indicado
// ---------------------------------------------------------------------------
void enviarEstado(uint8_t numCliente) {
  JsonDocument documento;
  documento["evento"] = "estado";
  JsonArray salidas = documento["salidas"].to<JsonArray>();
  for (int i = 0; i < cantidadSalidas; i++) {
    salidas.add(estadoSalidas[i]);
  }
  documento["rgb"] = estadoRgb;

  String texto;
  serializeJson(documento, texto);
  servidorSocket.sendTXT(numCliente, texto);
}

// ---------------------------------------------------------------------------
// Avisa a TODOS los clientes que se presiono un boton (para disparar acciones)
// ---------------------------------------------------------------------------
void avisarBoton(const char* nombreBoton) {
  JsonDocument documento;
  documento["evento"] = "boton";
  documento["boton"]  = nombreBoton;

  String texto;
  serializeJson(documento, texto);
  servidorSocket.broadcastTXT(texto);
  Serial.print("Boton presionado: ");
  Serial.println(nombreBoton);
}

// ---------------------------------------------------------------------------
// Interpreta un comando JSON recibido desde la interfaz de Qt
// ---------------------------------------------------------------------------
void procesarComando(uint8_t numCliente, uint8_t* carga, size_t longitud) {
  JsonDocument documento;
  DeserializationError error = deserializeJson(documento, carga, longitud);
  if (error) {
    Serial.println("JSON invalido, se ignora.");
    servidorSocket.sendTXT(numCliente, "{\"evento\":\"error\",\"detalle\":\"json_invalido\"}");
    return;
  }

  const char* comando = documento["comando"];
  if (comando == nullptr) return;

  if (strcmp(comando, "salida") == 0) {
    int  canal  = documento["canal"] | 0;
    bool estado = documento["estado"] | false;
    aplicarSalida(canal, estado);
    enviarEstado(numCliente);                 // confirmamos el nuevo estado
  }
  else if (strcmp(comando, "rgb") == 0) {
    bool estado = documento["estado"] | false;
    aplicarLedRgb(estado);
    enviarEstado(numCliente);
  }
  else if (strcmp(comando, "consulta") == 0) {
    enviarEstado(numCliente);
  }
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
      enviarEstado(numCliente);               // al conectar, mandamos el estado
      break;
    }
    case WStype_DISCONNECTED:
      Serial.printf("Cliente %u desconectado\n", numCliente);
      break;
    case WStype_TEXT:
      procesarComando(numCliente, carga, longitud);
      break;
    default:
      break;
  }
}

// ---------------------------------------------------------------------------
// Lee los botones con antirrebote y avisa a la interfaz cuando se presionan
// ---------------------------------------------------------------------------
void revisarBotones() {
  if (millis() - marcaTiempoRebote < tiempoAntirrebote) return;
  marcaTiempoRebote = millis();

  int nivelBoot       = digitalRead(pinBotonBoot);
  int nivelReproducir = digitalRead(pinBotonReproducir);
  int nivelPausar     = digitalRead(pinBotonPausar);

  // Flanco de bajada = el boton pasa de reposo (HIGH) a presionado (LOW)
  if (nivelPrevioBoot == HIGH && nivelBoot == LOW)             avisarBoton("boot");
  if (nivelPrevioReproducir == HIGH && nivelReproducir == LOW) avisarBoton("reproducir");
  if (nivelPrevioPausar == HIGH && nivelPausar == LOW)         avisarBoton("pausar");

  nivelPrevioBoot       = nivelBoot;
  nivelPrevioReproducir = nivelReproducir;
  nivelPrevioPausar     = nivelPausar;
}

// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  // Configurar salidas
  for (int i = 0; i < cantidadSalidas; i++) {
    pinMode(pinesSalida[i], OUTPUT);
    digitalWrite(pinesSalida[i], LOW);
  }
  ledPlaca.begin();
  aplicarLedRgb(false);

  // Configurar botones con resistencia pull-up interna
  pinMode(pinBotonBoot,       INPUT_PULLUP);
  pinMode(pinBotonReproducir, INPUT_PULLUP);
  pinMode(pinBotonPausar,     INPUT_PULLUP);

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
  revisarBotones();        // revisa si se presiono algun boton
}
