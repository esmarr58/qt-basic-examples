//Esp32 3.0.5
//Arduino 1.8.19


#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>  //By lacemra 3.1.0 con modificaciones
#include <AsyncWebSocket.h>
#include <ArduinoJson.h>  //BenoitBlanchon
#include <time.h>

// Configura tus credenciales de WiFi
const char* ssid = "TP-Link_Extender";
const char* password = "1659884E";

// Configuración del servidor y WebSocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Pin del ADC sugerido
const int adcPin = 34;  // GPIO 34 es un pin ADC libre
const int blinkPin = 13; // GPIO 13 para el LED
const int numParts = 500;  // Número de partes de la señal
int i = 0;  // Contador para las 50 partes de la señal senoidal
int tiempoMuestreo = 1000;

// Variables para manejar el envío periódico de datos
bool heartbeatReceived = false;  // Indica si ya se ha recibido el primer heartbeat
unsigned long lastAdcSendTime = 0;  // Tiempo de la última lectura del ADC

// Variables para simulación de señal
bool simulateSineWave = true;  // Bandera para activar la señal senoidal
float sineFrequency = 2.0;      // Frecuencia de la señal senoidal
unsigned long startTime = 0;    // Tiempo de inicio para la señal senoidal

// Función para actualizar el tiempo de la ESP32
void updateEsp32Time(unsigned long timestamp) {
    timeval tv;
    tv.tv_sec = timestamp;
    tv.tv_usec = 0;
    settimeofday(&tv, nullptr);  // Actualizar el tiempo de la ESP32
    Serial.println("Tiempo de la ESP32 actualizado.");
}

// Función para manejar mensajes JSON
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
                if (strcmp(tipo, "heartbeat") == 0) {
                    // Obtener el timestamp del heartbeat
                    unsigned long timestamp = doc["timestamp"];
                    Serial.print("Heartbeat recibido con timestamp: ");
                    Serial.println(timestamp);

                    // Actualizar el tiempo de la ESP32
                    updateEsp32Time(timestamp);

                    // Marcar que se ha recibido el primer heartbeat
                    heartbeatReceived = true;
                }

                // Verificar si se solicita la simulación de señal senoidal
                if (doc.containsKey("simulate_sine")) {
                    simulateSineWave = doc["simulate_sine"];
                    Serial.print("Simulación de señal senoidal activada: ");
                    Serial.println(simulateSineWave);
                }
            } else {
                // Si el mensaje no es un JSON válido
                Serial.println("Mensaje no es un JSON válido, ignorando.");
                client->text("Error: Mensaje no es un JSON válido.");
            }
        } else {
            Serial.println("No data received.");
        }
    } else if (type == WS_EVT_CONNECT) {
        Serial.println("Client connected");
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.println("Client disconnected");
    }
}

// Función para enviar los datos del ADC o la señal senoidal en formato JSON
void sendAdcReading(AsyncWebSocketClient *client) {
    if (heartbeatReceived) {
        float adcVoltage;

        if (simulateSineWave) {
            // Calcular la señal senoidal
            unsigned long currentTime = millis() - startTime;
            float radians = 2 * PI * (float)i / (float)numParts;  // Convertir a radianes dividiendo entre partes

             adcVoltage = 1.65 + 1.65 * sin(radians);  // Señal senoidal entre 0 y 3.3V
             i++;  // Incrementar el contador
             if (i >= numParts) {
            i = 0;  // Reiniciar el ciclo de la señal
        }
        } else {
            // Leer el valor del ADC
            int adcValue = analogRead(adcPin);
            adcVoltage = (adcValue / 4095.0) * 3.3;  // Convertir a voltaje (asumiendo un rango de 0-3.3V)
        }

        // Obtener el timestamp actual de la ESP32
        unsigned long currentTimestamp = time(nullptr);

        // Crear un objeto JSON para enviar la lectura
        DynamicJsonDocument doc(256);
        doc["type"] = "adc_reading";
        doc["adc_value"] = adcVoltage;
        doc["timestamp"] = currentTimestamp;

        // Serializar el JSON y enviarlo
        String jsonString;
        serializeJson(doc, jsonString);
        client->text(jsonString);

        Serial.print("Enviando lectura: ");
        Serial.println(jsonString);
    }
}

// Task para el parpadeo del LED en el core 1
void blinkTask(void *param) {
    pinMode(blinkPin, OUTPUT);
    while (1) {
        digitalWrite(blinkPin, HIGH);
        vTaskDelay(500 / portTICK_PERIOD_MS);  // Encender por 500 ms
        digitalWrite(blinkPin, LOW);
        vTaskDelay(500 / portTICK_PERIOD_MS);  // Apagar por 500 ms
    }
}

// Task para manejar el loop del WebSocket y ADC en el core 0
void wsAdcTask(void *param) {
    while (1) {
        // Mantén limpios los clientes inactivos del WebSocket
        ws.cleanupClients();

        // Verificar si ya se ha recibido el primer heartbeat y enviar lecturas cada 100 ms
        if (heartbeatReceived && (millis() - lastAdcSendTime >= tiempoMuestreo)) {
            lastAdcSendTime = millis();  // Actualizar el tiempo de la última lectura
            // Enviar lectura a los clientes conectados
            for (AsyncWebSocketClient *client : ws.getClients()) {
                if (client->status() == WS_CONNECTED) {
                    sendAdcReading(client);
                }
            }
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);  // Pequeño retardo para no saturar el procesador
    }
}

void setup() {
    // Inicializa el puerto serial
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

    // Configurar el pin ADC
    pinMode(adcPin, INPUT);
    startTime = millis();  // Inicializa el tiempo para la señal senoidal

    // Crear la tarea para el WebSocket y ADC en Core 0
    xTaskCreatePinnedToCore(
        wsAdcTask,       // Función de la tarea
        "WS ADC Task",   // Nombre de la tarea
        4096,            // Tamaño de la pila
        NULL,            // Parámetro de entrada (null en este caso)
        1,               // Prioridad
        NULL,            // Puntero a la tarea (no lo usamos)
        0);              // Ejecutar en el core 0

    // Crear la tarea para el blink en Core 1
    xTaskCreatePinnedToCore(
        blinkTask,       // Función de la tarea
        "Blink Task",    // Nombre de la tarea
        2048,            // Tamaño de la pila
        NULL,            // Parámetro de entrada (null en este caso)
        1,               // Prioridad
        NULL,            // Puntero a la tarea (no lo usamos)
        1);              // Ejecutar en el core 1
}

void loop() {
    // No hacemos nada en el loop, ya que todas las tareas corren en FreeRTOS
}
