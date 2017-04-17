/* This Master code incorporates the float sensors, temperature sensors, and fish feeder control in order
to gather data that can be used to monitor the aquaponics system on 4th floor Sullivan. Created by Shane and Matthew
Project: RoofTopGarden */
#include <UbidotsMicroESP8266.h>
#include "HTTPSRedirect.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#define ID1 "5890e3fc76254253b40c38b5"
#define ID2 "588baa1e762542036842ef85"
#define ID3 "5890e63476254253b40c4d2c"
#define ID4 "589e1b3e76254210a06ba5db"
#define TOKEN "wemRu3qNO3DyZHDLKkNMhQboTOXH3O" // Put here your Ubidots TOKEN
#define WIFISSID "robotics3"
#define PASSWORD "Memde=Boss"
#define ONE_WIRE_BUS D1
#define SensorPin A0

OneWire oneWire (ONE_WIRE_BUS);

DallasTemperature sensor(&oneWire);

Ubidots client1(TOKEN);


float value;
float phValue;
int f_one = D2;
int f_two = D3;
int f_three = D7;
int f_four = D5;

int floatValue1;
int floatValue2;
int floatValue3;
int floatValue4;
const char *GScriptId = "AKfycbyADaLgXg7z5jGJZCI5vA0eaqvZslhVr2qLIveNxy4ULRgCpb40";

// Push data on this interval
const int dataPostDelay = 300000;  // 15 minutes = 15 * 60 * 1000

const char* host = "script.google.com";
const char* googleRedirHost = "script.googleusercontent.com";

const int httpsPort =  443;
HTTPSRedirect client(httpsPort);

// Prepare the url (without the varying data)
String url = String("/macros/s/") + GScriptId + "/exec?";

const char* fingerprint = "F0 5C 74 77 3F 6B 25 D7 3B 66 4D 43 2F 7E BC 5B E9 28 86 AD";

void setup() {
  Serial.begin(115200);
  delay(10);
  client1.wifiConnection(WIFISSID, PASSWORD);
  pinMode(D2, INPUT_PULLUP);
  pinMode(D3, INPUT_PULLUP);
  pinMode(D7, INPUT_PULLUP);
  pinMode(D5, INPUT_PULLUP);
  pinMode(D6, OUTPUT);
  Serial.println("Connecting to wifi: ");
  Serial.println(ssid);
  Serial.flush();

  Serial.print(String("Connecting to "));
  Serial.println(host);

  bool flag = false;
  for (int i=0; i<5; i++){
    int retval = client.connect(host, httpsPort);
    if (retval == 1) {
       flag = true;
       break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }

  // Connection Status, 1 = Connected, 0 is not.
  Serial.println("Connection Status: " + String(client.connected()));
  Serial.flush();
  
  if (!flag){
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    Serial.println("Exiting...");
    Serial.flush();
    return;
  }

  // Data will still be pushed even certification don't match.
  if (client.verify(fingerprint, host)) {
    Serial.println("Certificate match.");
  } else {
    Serial.println("Certificate mis-match");
  }
}

void loop() {

  floatValue1 = digitalRead(f_one);
  floatValue2 = digitalRead(f_two);
  floatValue3 = digitalRead(f_three);
  floatValue4 = digitalRead(f_four);
  high_lvl1();
  high_lvl2();
  normal();
  low_lvl1();
  low_lvl2();
  phSensor();
  float value3 = client1.getValue(ID3);
  Serial.println(value3);
  digitalWrite(D6, LOW);
  
 
  float value2 = getTemp();
  client1.add(ID1, value);
  client1.add(ID2, value2);
  client1.add(ID4, phValue);
  client1.sendAll();

  postData(value, value2, phValue);
}

float getTemp(){
  sensor.requestTemperatures();
  float temp = sensor.getTempCByIndex(0);
  if(temp > -127 && temp <50){
    return temp;
  }
}

void normal(){
  if(floatValue1 == 0 && floatValue2 == 0 && floatValue3 == 1 && floatValue4 == 1){
    value = 3;
  }
}

void high_lvl1(){
  if(floatValue1 == 0 && floatValue2 == 1 && floatValue3 == 1){
    value = 2;
  }
}

void high_lvl2(){
  if(floatValue1 == 1){
    value = 1;
  }
}

void low_lvl1(){
  if(floatValue1 == 0 && floatValue2 == 0 && floatValue3 == 0 && floatValue4 == 1){
    value = 4;
  }
}

void low_lvl2(){
  if(floatValue4 == 0){
    value = 5;
  }
}
void phSensor(){
  phValue = analogRead(SensorPin);
  phValue = phValue/94.6;
}

void postData(float fsensor, float temps, float pHs){
  if (!client.connected()){
    Serial.println("Connecting to client again..."); 
    client.connect(host, httpsPort);
  }
  String urlFinal = url + "fsensor=" + String(fsensor) + "&temps=" + String(temps) + "&pHs=" + String(pHs);
  client.printRedir(urlFinal, host, googleRedirHost);
}

/*
Scenarios:

(1)high level 2;
com1 = high
com2 = high
com3 = high
com4 = high

(2)High level 1: com1 = low
com2 = high
com3 = high
com4 = high

(3)Normal
com1 = low
com2 = low
com3 = high
com4 = high

(4)Low level1;
com1 = low
com2 = low
com3 = low
com4 = high

(5)low level 2: com1 = low
com2 = low
com3 = low
com4 = low

*/

/* Pin Layout: D7: Free
D1: Temp D2:Float1
D3:Flaot2 D7:Float3
D5:Float4 D6:Fish Feeder D8: Free
AD7: (Analog) Ph 
*/
