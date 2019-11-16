#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <SocketIoClient.h>
#include <ArduinoJson.h>

#define USE_SERIAL Serial

//defines - mapeamento de pinos do NodeMCU
#define D0    16
#define D1    5
#define D2    4
#define D3    0
#define D4    2
#define D5    14
#define D6    12
#define D7    13
#define D8    15
#define D9    3
#define D10   1

ESP8266WiFiMulti WiFiMulti;
SocketIoClient webSocket;

void event(const char * payload, size_t length) {
  USE_SERIAL.printf("got message: %s\n", payload);
}

const int selectPins[3] = {0, 4, 5}; // S0~D3, S1~D2, S2~D1
const int zInput = A0; // Connect common (Z) to A0 (analog input)

float Vo =  0,
      Vi = 0, 
      R1 = 0, 
      R2 = 10000, 
      A = 0,
      Vsen = 0;
int i = 0;

void setup() {
    USE_SERIAL.begin(115200);

    USE_SERIAL.setDebugOutput(true);

    USE_SERIAL.println();
    USE_SERIAL.println();
    USE_SERIAL.println();

      for(uint8_t t = 4; t > 0; t--) {
          USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
          USE_SERIAL.flush();
          delay(1000);
      }

    WiFiMulti.addAP("VISITORS", "nossanet1000");

    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
    }

    webSocket.on("event", event);
    webSocket.begin("192.168.0.11", 3000);

    for (int i=0; i<3; i++) {
        pinMode(selectPins[i], OUTPUT);
        digitalWrite(selectPins[i], HIGH);
    }
}

void loop() {      
    // Loop through all eight pins.
    for (byte pin=0; pin<=7; pin++) {
        for (int i=0; i<3; i++) {
            digitalWrite(selectPins[i], pin & (1<<i)?HIGH:LOW);
        }
        
        int inputValue = analogRead(zInput);
        float volt = (inputValue*(3.3/1023));
        
        if (pin == 4) {
            //USE_SERIAL.print("Vo: " + String(volt) + "\t");
            Vo = volt;
        } else if (pin == 7) {
            //USE_SERIAL.print("Vi: " + String(volt) + "\t");
            Vi = volt; 
        } else if (pin == 6) {
            //USE_SERIAL.print("Vsen: " + String(volt) + "\t");
            Vsen = volt;
        }
    }
    A = (Vo/Vi);
    R1 = (R2/(A-1));
    //USE_SERIAL.print("A: " + String(A) + "\t");
    //USE_SERIAL.print("R1: " + String(R1) + "\t"); 
    StaticJsonDocument<200> msg;
    char mensage[200];
    
    msg["saidaLM"] = String(Vo);
    msg["entraLM"] = String(Vi);
    msg["ganhoLM"] = String(A);
    msg["valueR1"] = String(R1);

    serializeJson(msg, mensage);
    
    //USE_SERIAL.println();
    //"{\"saidaLM\":\"bar\", \"entraLM\":\"bar\", \"saidaSensor\":\"bar\", \"ganhoLM\":\"bar\", \"valueR1\":\"bar\"}"
    webSocket.emit("sendMessage", mensage);
    delay(1500);
    webSocket.loop();
}

