/*
 * ============================================================================
 *  Clase 17 - Ejemplo 4 - Diseño de Interfaces (I7262) - ESP32-S3-DevKitC-1
 * ----------------------------------------------------------------------------
 *  LOTES de muestras con marca de tiempo EPOCH, por WebSocket. Igual que en el
 *  TC4, la ESP32 es el SERVIDOR WebSocket (puerto 81) y Qt es el CLIENTE.
 *
 *  Mide la distancia cada intervaloMuestra ms y junta muestrasPorLote muestras
 *  en UN solo mensaje JSON. La ESP32 NO sabe la hora real: cuenta desde que
 *  arranco. La interfaz de Qt se la dice con el comando "sincronizar".
 *
 *  usarSensorReal = false -> distancia simulada (sin hardware)
 *  usarSensorReal = true  -> HC-SR04 real (TRIG = GPIO 12, ECHO = GPIO 13, como la Practica #2)
 *
 *  Librerias (las mismas del TC4):
 *   - WebSockets  (Markus Sattler / Links2004)  -> servidor WebSocket
 *   - ArduinoJson (Benoit Blanchon, v7)          -> armar/leer JSON
 *
 *  Placa (Arduino IDE 1.8.19): "ESP32S3 Dev Module".
 *
 *  PROTOCOLO JSON  (mismo esquema del TC4: campo "tipo")
 *  --------------
 *  Computadora  ->  ESP32 (comandos):
 *     {"tipo":"sincronizar", "epoch_ms":1792069200000}   // la hora real, en ms desde 1970 (UTC)
 *
 *  ESP32  ->  Computadora:
 *     {"tipo":"saludo",       "mensaje":"...", "puerto":81}
 *     {"tipo":"sincronizado", "epoch_ms":1792069200000}
 *     {"tipo":"lote", "seq":7, "t0":1792069200123, "dt":60, "sinc":true,
 *      "dist":[37.4, 37.9, null, 38.6, ...]}
 *
 *       seq  : numero de lote (0,1,2,...) -> Qt detecta lotes perdidos
 *       t0   : epoch en MILISEGUNDOS de la primera muestra del lote
 *       dt   : milisegundos entre muestras -> la muestra i ocurrio en t0 + i*dt
 *       sinc : false mientras Qt no haya mandado la hora real
 *       dist : distancias en cm; null = lectura fallida (JSON no acepta NaN)
 * ============================================================================
 */

#include <WiFi.h>
#include <WebSocketsServer.h>   // WebSockets (Links2004)
#include <ArduinoJson.h>        // ArduinoJson v7 (Benoit Blanchon)
#include "esp_timer.h"          // reloj de 64 bits (microsegundos desde el arranque)

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
// 3) Sensor ultrasonico (mismos pines que la Practica #2)
// ---------------------------------------------------------------------------
const bool usarSensorReal = false;   // <-- true para usar el HC-SR04
const int  pinTrig = 12;
const int  pinEcho = 13;

// ---------------------------------------------------------------------------
// 4) Lote de muestras
// ---------------------------------------------------------------------------
const int intervaloMuestra = 60;     // ms entre muestras (el HC-SR04 pide >= 60 ms)
const int muestrasPorLote  = 10;     // 10 x 60 ms = un lote cada 0.6 s

float    lote[muestrasPorLote];
int      muestrasEnLote = 0;
int64_t  t0Lote = 0;                 // epoch (ms) de la primera muestra del lote
uint32_t numeroLote = 0;             // "seq"
int64_t  proximaMuestra = 0;         // ms desde el arranque de la siguiente muestra

// ---------------------------------------------------------------------------
// 5) Hora real: epoch = desfaseEpoch + ms desde el arranque
// ---------------------------------------------------------------------------
int64_t desfaseEpoch = 0;            // sin sincronizar = 0 -> fechas de 1970
bool    sincronizado = false;

// Milisegundos desde el arranque en 64 bits: no se desborda como millis() (~49.7 dias)
int64_t msDesdeArranque() {
  return esp_timer_get_time() / 1000;
}

int64_t epochMs(int64_t msArranque) {
  return desfaseEpoch + msArranque;
}

// ---------------------------------------------------------------------------
// (1) Medir: HC-SR04 real o distancia simulada
// ---------------------------------------------------------------------------
float medirDistancia() {
  if (usarSensorReal) {
    digitalWrite(pinTrig, LOW);  delayMicroseconds(2);
    digitalWrite(pinTrig, HIGH); delayMicroseconds(10);      // pulso de disparo
    digitalWrite(pinTrig, LOW);
    unsigned long duracion = pulseIn(pinEcho, HIGH, 25000);  // timeout ~4 m
    if (duracion == 0) return NAN;                           // sin eco
    return duracion * 0.034 / 2.0;
  }
  float segundos = msDesdeArranque() / 1000.0;
  float distancia = 60.0 + 40.0 * sin(2.0 * PI * segundos / 8.0);   // va y viene cada 8 s
  return distancia + random(-10, 11) / 10.0;                         // +-1 cm de ruido
}

// ---------------------------------------------------------------------------
// (2) Serializar + (3) Enviar el lote completo a TODOS los clientes
// ---------------------------------------------------------------------------
void enviarLote() {
  JsonDocument documento;
  documento["tipo"] = "lote";
  documento["seq"]  = numeroLote;
  documento["t0"]   = t0Lote;           // int64_t: 13 digitos
  documento["dt"]   = intervaloMuestra;
  documento["sinc"] = sincronizado;
  JsonArray distancias = documento["dist"].to<JsonArray>();
  for (int i = 0; i < muestrasPorLote; i++) {
    if (isnan(lote[i])) distancias.add(nullptr);            // -> null
    else                distancias.add(round(lote[i] * 10) / 10.0);
  }

  String texto;
  serializeJson(documento, texto);
  servidorSocket.broadcastTXT(texto);   // un mensaje = un lote
  numeroLote++;
}

// ---------------------------------------------------------------------------
// Interpreta un comando JSON recibido desde la interfaz de Qt
// ---------------------------------------------------------------------------
void procesarComando(uint8_t numCliente, uint8_t* carga, size_t longitud) {
  JsonDocument documento;
  DeserializationError error = deserializeJson(documento, carga, longitud);
  if (error) {
    Serial.println("JSON invalido, se ignora.");
    servidorSocket.sendTXT(numCliente, "{\"tipo\":\"error\",\"detalle\":\"json_invalido\"}");
    return;
  }

  const char* tipoMsg = documento["tipo"];
  if (tipoMsg == nullptr) return;

  if (strcmp(tipoMsg, "sincronizar") == 0) {
    int64_t epochQt = documento["epoch_ms"] | (int64_t)0;
    desfaseEpoch = epochQt - msDesdeArranque();
    sincronizado = true;
    muestrasEnLote = 0;               // el lote a medias tenia la hora vieja: se descarta

    JsonDocument respuesta;
    respuesta["tipo"]     = "sincronizado";
    respuesta["epoch_ms"] = epochQt;
    String texto;
    serializeJson(respuesta, texto);
    servidorSocket.sendTXT(numCliente, texto);
    Serial.println("Hora sincronizada desde Qt.");
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
      JsonDocument saludo;
      saludo["tipo"]    = "saludo";
      saludo["mensaje"] = "Ejemplo 4: lotes con epoch por WebSocket";
      saludo["puerto"]  = 81;
      String textoSaludo;
      serializeJson(saludo, textoSaludo);
      servidorSocket.sendTXT(numCliente, textoSaludo);
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
void setup() {
  Serial.begin(115200);
  if (usarSensorReal) {
    pinMode(pinTrig, OUTPUT);
    pinMode(pinEcho, INPUT);
  }

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

  proximaMuestra = msDesdeArranque();
}

// ---------------------------------------------------------------------------
void loop() {
  servidorSocket.loop();   // atiende a los clientes WebSocket

  if (msDesdeArranque() < proximaMuestra) return;
  int64_t instanteMuestra = proximaMuestra;   // instante NOMINAL de esta muestra
  proximaMuestra += intervaloMuestra;         // ritmo fijo: dt constante

  if (muestrasEnLote == 0) t0Lote = epochMs(instanteMuestra);   // hora de la PRIMERA muestra
  lote[muestrasEnLote++] = medirDistancia();                    // (1) medir y acumular

  if (muestrasEnLote == muestrasPorLote) {
    enviarLote();
    muestrasEnLote = 0;
  }
}
