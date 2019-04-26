int forcePin = 0;

int state = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(8000);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  while(true){
    state = Serial.read();
    if(state == '9'){
      delay(7000);
      break;
    }
  }
  
}

int counter1 = 0;
int timeFlag = 0;
int sosFlag = 0;

void loop() {
  // put your main code here, to run repeatedly:
  int reading = analogRead(forcePin);
  if(Serial.available()>0){
    state = Serial.read();
  }
  if(reading > 300) {
    Serial.write('2');
    counter1 = 0;
  }
  if(counter1 > 200){
    Serial.write('3');
    timeFlag = 1;
  }
  if(timeFlag == 1){
    while(true){
      if(Serial.available()>0){
        state = Serial.read();
      }
      if(state == '0' || state == '6'){
        sosFlag = 2;
        break;
      }
      if(state == '1'){
        while(true){
          delay(1000); //send msg mode
        }
      }
    }
  }
  if(sosFlag == 1){
    timeFlag = 1;
  }
  if(sosFlag == 2){
    Serial.write('4');
    sosFlag = 0;
    counter1 = 0;
    timeFlag = 0;
  }
  counter1++;
  delay(50);
}
