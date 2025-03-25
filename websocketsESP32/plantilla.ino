//Version 2
#include <WiFi.h>
//#include <DHT.h>  //By adafruit 1.4.6 mas dependencias
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>  //By lacemra 3.1.0 con modificaciones
#include <AsyncWebSocket.h>
#include <ArduinoJson.h>  //BenoitBlanchon
#include <time.h>
#include <ESP32Servo.h>

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char* ssid = "GWN571D04";
const char* password = "ESP32CUCEI$$";

// Variables para manejar el envío periódico de datos
bool heartbeatReceived = false;  // Indica si ya se ha recibido el primer heartbeat
unsigned long lastAdcSendTime = 0;  // Tiempo de la última lectura del ADC

void onWebSocketMessage(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_DATA) {
        Serial.print("Message length: ");
        Serial.println(len);  // Mostrar la longitud del mensaje

        if (len > 0) {
            // Convertir los datos recibidos en una cadena
            String message = String((char*)data);
            Serial.println("Mensaje recibido: " + message);

            // Crear un objeto DynamicJsonDocument para almacenar el JSON
            DynamicJsonDocument doc(1024);

            // Intentar deserializar el mensaje como JSON
            DeserializationError error = deserializeJson(doc, message);

            // Si el mensaje es un JSON válido
            if (!error) {
                Serial.println("Mensaje JSON válido recibido");

                // Comprobar si es un heartbeat
                const char* tipo = doc["type"];
            }
        }
    }
    else if (type == WS_EVT_CONNECT) {
        Serial.println("Client connected");
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.println("Client disconnected");
    }
}

void tarea1(void *param){
  while (1) {
        // Mantén limpios los clientes inactivos del WebSocket
        ws.cleanupClients();
        vTaskDelay(10 / portTICK_PERIOD_MS);  // Pequeño retardo para no saturar el procesador


  }
  
}

void tarea2(void *param){
  while(1)
  {
    delay(1000);
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // Conéctate a la red WiFi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Conectando a WiFi...");
    }
    Serial.println("Conectado a WiFi");

    // Imprime la IP asignada por el router
    Serial.print("IP asignada: ");
    Serial.println(WiFi.localIP());

     // Configura el WebSocket y asigna el manejador de eventos
    ws.onEvent(onWebSocketMessage);
    server.addHandler(&ws);

    // Inicia el servidor
    server.begin();
    Serial.println("Servidor WebSocket iniciado");

    xTaskCreatePinnedToCore(
        tarea1,       // Función de la tarea
        "WS ADC Task",   // Nombre de la tarea
        4096,            // Tamaño de la pila
        NULL,            // Parámetro de entrada (null en este caso)
        1,               // Prioridad
        NULL,            // Puntero a la tarea (no lo usamos)
        0);              // Ejecutar en el core 0
  
    // Crear la tarea para el blink en Core 1
    xTaskCreatePinnedToCore(
        tarea2,       // Función de la tarea
        "Blink Task",    // Nombre de la tarea
        2048,            // Tamaño de la pila
        NULL,            // Parámetro de entrada (null en este caso)
        1,               // Prioridad
        NULL,            // Puntero a la tarea (no lo usamos)
        1);              // Ejecutar en el core 1



}

void loop() {
  // put your main code here, to run repeatedly:

}
