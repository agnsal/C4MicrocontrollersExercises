
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
#define CLK 2 
#define DT 3
#define SW 4  

// Delay:
#define DELAY_MILLIS 10


void setup() {
  noInterrupts();
  Serial.begin(115200); //Initiate Serial communication.
  pinMode(SW, INPUT_PULLUP);
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(CLK), clkInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(DT), dtInterrupt, CHANGE);
  interrupts();
}

void loop() {

}

void clkInterrupt(){
  delay(DELAY_MILLIS);
  uint8_t level = digitalRead(CLK);
  Serial.print("- CLK Interrupt! - "); 
  (level == 1) ? Serial.println("up") : Serial.println("down");
  printSignals();
}

void dtInterrupt(){
  delay(DELAY_MILLIS);
  uint8_t level = digitalRead(DT);
  Serial.print("- DT Interrupt! - ");
  (level == 1) ? Serial.println("up") : Serial.println("down");
  printSignals();
}

void printSignals(){
  Serial.print("CLK = ");
  Serial.println(digitalRead(CLK));
  Serial.print("DT = ");
  Serial.println(digitalRead(DT));
  Serial.print("SW = ");
  Serial.println(digitalRead(SW));
}

// The resulting sequence is:
// Clockwise Rotation: 00 - 10 - 11 - 01 - 00 ...
// Counterclockwise rotation is the same, but order is inverted.
