/*
Dr. Ruben Estrada Marmolejo.
2024.

ESP32: 3.0.5
JSON: 7.1.0
*/

#include "BluetoothSerial.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include "esp_system.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ESP32Servo.h>  // Librería para control del servomotor

#define SERVICE_UUID           "7271d0aa-81d1-4c4d-aa45-2eaf95d2c7dd"
#define CHARACTERISTIC_UUID_RX "7271d0ab-81d1-4c4d-aa45-2eaf95d2c7dd"
#define CHARACTERISTIC_UUID_TX "7271d0ac-81d1-4c4d-aa45-2eaf95d2c7dd"

BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

WiFiServer servidor(80);
char ssid[100]     = "GWN571D04";
char password[100] = "ESP32CUCEI$$";
bool modoServidor = true;
long int tiempoInicio = 0;
long int tiempoFinal = 0;
long int tiempoConexionInicio = 0;
long int tiempoComparacion = 0;
int contador = 0;
String currentLine = "";
bool finMensaje = false;

// Configuración del servomotor
#define SERVO_PIN 14  // Pin donde se conecta el servomotor
Servo servoMotor;     // Objeto para el servomotor

// Callbacks del servidor BLE
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

void conectaWiFi() {
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("MAC address: ");
    Serial.println(WiFi.macAddress());

    if (!MDNS.begin("sensor1")) {
        Serial.println("Error al iniciar MDNS");
    } else {
        servidor.begin();
        MDNS.addService("http", "tcp", 80);
    }
}

void setup() {
    Serial.begin(115200);

    // Configurar el servomotor
    servoMotor.attach(SERVO_PIN, 500, 2500); // Rango de pulso adecuado para ESP32

    // Configuración del Bluetooth BLE
    BLEDevice::init("UART Service");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);
    pTxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_TX,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pTxCharacteristic->addDescriptor(new BLE2902());

    BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
        BLECharacteristic::PROPERTY_WRITE
    );

    // Iniciar el servicio BLE
    pService->start();
    pServer->getAdvertising()->start();
    conectaWiFi();
    tiempoInicio = millis();
}

void loop() {
    // Manejo de conexión BLE
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);
        pServer->startAdvertising();
        Serial.println("Start advertising");
        oldDeviceConnected = deviceConnected;
    }
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
    }

    if (modoServidor) {
        currentLine = "";
        WiFiClient clienteServidor = servidor.available();
        finMensaje = false;

        if (clienteServidor) {
            tiempoConexionInicio = xTaskGetTickCount();

            while (clienteServidor.connected()) {
                if (clienteServidor.available() > 0) {
                    char c = clienteServidor.read();
                    Serial.print(c);  // Mostrar caracteres recibidos

                    if (c == '}') { finMensaje = true; }

                    if (c == '\n') {
                        if (currentLine.length() == 0) {
                            // Respuesta HTTP
                        } else {
                            currentLine = "";
                        }
                    } else if (c != '\r') {
                        currentLine += c;
                    }

                    if (finMensaje) {
                        String mensajeJSON = currentLine;
                        Serial.println("\nMensaje JSON recibido:");
                        Serial.println(mensajeJSON);

                        JsonDocument doc;
                        DeserializationError error = deserializeJson(doc, mensajeJSON);
                        if (error) {
                            Serial.println("Error al parsear JSON");
                            continue;
                        }

                        // Identificar el tipo de dispositivo
                        const char* dispositivo = doc["dispositivo"];
                        if (strcmp(dispositivo, "foco") == 0) {
                            // Manejo del foco
                            int numeroPin = doc["pin"];
                            const char* estado = doc["estado"];

                            pinMode(numeroPin, OUTPUT);
                            if (strcmp(estado, "on") == 0) {
                                digitalWrite(numeroPin, HIGH);
                            } else {
                                digitalWrite(numeroPin, LOW);
                            }
                            Serial.print("Foco en el pin ");
                            Serial.print(numeroPin);
                            Serial.print(" encendido: ");
                            Serial.println(strcmp(estado, "on") == 0 ? "Sí" : "No");

                        } else if (strcmp(dispositivo, "servo") == 0) {
                            // Manejo del servomotor
                            int angulo = doc["angulo"];
                            if (angulo >= 0 && angulo <= 180) {
                                servoMotor.write(angulo);
                                Serial.print("Servo movido a ");
                                Serial.print(angulo);
                                Serial.println(" grados");
                            } else {
                                Serial.println("Ángulo fuera de rango");
                            }
                        }

                        // Respuesta al cliente
                        clienteServidor.println("HTTP/1.1 200 OK");
                        clienteServidor.println("Content-type:text/html");
                        clienteServidor.println();
                        clienteServidor.println("Configuracion Recibida");
                        clienteServidor.println();
                        break;
                    }

                    tiempoComparacion = xTaskGetTickCount();
                    if (tiempoComparacion > (tiempoConexionInicio + 3000)) {
                        Serial.println("Error timeout");
                        break;
                    }
                }
            }
            clienteServidor.stop();
            delay(500);
        }
    } else {
        tiempoFinal = millis();
        if (tiempoFinal > (tiempoInicio + 10000)) {
            tiempoInicio = millis();
        }
    }
}
