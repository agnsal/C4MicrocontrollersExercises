
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

#define CLK 2 
#define DT 3 
#define SW 4

void setup() {
  Serial.begin(115200); //Initiate Serial communication.
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DT, INPUT_PULLUP);
  pinMode(SW, INPUT_PULLUP);
}

void loop() {
  uint8_t val = readValues();
  Serial.print(val);
  Serial.print(", ");
  Serial.println(digitalRead(SW));
  delay(50);
}

uint8_t readValues(){
  return (digitalRead(CLK)<<1)|(digitalRead(DT));
}
