
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

//#include <TM1637.h> // TODO

// Signals:
#define BTN 2
#define CLK 3
#define DT 4
#define BUZZER 5
#define HEATER 6

// Display Pins:
// #define DISP_1 12 // TODO
// #define DISP_2 13 // TODO

// States:
#define STDBY 0
#define DISPLAY_ON 1
#define DISPLAY_OFF 2
#define BUZZING 3

// Configuration Parameters:
#define BTN_PRESS_DELAY_SEC 2 // seconds
#define BRIGHTNESS_IDLE 0
#define BRIGHTNESS_MAX 255
#define HEAT_DEFAULT 50 // %
#define COUNTDOWN_DEFAULT_SEC 1800 // seconds
#define COUNTDOWN_INCR_SEC 1800 // seconds
#define COUNTDOWN_MAX_SEC 36000 // seconds (has to be <= 65535)
#define DISPLAY_ON_T_SEC 10 // seconds
#define SHOW_T_SEC 2 // seconds
#define BUZZING_T_SEC 10 // seconds
#define BUZZING_T_TO_TIMEOVER 10 // seconds

//Global Variables:
uint8_t state;
uint8_t heat;
unsigned long countdownMillis;
//TM1637 display(DISP_1, DISP_2); // TODO

//Global Control Variables:
bool btnPressed;
bool btnPressedForMoreTime;
bool stateChanged;
bool clkDtRotation;
bool buzzingTime;
bool timeOver;


void setup() {
  noInterrupts();
  cleanControlVariables();
  heat = HEAT_DEFAULT;
  countdownMillis = COUNTDOWN_DEFAULT_SEC * 1000;
  pinMode(BTN, INPUT_PULLUP);
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(HEATER, OUTPUT);
  interrupts();
  stdby();
}

void loop() {
  noInterrupts();
  updateState();
  cleanControlVariables();
  interrupts();
  performState();
}

void cleanControlVariables(){
  btnPressed = false;
  btnPressedForMoreTime = false;
  stateChanged = false;
  clkDtRotation = false;
  buzzingTime= false;
  timeOver = false;
}

void updateState(){
  if(btnPressedForMoreTime){
    state = STDBY;
    return;
  }
  else if(buzzingTime){
    state = BUZZING;
    return;
  }
  else{
    switch(state){
      case STDBY:
        if(btnPressed){
          state = DISPLAY_ON;
        }
        break;
      case DISPLAY_ON:
        if(btnPressed){
          incrTime();
        }
        else state = DISPLAY_OFF;
        break;
      case DISPLAY_OFF:
        if(btnPressed || clkDtRotation){
          state = DISPLAY_ON; 
        }
        break;
      case BUZZING:
        state = STDBY;
        break;
    }    
  }
}

void performState(){
  switch(state){
    case STDBY:
      stdby();
      break;
    case DISPLAY_ON:
      displayOn();
      break;
    case DISPLAY_OFF:
      displayOff();
      break;
    case BUZZING:
      buzzing();
      break;
  }
}

void setBrightness(uint8_t bright){
  // TODO
}

void show_duty(uint8_t level){
  // TODO
}

void show_time(uint16_t sec){
  // TODO
}

void btnPressedInterrupt(){
  stateChanged = true;
  btnPressed = true;
  unsigned long startTimeMillis = millis();
  while(digitalRead(BTN) == HIGH);
  if(millis() - startTimeMillis > BTN_PRESS_DELAY_SEC * 1000){
    btnPressedForMoreTime = true;
  }
}

void stdby(){
  state = STDBY;
  digitalWrite(BUZZER, LOW);
  digitalWrite(HEATER, LOW);
  setBrightness((uint8_t)BRIGHTNESS_IDLE);
  while(!stateChanged);
}

void incrTime(){
  if(countdownMillis <= (COUNTDOWN_MAX_SEC - COUNTDOWN_INCR_SEC) * 1000){
    countdownMillis += COUNTDOWN_INCR_SEC * 1000;
  }
}

void changeHeat(uint8_t oldCLK, uint8_t oldDT, uint8_t newCLK, uint8_t newDT){
  stateChanged = true;
  clkDtRotation = true;
  if(oldCLK == LOW && oldDT == LOW){
    if(newCLK == HIGH){
        heat += 10;
        heat %= 100;
      }
    else{
        heat -= 10;
        heat %= 100;
    }
  }
  else if(oldCLK == LOW && oldDT == HIGH){
    if(newDT == LOW){
      heat += 10;
      heat %= 100;
    }
    else{
      heat -= 10;
      heat %= 100;
    }
  }
  else if(oldCLK == HIGH && oldDT == LOW){
    if(newDT == HIGH){
      heat += 10;
      heat %= 100;
    }
    else{
      heat -= 10;
      heat %= 100;
    }
  }
  else{ // Case 1 1
    if(newCLK == LOW){
      heat += 10;
      heat %= 100;
    }
    else{
      heat -= 10;
      heat %= 100;
    }
  }
}

void displayOn(){
  state = DISPLAY_ON;
  uint8_t oldCLK = digitalRead(CLK);
  uint8_t oldDT = digitalRead(DT);
  uint8_t newCLK;
  uint8_t newDT;
  unsigned long startTimeMillis = millis();
  digitalWrite(HEATER, HIGH);
  setBrightness((uint8_t)BRIGHTNESS_MAX);
  show_duty(heat);
  while((!stateChanged) && (millis() - startTimeMillis <= SHOW_T_SEC * 1000));
  while((!stateChanged) && ((millis() - startTimeMillis) < (DISPLAY_ON_T_SEC * 1000))){
    if(millis() - startTimeMillis <= (countdownMillis - BUZZING_T_TO_TIMEOVER * 1000)){
      buzzingTime = true;
      return;
    }
    show_time((uint16_t)(countdownMillis / 1000));
    newCLK = digitalRead(CLK);
    newDT = digitalRead(DT);
    if(newCLK != oldCLK || newDT != oldDT){
      changeHeat(oldCLK, oldDT, newCLK, newDT);
      return;
    }
    countdownMillis -= millis() - startTimeMillis;
  }
}

void displayOff(){
  state = DISPLAY_OFF;
  uint8_t oldCLK = digitalRead(CLK);
  uint8_t oldDT = digitalRead(DT);
  uint8_t newCLK;
  uint8_t newDT;
  unsigned long startTimeMillis = millis();
  setBrightness((uint8_t)BRIGHTNESS_IDLE);
  while((!stateChanged) && (millis() - startTimeMillis <= countdownMillis - BUZZING_T_TO_TIMEOVER * 1000)){
    newCLK = digitalRead(CLK);
    newDT = digitalRead(DT);
    if(newCLK != oldCLK || newDT != oldDT){
      changeHeat(oldCLK, oldDT, newCLK, newDT);
      return;
    }
    countdownMillis -= millis() - startTimeMillis;
  }
  buzzingTime = true;
}

void buzzing(){
  state = BUZZING;
  buzzingTime = false;
  unsigned long startTimeMillis = millis();
  digitalWrite(BUZZER, HIGH);
  while((!stateChanged) && (millis() - startTimeMillis <= BUZZING_T_SEC * 1000));
  digitalWrite(BUZZER, LOW);
}
