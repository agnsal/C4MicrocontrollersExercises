
/*
Copyright 2019 Agnese Salutari.
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
#define COUNTDOWN_DEFAULT_SEC 1800 // seconds
#define COUNTDOWN_INCR_SEC 1800 // seconds
#define COUNTDOWN_MAX_SEC 36000 // seconds (has to be <= 65535)
#define BUZZING_T_SEC 10 // seconds
#define SEND_PERIOD_SEC 1

//Global Variables:
uint8_t state;
uint8_t heat;
unsigned long countdownMillis;
String ON_MSG = "ON";
String OFF_MSG = "OFF";
String HEAT_MSG = "L";
String TIME_MSG = "TIME";
String MSG_DELIMITER = "\n";

//Global Control Variables:
bool msgReceived;
String inputString;         // a String to hold incoming data


void setup() {
  cleanControlVariables();
  heat = HEAT_DEFAULT;
  countdownMillis = COUNTDOWN_DEFAULT_SEC * 1000;
  pinMode(BUZZER, OUTPUT);
  pinMode(HEATER, OUTPUT);
  // https://www.arduino.cc/en/Tutorial/SerialEvent
  // initialize serial:
  Serial.begin(9600);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
  stdby();
}

void loop() {
  updateState();
  cleanControlVariables();
  performState();
}

void cleanControlVariables(){
  inputString = "";
  msgReceived = false;
}

void updateState(){
  if(inputString == OFF_MSG + MSG_DELIMITER){
    state = STDBY;
    return;
  }
  switch(state){
    case STDBY:
      if(inputString == ON_MSG + MSG_DELIMITER){
        state = ON;
      }
      break;
    case ON:
      if(!msgReceived){
        state = BUZZING;
      }
      else if(inputString == TIME_MSG + MSG_DELIMITER){
        incrTime();
      }
      else if(inputString[0] == (char)"L"){
        heat = (uint8_t)inputString[1];
      }
      break;
    case BUZZING:
      state = STDBY;
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

void heater_apply(uint8_t heat){
  if(heat == HEAT_IDLE){
    digitalWrite(HEATER, LOW);
    return;
  }
  if(heat > 9){
    heat = 9;
  }
  unsigned long activeTMillis = heat * 100;
  digitalWrite(HEATER, HIGH);
  delay(activeTMillis);
  digitalWrite(HEATER, LOW);
  delay(1000 - activeTMillis);
}

void stdby(){
  state = STDBY;
  heater_apply(HEAT_IDLE);
  bool periodPast = false;
  unsigned long periodStartMillis = millis();
  while(Serial.available() && !msgReceived) {
    if(Serial.availableForWrite() && (millis() - periodStartMillis >= SEND_PERIOD_SEC * 1000)){
      Serial.print(OFF_MSG);
      Serial.print(MSG_DELIMITER);
      periodStartMillis = millis();
    }
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      msgReceived = true;
    }
  }
}

void on(){
  state = ON;
  unsigned long startTimeMillis = millis();
  unsigned long periodStartMillis = startTimeMillis;
  while(Serial.available() && !msgReceived && (millis() - startTimeMillis <= countdownMillis - BUZZING_T_SEC * 1000)) {
    heater_apply(heat);
    countdownMillis -= millis() - startTimeMillis;
    if(Serial.availableForWrite() && (millis() - periodStartMillis >= SEND_PERIOD_SEC * 1000)){
      Serial.print(HEAT_MSG);
      Serial.print(heat);
      Serial.print(MSG_DELIMITER);
      Serial.print(TIME_MSG);
      Serial.print((int)(countdownMillis/(1000 * 60))); // mm
      Serial.print(":");
      Serial.print(((int)(countdownMillis/1000) % 60)); //ss
      Serial.print(MSG_DELIMITER);
      periodStartMillis = millis();
    }
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      msgReceived = true;
    }
    countdownMillis -= millis() - startTimeMillis;
  }
}

void buzzing(){
  state = BUZZING;
  unsigned long startTimeMillis = millis();
  digitalWrite(BUZZER, HIGH);
  while(Serial.available() && !msgReceived && (millis() - startTimeMillis <= countdownMillis)) {
    heater_apply(heat);
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      msgReceived = true;
    }
    countdownMillis -= millis() - startTimeMillis;
  }
}

void incrTime(){
  if(countdownMillis <= (COUNTDOWN_MAX_SEC - COUNTDOWN_INCR_SEC) * 1000){
    countdownMillis += COUNTDOWN_INCR_SEC * 1000;
  }
}
