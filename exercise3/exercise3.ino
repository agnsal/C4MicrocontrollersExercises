
/*
Copyright 2019-2021 Agnese Salutari.
Licensed under the Apache License, Version 2.0 (the "License"); 
you may not use this file except in compliance with the License. 
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on 
an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
See the License for the specific language governing permissions and limitations under the License
*/

// Signals:
#define RX 2
#define TX 3
#define BUZZER 4
#define HEATER 5

// States:
#define STDBY 0
#define ON 1
#define BUZZING 2

// Configuration Parameters:
#define HEAT_DEFAULT 5 // [0, 9]
#define HEAT_IDLE 0
#define HEAT_MAX 9
#define TIME_DEFAULT_SEC 1800 // seconds
#define TIME_INCR_SEC 1800 // seconds
#define TIME_MAX_SEC 36000 // seconds (has to be <= 65535)
#define BUZZING_T_SEC 10 // seconds
#define SEND_PERIOD_SEC 1 // seconds

//Global Variables:
uint8_t state;
uint8_t heat;
unsigned long endTimeMillis;
String ON_MSG = "ON";
String OFF_MSG = "OFF";
String HEAT_MSG = "L";
String TIME_MSG = "TIME";
String MSG_DELIMITER = "\n";

//Global Control Variables:
bool stateChanged;
bool buzzingTime;
bool theEnd;
String inputString; // A String to hold incoming data


void setup() {
  noInterrupts();
  cleanControlVariables();
  heat = (uint8_t)HEAT_DEFAULT;
  pinMode(BUZZER, OUTPUT);
  pinMode(HEATER, OUTPUT);
  // https://www.arduino.cc/en/Tutorial/SerialEvent
  Serial.begin(9600); // // Initialize serial.
  inputString.reserve(200); // Reserve 200 bytes for the inputString.
  interrupts();
  state = STDBY;
}

void loop() {
  updateState();
  cleanControlVariables();
  performState();
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stateChanged = true;
    }
  }
}

void cleanControlVariables(){
  inputString = "";
  stateChanged = false;
  buzzingTime = false;
  theEnd = false;
}

void updateState(){
  if(buzzingTime){
    state = BUZZING;
    return;
  }
  else if(theEnd || inputString == OFF_MSG + MSG_DELIMITER){
    state = STDBY;
    return;
  }
  switch(state){
    case STDBY:
      if(stateChanged && inputString == ON_MSG + MSG_DELIMITER){
        state = ON;
      }
      break;
    case ON:
      if(stateChanged){
        if(inputString == TIME_MSG + MSG_DELIMITER){
          incrTime();
        }
        else if(inputString[0] == 'L'){
          heat = (uint8_t)inputString[1] - 48; // Char to int
        }  
      }
      break;
    case BUZZING:
      if(stateChanged){
        if(inputString == TIME_MSG + MSG_DELIMITER ){
          incrTime();
          state = ON;
        }
        else if(inputString[0] == (char)"L"){
          state = ON;
        }
      }
    break;
  }
}

void performState(){
  switch(state){
    case STDBY:
      stdby();
      break;
    case ON:
      on();
      break;
    case BUZZING:
      buzzing();
      break;
  }
}

void heater_apply(){
  if(heat == HEAT_IDLE){
    digitalWrite(HEATER, LOW);
    return;
  }
  digitalWrite(HEATER, HIGH);
  delay(heat * 100);
  digitalWrite(HEATER, LOW);
  delay(1000 - heat * 100);
}

void incrTime(){
  if(endTimeMillis <= ((unsigned long)(TIME_MAX_SEC - TIME_INCR_SEC)) * 1000){
    endTimeMillis += (unsigned long)TIME_INCR_SEC * 1000;
  }
}

void stdby(){ // It doesn't contain cycles because they whould stop message receiving.
  digitalWrite(BUZZER, LOW);
  digitalWrite(HEATER, LOW);
  Serial.print(OFF_MSG);
  Serial.print(MSG_DELIMITER);
  delay(SEND_PERIOD_SEC * 1000);
  endTimeMillis = millis() + (unsigned long)TIME_DEFAULT_SEC * 1000;
}

void on(){
  digitalWrite(BUZZER, LOW);
  unsigned long remainingTimeMillis = endTimeMillis - millis();
  if(remainingTimeMillis >= BUZZING_T_SEC * 1000){
    heater_apply();
    Serial.print(HEAT_MSG);
    Serial.print(heat);
    Serial.print(MSG_DELIMITER);
    Serial.print(TIME_MSG);
    Serial.print(remainingTimeMillis/ 60000); // mm
    Serial.print(":");
    Serial.print((remainingTimeMillis / 1000) % 60); //ss
    Serial.print(MSG_DELIMITER);
    delay(SEND_PERIOD_SEC * 1000);
    remainingTimeMillis = endTimeMillis - millis();
  }
  else{
    buzzingTime = true;
  }
}

void buzzing(){
  digitalWrite(BUZZER, HIGH);
  if(millis() <= endTimeMillis){
    heater_apply();
  }
  else{
    digitalWrite(BUZZER, LOW);
    theEnd = true; 
  }
}
