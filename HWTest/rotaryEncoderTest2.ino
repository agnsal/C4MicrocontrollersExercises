
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
#define SW 4 
#define CLK 2 
#define DT 3 


void setup() {
  noInterrupts();
  Serial.begin(115200); //Initiate Serial communication.
  pinMode(SW, INPUT);
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DT, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(CLK), clkInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(DT), dtInterrupt, CHANGE);
  interrupts();
}

void loop() {
}

void clkInterrupt(){
  uint8_t level = digitalRead(CLK);
  Serial.print("- CLK Interrupt! -"); 
  (level == 1) ? Serial.println("alto") : Serial.println("basso");
  Serial.println(readValues());
  delay(40);
}

void dtInterrupt(){
  uint8_t level = digitalRead(DT);
  Serial.print("- DT Interrupt! -");
  (level == 1) ? Serial.println("alto") : Serial.println("basso");
  Serial.println(readValues());
  delay(40);
}

uint8_t readValues(){
  return (digitalRead(CLK)<<1)|(digitalRead(DT));
}
