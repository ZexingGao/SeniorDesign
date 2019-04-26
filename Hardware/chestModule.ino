int state = 0;

#include "Adafruit_FONA.h"

#define FONA_RX 3
#define FONA_TX 4
#define FONA_RST 5
#define USE_ARDUINO_INTERRUPTS true

char replybuffer[255];

float latitude, longitude;

const int PulseWire = 3;
int Threshold = 550;

#include <PulseSensorPlayground.h>
#include <SoftwareSerial.h>
//SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial fonaSS(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
PulseSensorPlayground pulseSensor;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);

  pulseSensor.begin();
  
  fonaSerial->begin(4800);
  if(!fona.begin(*fonaSerial)){
    while(1);
  }
  fona.enableGPRS(true);
  initGPS();
  digitalWrite(2, HIGH);
  delay(2000);
  digitalWrite(2, LOW);
  Serial.write('9');
  
}

void initGPS(){
  while(true){
    delay(2000);
    if(fona.getNetworkStatus() == 1){
      bool gsmLocSuccess = fona.getGSMLoc(&latitude, &longitude);

      if(gsmLocSuccess){
        break;
      }else{
        fona.enableGPRS(false);
        if(!fona.enableGPRS(true)){
          delay(100);
        }
      }
    }
  }
}

int counter = 0;

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial.available()>0){
    state = Serial.read();
  }
  int myBPM = pulseSensor.getBeatsPerMinute();
  if(myBPM > 180){
    state = '3';
  }
  if(pulseSensor.sawStartOfBeat()) {
    if(myBPM > 180){
      state = '3';
    }
  }
  if(state == '3'){
    counter = 0;
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(2, HIGH);
    while(counter < 500){
      if(analogRead(2)>600){
        Serial.write('0');
        digitalWrite(LED_BUILTIN, LOW);
        digitalWrite(2, LOW);
        delay(200);
        counter = 0;
        Serial.write('6');
        break;
      }
      if(counter == 498){
        Serial.write('1');
        sendMSG();
        counter = 0;
        break;
      }
      counter++;
      delay(20);
    }
  }
  if(state == '2'){
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(2, LOW);
  }
}

void sendMSG() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(200);
  digitalWrite(LED_BUILTIN, LOW);
  delay(200);
  digitalWrite(LED_BUILTIN, HIGH);
  while(true){
    fona.getGSMLoc(&latitude, &longitude);
    Serial.println(latitude);
    char message[141];
    char LAT1[20];
    char LONG1[10];
    char LAT[10];
    char LONG[10];
    dtostrf(latitude, 3, 7, LAT1);
    dtostrf(longitude, 10, 7, LONG1);
    for(int i = 0; i < 9; i++){
      LAT[i] = LAT1[i];
      LONG[i] = LONG1[i];
    }
    LAT[9] = '\0';
    LONG[9] = '\0';
    sprintf(message, "This is device 1. HELP! I'M IN TROUBLE! https://www.google.com/maps/search/?api=1&query=%s,%s", LAT, LONG);
    char sendto[13] = "+18572148417";
    fona.sendSMS(sendto, message);
    delay(30000);
  }  
}
