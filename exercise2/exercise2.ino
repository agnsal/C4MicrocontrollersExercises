
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

//#include <TM1637.h> // Not used for this exercise

// Signals:
#define BTN 4 // LOW when active, HIGH otherwise
#define CLK 2 // LOW when active, HIGH otherwise
#define DT 3 // LOW when active, HIGH otherwise
#define BUZZER 5 // HIGH when activated, LOW otherwise
#define HEATER 6 // HIGH when activated, LOW otherwise

// Display Pins:
// #define DISP_1 12 // Not used for this exercise
// #define DISP_2 13 // Not used for this exercise

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
#define TOTAL_T_DEFAULT_SEC 1800 // seconds
#define TOTAL_T_INCR_SEC 1800 // seconds
#define TOTAL_T_MAX_SEC 36000 // seconds (has to be <= 65535)
#define DISPLAY_ON_T_SEC 10 // seconds
#define SHOW_T_SEC 2 // seconds
#define BUZZING_T_SEC 10 // seconds

//Global Variables:
uint8_t state;
volatile uint8_t heat;
unsigned long remainingTimeMillis;
//TM1637 display(DISP_1, DISP_2); // Not used in this exercise

//Global Control Variables:
unsigned long btnPressedStartTMillis;
unsigned long btnPressedEndTMillis;
volatile bool stateChanged;
bool clkDtRotation;
uint8_t clk;
uint8_t dt;


void setup() {
  noInterrupts();
  cleanControlVariables();
  heat = HEAT_DEFAULT;
  remainingTimeMillis = TOTAL_T_DEFAULT_SEC * 1000;
  pinMode(BTN, INPUT);
  pinMode(CLK, INPUT);
  pinMode(DT, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(HEATER, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(CLK), rotationInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(DT), rotationInterrupt, CHANGE);
  interrupts();
  state = STDBY;
}

void loop() {
  updateState();
  cleanControlVariables();
  performState();
}

void cleanControlVariables(){
  btnPressedStartTMillis = 0;
  btnPressedEndTMillis = 0;
  stateChanged = false;
  clkDtRotation = false;
  clk = digitalRead(CLK);
  dt = digitalRead(dt);
}

void updateState(){
  if(btnPressedStartTMillis != 0 && btnPressedEndTMillis != 0){
    if(btnPressedEndTMillis - btnPressedStartTMillis >= BTN_PRESS_DELAY_SEC * 1000){
      state = STDBY;
      return;
    }
    else{
      if(state == DISPLAY_ON){
        incrTime();
      }
      else{
        state = DISPLAY_ON;
      }
      return;
    }
  }
  else{
    switch(state){
      case DISPLAY_ON:
        if(clkDtRotation){
          changeHeat();
        }
        else{
          state = DISPLAY_OFF;
        }
        break;
      case DISPLAY_OFF:
        if(clkDtRotation){
          state = DISPLAY_ON; 
        }
        break;
      case BUZZING:
        if(clkDtRotation){
          state = DISPLAY_ON; 
        }
        else{
          state = STDBY;
        }
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


void stdby(){
  digitalWrite(BUZZER, LOW);
  digitalWrite(HEATER, LOW);
  setBrightness((uint8_t)BRIGHTNESS_IDLE);
  while(!stateChanged);
}

void rotationInterrupt(){
  stateChanged = true;
  clkDtRotation = true;
}

void incrTime(){
  if(remainingTimeMillis <= (TOTAL_T_MAX_SEC - TOTAL_T_INCR_SEC) * 1000){
    remainingTimeMillis += TOTAL_T_INCR_SEC * 1000;
  }
}

void changeHeat(){
  uint8_t newClk = digitalRead(CLK);
  uint8_t newDt = digitalRead(DT);
  if(clk == LOW && dt == LOW){
    if(newClk == HIGH){
        heat += 10;
        heat %= 100;
      }
    else{
        heat -= 10;
        heat %= 100;
    }
  }
  else if(clk == LOW && dt == HIGH){
    if(newDt == LOW){
      heat += 10;
      heat %= 100;
    }
    else{
      heat -= 10;
      heat %= 100;
    }
  }
  else if(clk == HIGH && dt == LOW){
    if(newDt == HIGH){
      heat += 10;
      heat %= 100;
    }
    else{
      heat -= 10;
      heat %= 100;
    }
  }
  else{ // Case 1 1
    if(newClk == LOW){
      heat += 10;
      heat %= 100;
    }
    else{
      heat -= 10;
      heat %= 100;
    }
  }
}

void applyHeat(){
  digitalWrite(HEATER, HIGH);
  delay(heat * 10);
  digitalWrite(HEATER, LOW);
  delay(1000 - heat * 10);
}

bool checkBtn(){
  bool pressed = false;
  if(!digitalRead(BTN)){ // If BTN is LOW = active
    pressed = true;
    btnPressedStartTMillis = millis();
    while(!digitalRead(BTN));
    btnPressedEndTMillis = millis();
  }
  return pressed;
}

void displayOn(){
  unsigned long startTimeMillis = millis();
  unsigned long countdownMillis;
  setBrightness((uint8_t)BRIGHTNESS_MAX);
  show_duty(heat);
  while(!stateChanged){ // While stateChange is false
    remainingTimeMillis -= millis();
    show_time((uint16_t)countdownMillis * 1000);
    applyHeat();
    if(checkBtn || millis() - startTimeMillis > DISPLAY_ON_T_SEC * 1000){ // If btn has been pressed or spent time is more than DISPLAY_ON_T_SEC * 1000
      stateChanged = true;
    }
  }
}

void displayOff(){
  unsigned long startTimeMillis = millis();
  setBrightness((uint8_t)BRIGHTNESS_IDLE);
  while(!stateChanged){
    applyHeat();
    remainingTimeMillis -= millis();
    if(checkBtn || remainingTimeMillis <= BUZZING_T_SEC * 1000){ // If it's buzzing time
      stateChanged = true;
    }
  }
}

void buzzing(){
  unsigned long startTimeMillis = millis();
  digitalWrite(BUZZER, HIGH);
  while(!stateChanged){
    applyHeat();
    remainingTimeMillis -= millis();
    if(checkBtn || remainingTimeMillis <= 0){ // If it's end time
      stateChanged = true;
      digitalWrite(BUZZER, LOW);
    }
  }
}
