
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
#define CLK 2 // UP for clockwise rotation, DOWN for counterclockwise rotation
#define DT 3
#define SW 4 // BACK button: 0 when pressed, 1 otherwise.
#define HOME_POS 4 // Button for HOME_POS too.
#define SPRINKLER 5
#define MOTOR 6

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
volatile bool stateChanged; // true when state is changed, false otherwise
volatile bool upPressed; // true when clockwise rotation, false otherwise
volatile bool downPressed; // true when counterclockwise rotation, false otherwise
bool backPressed; // true when pressed, false otherwise
bool motorAuth; // true when motor is authorized to move, false otherwise

uint8_t lastClk;
uint8_t lastDt;
uint8_t lastSw;


void setup() {
  noInterrupts();
  Serial.begin(115200);
  cleanControlVariables();
  lastSingleTimeMillis = 0;
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DT, INPUT_PULLUP);
  pinMode(SW, INPUT_PULLUP);
  pinMode(SPRINKLER, OUTPUT);
  pinMode(MOTOR, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(CLK), clkInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(DT), dtInterrupt, CHANGE);
  interrupts();
  lastClk = digitalRead(CLK);
  lastDt = digitalRead(DT);
  lastSw = digitalRead(SW);
  state = OFF;
  goMotor1Round(SLOW_MOTOR); // To go in Home Position by default.
}

void loop() {
  updateState();
  Serial.print("State: ");
  Serial.println(state);
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
  switch(state){
    case OFF:
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
    case SINGLE:
      state = OFF;
      break;
    case SPRINKLING:
      state = OFF;
      break;
    case PULSED:
      if(upPressed){
        state = SLOW;
      }
      else if(downPressed){
        state = OFF;
      }
      break;
    case SLOW:
      if(upPressed){
        state = FAST;
      }
      else if(downPressed){
        state = PULSED;
      }
      break;
    case FAST:
      if(downPressed){
        state = SLOW;
      }
      break;
  }
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

void clkInterrupt(){
  upAndDownUpdate();
  stateChanged = true;
}

void dtInterrupt(){
  upAndDownUpdate();
  stateChanged = true;
}

void upAndDownUpdate(){
  delay(20);
  uint8_t newClk = digitalRead(CLK);
  uint8_t newDt = digitalRead(DT);
  if(lastClk == LOW && lastDt == LOW){
    if(newClk == HIGH){
        upPressed = true;
      }
    else if(newDt == HIGH){
        downPressed = true;
    }
  }
  else if(lastClk == LOW && lastDt == HIGH){
    if(newDt == LOW){
      upPressed = true;
    }
    else if(newClk == HIGH){
      downPressed = true;
    }
  }
  else if(lastClk == HIGH && lastDt == LOW){
    if(newDt == HIGH){
      upPressed = true;
    }
    else if (newClk == LOW){
      downPressed = true;
    }
  }
  else{ // Case 1 1
    if(newClk == LOW){
      upPressed = true;
    }
    else if(newDt == LOW){
      downPressed = true;
    }
  }
  lastClk = newClk;
  lastDt = newDt;
  Serial.print("Up and Down Pressed Update - ");
  Serial.print("UP = ");
  Serial.print(upPressed);
  Serial.print(";  DOWN = ");
  Serial.println(downPressed);
}

void goMotor1Round(uint8_t vel){
  if(!motorAuth && digitalRead(HOME_POS)){
    return;
  }
  analogWrite(MOTOR, vel);
  while(digitalRead(HOME_POS) == LOW);
  while(!digitalRead(HOME_POS) == LOW);
  analogWrite(MOTOR, IDLE_MOTOR); 
}

void off(){
  while((!stateChanged) && (!backPressed)){
    if(digitalRead(SW) == LOW){
      backPressed = true;
      stateChanged = true;
    }
  }
}

void single(){
  lastSingleTimeMillis = millis();
  motorAuth = true;
  goMotor1Round(FAST_MOTOR); 
}

void sprinkling(){
  digitalWrite(SPRINKLER, HIGH);
  for(uint8_t i = 0; (i < SPRINKLING_ROUNDS) && (!stateChanged); i++){
    motorAuth = true;
    goMotor1Round(FAST_MOTOR);
  }
  digitalWrite(SPRINKLER, LOW);
}

void pulsed(){
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
  while(!stateChanged){
    motorAuth = true;
    goMotor1Round(SLOW_MOTOR);
  }
}

void fast(){
  while(!stateChanged){
    motorAuth = true;
    goMotor1Round(FAST_MOTOR);
  }
}
