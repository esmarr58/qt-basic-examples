//Esp32 3.0.5
//Arduino 1.8.19


#include <WiFi.h>
//#include <DHT.h>  //By adafruit 1.4.6 mas dependencias
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>  //By lacemra 3.1.0 con modificaciones
#include <AsyncWebSocket.h>
#include <ArduinoJson.h>  //BenoitBlanchon
#include <time.h>
#include <ESP32Servo.h>


//#define DHTPIN 7       // Pin donde está conectado el sensor
//#define DHTTYPE DHT22  // Cambia a DHT11 si usas ese modelo
#define ADC_PIN 13 

 #define tiempoAdelante 100
 #define tiempoAtras    100
 #define tiempoDerecha  50
 #define tiempoIzquierda 50
 
// Configura tus credenciales de WiFi
const char* ssid = "GWN571D04";
const char* password = "ESP32CUCEI$$";
//DHT dht(DHTPIN, DHTTYPE);
bool motorBActivado = false;

Servo miServo;        // Crear objeto tipo Servo
const int pinServo = 19;  // Pin donde está conectado el servo
int angulo = 90;         // Ángulo inicial

/*
const char* ssid = "GWN571D04";
const char* password = "ESP32CUCEI$$";
*/
// Configuración del servidor y WebSocket
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Pin del ADC sugerido
const int adcPin = 34;  // GPIO 34 es un pin ADC libre
const int blinkPin = 13; // GPIO 13 para el LED
const int numParts = 500;  // Número de partes de la señal
int i = 0;  // Contador para las 50 partes de la señal senoidal
int tiempoMuestreo = 1000;
volatile int velocidad = 0;

 bool motorActivado = false;
 long int tiempoMotorActivado = 0;
 long int tiempoMotor = 0;
int comandoNumerico = 0;
 

// Variables para manejar el envío periódico de datos
bool heartbeatReceived = false;  // Indica si ya se ha recibido el primer heartbeat
unsigned long lastAdcSendTime = 0;  // Tiempo de la última lectura del ADC

// Variables para simulación de señal
bool simulateSineWave = true;  // Bandera para activar la señal senoidal
float sineFrequency = 2.0;      // Frecuencia de la señal senoidal
unsigned long startTime = 0;    // Tiempo de inicio para la señal senoidal
volatile long tiempoBlink = 1000;
// Función para actualizar el tiempo de la ESP32
const char* comando;

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
                else if (strcmp(tipo, "motor") == 0) {
                comando = doc["comando"];
        
               if (strcmp(comando, "adelante") == 0) {
                    Serial.println("Motor adelante");
                    moverAdelante();
                    tiempoMotorActivado = millis();
                    motorBActivado = true;
                    comandoNumerico = 1;
                }
                else if (strcmp(comando, "atras") == 0) {
                    Serial.println("Motor atrás");
                    moverAtras();
                    tiempoMotorActivado = millis();
                    motorBActivado = true;
                    comandoNumerico = 2;
        
                }
                else if (strcmp(comando, "derecha") == 0) {
                    Serial.println("Motor derecha");
                    girarDerecha();
                    tiempoMotorActivado = millis();
                    motorBActivado = true;
                    comandoNumerico = 3;
        
                }
                else if (strcmp(comando, "izquierda") == 0) {
                    Serial.println("Motor izquierda");
                    girarIzquierda();
                    tiempoMotorActivado = millis();
                    motorBActivado = true;
                    comandoNumerico = 4;
        
                }
                else if (strcmp(comando, "paro") == 0) {
                    Serial.println("Motor detenido");
                    frenarMotores();
                }
            }

    // --- SERVOMOTOR ---
    else if (strcmp(tipo, "servo") == 0) {
        int angulo = doc["angulo"];
        Serial.print("Moviendo servo al ángulo: ");
        Serial.println(angulo);
              miServo.write(angulo);
             

    }

    // --- SENSOR ULTRASÓNICO ---
    else if (strcmp(tipo, "ultrasonico") == 0) {
        float distancia = medirDistancia();
        Serial.print("Distancia medida: ");
        Serial.print(distancia);
        Serial.println(" cm");
         // Crear un objeto JSON para enviar todas las lecturas
        DynamicJsonDocument doc(256);
        doc["type"] = "medicion_distancia";
        doc["distancia"] = distancia;

        // Serializar el JSON y enviarlo
        String jsonString;
        serializeJson(doc, jsonString);
        client->text(jsonString);

        // Log para ver los datos enviados
        Serial.print("Enviando datos: ");
        Serial.println(jsonString);
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
        //float humedad = dht.readHumidity();
        //float temperatura = dht.readTemperature();
        float humedad = 1.0;
        float temperatura = 1.0;

        if (isnan(humedad) || isnan(temperatura)) {
            Serial.println("Error al leer del sensor DHT!");
            return;  // Salir de la función si hay un error en el sensor
        }

        // Leer el valor del ADC y convertir a voltaje
        int adcValue = analogRead(ADC_PIN);
        adcVoltage = (adcValue / 4095.0) * 3.3;  // Convertir a voltaje (0-3.3V)

        // Crear un objeto JSON para enviar todas las lecturas
        DynamicJsonDocument doc(256);
        doc["type"] = "adc_reading";
        doc["temperature"] = temperatura;
        doc["humidity"] = humedad;
        doc["adc_value"] = adcVoltage;

        // Serializar el JSON y enviarlo
        String jsonString;
        serializeJson(doc, jsonString);
        client->text(jsonString);

        // Log para ver los datos enviados
        Serial.print("Enviando datos: ");
        Serial.println(jsonString);
    }
}

// Task para el parpadeo del LED en el core 1
void blinkTask(void *param) {
    long int ta = millis();
    long int tb = 0;
    bool estadoLed = false;
    //aqui
    pinMode(blinkPin, OUTPUT);
    while (1) {
        tb = millis();
        if((tb-ta)>tiempoBlink){
          ta = millis();
          digitalWrite(blinkPin, estadoLed);
          estadoLed = !estadoLed;
        }
        if(motorBActivado){

          tiempoMotor =  millis();
            //Hay un comando para procesar.
            if(comandoNumerico  == 1){
                if((tiempoMotor-tiempoMotorActivado)>tiempoAdelante){
                  frenarMotores();
                  Serial.println("ok Motor apagado adelante");
                  motorBActivado = false;

                }
            }
            else if(comandoNumerico == 2){
              if((tiempoMotor-tiempoMotorActivado)>tiempoAtras){
                  frenarMotores();
                  Serial.println("ok Motor apagado atras");
                  motorBActivado = false;
                  motorBActivado = false;

                }
            }
            else if(comandoNumerico == 3){
              if((tiempoMotor-tiempoMotorActivado)>tiempoDerecha){
                  frenarMotores();
                  Serial.println("ok Motor apagado der");
                  motorBActivado = false;

                }
            }
            else if(comandoNumerico == 4){
              if((tiempoMotor-tiempoMotorActivado)>tiempoIzquierda){
                  frenarMotores();
                  Serial.println("ok Motor apagado izq");
                  motorBActivado = false;

                }
            }else{
              Serial.println("Error-2");
            }
           


          
        }
      
   
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
    setupMotores();  
    frenarMotores();  
    ultrasonicoSetup();
    //dht.begin();
    miServo.setPeriodHertz(50);          // Frecuencia típica de los servos
   miServo.attach(pinServo, 500, 2400); // Pin, pulso mínimo y máximo en microsegundos
   miServo.write(angulo);  
  
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
