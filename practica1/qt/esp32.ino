/*
Dr. Ruben Estrada Marmolejo.
2024.

ESP32: 3.0.4
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


#define SERVICE_UUID           "7271d0aa-81d1-4c4d-aa45-2eaf95d2c7dd" // UART service UUID
#define CHARACTERISTIC_UUID_RX "7271d0ab-81d1-4c4d-aa45-2eaf95d2c7dd"
#define CHARACTERISTIC_UUID_TX "7271d0ac-81d1-4c4d-aa45-2eaf95d2c7dd"

#define adelanteTiempo 250
#define atrasTiempo 250
#define derechaTiempo 150
#define izquierdaTiempo 150

BLEServer *pServer = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t txValue = 0;


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

int motor1A = 12;
int motor1B = 11;
int motor2A = 10;
int motor2B = 9;

long int tiempoMotorInicio  = 0;
long int tiempoMotorFin     = 0;
bool     motorActivado = false;
bool     adelante = false;
bool     atras    = false;
bool     derecha  = false;
bool     izquierda = false;

void pararMotor(){
  digitalWrite(motor1A, 0);
  digitalWrite(motor1B, 0);
  digitalWrite(motor2A, 0);
  digitalWrite(motor2B, 0);
}
void configurarMotores(){
  pinMode(motor1A, OUTPUT);
  pinMode(motor1B, OUTPUT);
  pinMode(motor2A, OUTPUT);
  pinMode(motor2B, OUTPUT);
  pararMotor();
}

void motoresIzquierda(){
  digitalWrite(motor1A, HIGH);
  digitalWrite(motor1B, LOW);
  digitalWrite(motor2A, HIGH);
  digitalWrite(motor2B, LOW);
}
void motoresDerecha(){
  digitalWrite(motor1A, LOW);
  digitalWrite(motor1B, HIGH);
  digitalWrite(motor2A, LOW);
  digitalWrite(motor2B, HIGH);
}
void motoresAdelante(){
  digitalWrite(motor1A, HIGH);
  digitalWrite(motor1B, LOW);
  digitalWrite(motor2A, LOW);
  digitalWrite(motor2B, HIGH);
}
void motoresAtras(){
  digitalWrite(motor1A, LOW);
  digitalWrite(motor1B, HIGH);
  digitalWrite(motor2A, HIGH);
  digitalWrite(motor2B, LOW);
}






class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      // Obtener el valor como String de Arduino
      String rxValueArduino = pCharacteristic->getValue(); 
      
      // Convertir el valor a std::string
      std::string rxValueStd = std::string(rxValueArduino.c_str());  

      // Verificar si el valor recibido tiene contenido
      if (rxValueStd.size() > 0) {
        /*
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValueStd.length(); i++)
          Serial.print(rxValueStd[i]);

        Serial.println();
        Serial.println("*********");
        */

        // Verificar si el primer carácter del valor recibido es '1'
        if(rxValueStd[0] == '1'){
          // Obtener la IP local como String de Arduino
          String IP = WiFi.localIP().toString();
       
          // Verificar si el dispositivo está conectado y enviar la IP
          if (deviceConnected) {
            pTxCharacteristic->setValue(IP.c_str());  // Convertir IP a una cadena estilo C
            pTxCharacteristic->notify();  // Notificar el valor al cliente BLE
          }
        }
      }
    }
};



void conectaWiFi(){
  
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
  
    WiFi.begin(ssid,password);
         
       

     
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
    //http://sensor1.local
    if (!MDNS.begin("sensor1")) {
      
    }
   else{
   servidor.begin();
   MDNS.addService("http", "tcp", 80); 
   } 
}



void conexionURL(){

  bool servidorOk = true;
  char mensajeJSON[100];
  char temporal[50];
  char mensajeHTML[400];
  char urlVar[10]  = "/";
  char Servidor[25]  = "192.168.43.112";
  char Usuario[10]  = "bot33";
  int j = 0;
 for(j = 0; j<=399; j++){
  mensajeHTML[j] = '\0';
 }
 for(j = 0; j<=99; j++){
  mensajeJSON[j] = '\0';
 }
  
  //Crear el mensaje JSON
  
  sprintf(mensajeJSON, "{\"var\":\"%i\",\"key\":\"sdfsdf\"}", contador);  
    contador++;

  int cuantosBytes = strlen(mensajeJSON);
  
  sprintf(temporal, "POST %s HTTP/1.0\r\n", urlVar);  
  strcat(mensajeHTML, temporal);
  
  sprintf(temporal, "Host: %s \r\n", Servidor);
  strcat(mensajeHTML, temporal);

  sprintf(temporal, "User-Agent: %s \r\n", Usuario);
  strcat(mensajeHTML, temporal);

  sprintf(temporal,"Content-Length: %i \r\n", cuantosBytes);
  strcat(mensajeHTML, temporal);

  strcat(mensajeHTML, "Content-Type: application/json\r\n");
  strcat(mensajeHTML, "\r\n");
  strcat(mensajeHTML, mensajeJSON);

  //Serial.println(mensajeHTML);
  int cuantosMensaje = strlen(mensajeHTML); 
  if(servidorOk){
  WiFiClient client;
   if (client.connect("192.168.43.112", 80)) {
     Serial.print("Bytes ara transmitir: ");
    Serial.println(cuantosMensaje);
    for(j = 0; j<=cuantosMensaje-1; j++){
      client.print(mensajeHTML[j]);
      Serial.print(mensajeHTML[j]);
      

    }
    Serial.println(" ");
      delay(1000);
      client.stop();
    }
   else{
    Serial.println("Error de conexion con el servidor");
   }
  }
  else{
    Serial.print("Bytes ara transmitir: ");
    Serial.println(cuantosMensaje);
    for(j = 0; j<=cuantosMensaje-1; j++){
      Serial.print(mensajeHTML[j]);
    }
    Serial.println(" ");
  }  
}
void setup() {
  Serial.begin(115200);
  configurarMotores();
  // Create the BLE Device
  BLEDevice::init("UART Service");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pTxCharacteristic = pService->createCharacteristic(
                    CHARACTERISTIC_UUID_TX,
                    BLECharacteristic::PROPERTY_NOTIFY
                  );
                      
  pTxCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                       CHARACTERISTIC_UUID_RX,
                      BLECharacteristic::PROPERTY_WRITE
                    );

  pRxCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  conectaWiFi();
  tiempoInicio = millis();
}


void loop() {
  if(motorActivado){
    tiempoMotorFin = millis();
    if(adelante){
      
      if((tiempoMotorFin-tiempoMotorInicio)>adelanteTiempo){
          motorActivado = false;
          adelante = false;
          pararMotor();
          
      }
    }
    else if(atras){
      if((tiempoMotorFin-tiempoMotorInicio)>atrasTiempo){
          motorActivado = false;
          atras = false;
          pararMotor();
          
      }
      
    }
    else if(izquierda){
      if((tiempoMotorFin-tiempoMotorInicio)>izquierdaTiempo){
          motorActivado = false;
          izquierda = false;
          pararMotor();
          
      }
      
    }
    else if(derecha){
      if((tiempoMotorFin-tiempoMotorInicio)>derechaTiempo){
          motorActivado = false;
          derecha = false;
          pararMotor();
          
      }
      
    }
    
  }
  
  //Para el bluetooth
  // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
  
  
  
  if(modoServidor){
   currentLine = " ";
   WiFiClient clienteServidor = servidor.available();
   finMensaje = false;
   if (clienteServidor) {
   tiempoConexionInicio = xTaskGetTickCount();
    
    while(clienteServidor.connected()){

      if(clienteServidor.available() > 0){
        char c = clienteServidor.read();             // read a byte, then
          //Quiero recibir un mensaje en formato JSON
          //{"NombreVariable":"ValorVariable", "NombreVariable2":"ValorVariable2"}
          //Ejemplo enviare desde el cliente QT:
          //{"pin":"27", "estado":"0"}
          //{"pin":"14", "estado":"0"}
          //{"pin":"27", "estado":"1"}
          //{"pin":"14", "estado":"1"}

          Serial.print((char)c);                // print it out the serial monitor

          ///Recibe mensaje JSON - Inicio
        if(c == '}'){ finMensaje = true; }
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
                //Inicia la respuesta
  /*
            
                //Termina la respuesta
                */
                
           
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
        
        if(finMensaje){
          String mensajeJSON = currentLine;
          //Serial.println(mensajeJSON);
            //Decodificacion mensaje JSON - inicio
            //StaticJsonBuffer<100> bufferJSON;
            //JsonObject& objetoJSON = bufferJSON.parseObject(mensajeJSON);
            JsonDocument doc;
            deserializeJson(doc, mensajeJSON);
            /*
              int numeroPin = doc["pin"];  
              int estadoPin = doc["estado"];
                pinMode(numeroPin, OUTPUT);
                digitalWrite(numeroPin, estadoPin);
             */ 
             int comando = doc["comando"];  
             if(comando == 1){
                motorActivado = true;
                adelante = true;
                tiempoMotorInicio = millis();
                motoresAdelante();
             }
             else if(comando == 2){
                motorActivado = true;
                atras = true;
                tiempoMotorInicio = millis();
                motoresAtras();
             }
             else if(comando == 3){
                motorActivado = true;
                derecha = true;
                tiempoMotorInicio = millis();
                motoresDerecha();
             }
             else if(comando == 4){
                motorActivado = true;
                izquierda = true;
                tiempoMotorInicio = millis();
                motoresIzquierda();
             }
             

            //Decodificacion mensaje JSON - fin
            

          
          
            clienteServidor.println("HTTP/1.1 200 OK");
            clienteServidor.println("Content-type:text/html");
            clienteServidor.println();

            // the content of the HTTP response follows the header:
            clienteServidor.println("Configuracion Recibida");

            // The HTTP response ends with another blank line:
            clienteServidor.println();
          
          //resuestaServidor(); 
                     
            

          break;
          
        }
        
       
        //resuestaServidor();
         
        //Termina recibir mensaje JSON
        tiempoComparacion =  xTaskGetTickCount();
        if(tiempoComparacion > (tiempoConexionInicio + 3000)){
            //clienteServidor.stop();
            Serial.println("Error timeout");
            break;

        }

      }
              
    }
            clienteServidor.stop();
    //delay(500);
            
    
   }
  }
  else{
    tiempoFinal = millis();
  if(tiempoFinal > (tiempoInicio+10000)){
    tiempoInicio = millis();
    //Este if, funcionara cada 2 segundos.
    conexionURL();
    
  }
  }

}
