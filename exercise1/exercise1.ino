
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
#define UP 2
#define DOWN 3
#define BACK 4
#define HOME_POS 5
#define SPRINKLER 6
#define MOTOR 12

// States:
#define OFF 0
#define SINGLE 1
#define SPRINKLING 2
#define PULSED 3
#define SLOW 4
#define FAST 5

// Configurations:
#define IDLE_MOTOR 0 // 0V
#define SLOW_MOTOR 153 // 3V
#define FAST_MOTOR 255 // 5V
#define SPRINKLING_ROUNDS 6
#define STANDARD_PULSE_T_SEC 6 // sec

// Global Variables:
uint8_t state;
unsigned long lastSingleTimeMillis;

// Global Control Variables:
bool stateChanged; // true when state is changed, false otherwise
bool upPressed; // true when pressed, false otherwise
bool downPressed; // true when pressed, false otherwise
bool backPressed; // true when pressed, false otherwise
bool motorAuth; // true when motor is authorized to move, false otherwise


void setup() {
  noInterrupts();
  cleanControlVariables();
  lastSingleTimeMillis = 0;
  pinMode(UP, INPUT_PULLUP);
  pinMode(DOWN, INPUT_PULLUP);
  pinMode(BACK, INPUT_PULLUP);
  pinMode(HOME_POS, INPUT);
  pinMode(SPRINKLER, OUTPUT);
  pinMode(MOTOR, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(UP), upInterrupt, RISING);
  attachInterrupt(digitalPinToInterrupt(DOWN), downInterrupt, RISING);
  interrupts();
  off();
}

void loop() {
  updateState();
  cleanControlVariables();
  performState();
}

void cleanControlVariables(){
  stateChanged = false;
  motorAuth = false;
  upPressed = false;
  downPressed = false;
  backPressed = false;
}

void updateState(){
  noInterrupts();
  switch(state){
    case(OFF):
      if(upPressed){
        state = PULSED;
      }
      else if(downPressed){
        state = SINGLE;
      }
      else if(backPressed){
        state = SPRINKLING;
      }
      break;
    case(SINGLE):
      state = OFF;
      break;
    case(SPRINKLING):
      state = OFF;
      break;
    case(PULSED):
      if(upPressed){
        state = SLOW;
      }
      else if(downPressed){
        state = OFF;
      }
      break;
    case(SLOW):
      if(upPressed){
        state = FAST;
      }
      else if(downPressed){
        state = PULSED;
      }
      break;
    case(FAST):
      if(downPressed){
        state = SLOW;
      }
      break;
  }
  interrupts();
}

void performState(){
  switch(state){
    case OFF:
      off();
      break;
    case SINGLE:
      single();
      break;
    case SPRINKLING:
      sprinkling();
      break;
    case PULSED:
      pulsed();
      break;
    case SLOW:
      slow();
      break;
    case FAST:
      fast();
      break;
  }
}

void upInterrupt(){
  upPressed = true;
  stateChanged = true;
}

void downInterrupt(){
  downPressed = true;
  stateChanged = true;
}

void goMotor1Round(uint8_t vel){
  while(motorAuth || (!digitalRead(HOME_POS))){
    motorAuth = false;
    analogWrite(MOTOR, vel);
  }
  analogWrite(MOTOR, IDLE_MOTOR); 
}

void off(){
  state = OFF;
  motorAuth = false;
  goMotor1Round(SLOW_MOTOR); // To go in Home Position by default.
  while((!stateChanged) && (!backPressed)){
    if(digitalRead(BACK)){
      backPressed = true;
      stateChanged = true;
    }
  }
}

void single(){
  state = SINGLE;
  lastSingleTimeMillis = millis();
  motorAuth = true;
  goMotor1Round(FAST_MOTOR); 
}

void sprinkling(){
  state = SPRINKLING;
  digitalWrite(SPRINKLER, HIGH);
  for(uint8_t i = 0; (i < SPRINKLING_ROUNDS) && (!stateChanged); i++){
    motorAuth = true;
    goMotor1Round(FAST_MOTOR);
  }
  digitalWrite(SPRINKLER, LOW);
}

void pulsed(){
  state = PULSED;
  unsigned long startTimeMillis = millis();
  unsigned long deltaTMillis = STANDARD_PULSE_T_SEC * 1000;
  if(lastSingleTimeMillis < deltaTMillis){
    deltaTMillis = lastSingleTimeMillis;
  }
  while(!stateChanged){
    motorAuth = true;
    goMotor1Round(SLOW_MOTOR);
    while((!stateChanged) && (millis() - startTimeMillis <= deltaTMillis));
  }
}

void slow(){
  state = SLOW;
  while(!stateChanged){
    motorAuth = true;
    goMotor1Round(SLOW_MOTOR);
  }
}

void fast(){
  state = FAST;
  while(!stateChanged){
    motorAuth = true;
    goMotor1Round(FAST_MOTOR);
  }
}
