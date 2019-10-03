
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

#include <TM1637.h> // https://github.com/AKJ7/TM1637


// Signals:
#define SW 4 // LOW when active, HIGH otherwise
#define CLK 2 // LOW when active, HIGH otherwise
#define DT 3 // LOW when active, HIGH otherwise
#define GREEN_LED 5 // HIGH when activated, LOW otherwise
#define RED_LED 6 // HIGH when activated, LOW otherwise

// Display Pins:
#define TM1637_CLK 19
#define TM1637_DIO 18

// Configuration Parameters:
#define BRIGHTNESS_IDLE 0
#define BRIGHTNESS_MAX 255

//Global Variables:
TM1637 tm1637(TM1637_DIO, TM1637_CLK);


void setup() {
  noInterrupts();
  Serial.begin(9600); //Initiate Serial communication.
  tm1637.init(); // Initialize the screen.
  tm1637.setBrightness(5);
  pinMode(SW, INPUT);
  pinMode(CLK, INPUT);
  pinMode(SW, INPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(CLK), clkInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(DT), dtInterrupt, CHANGE);
  interrupts();
}

void loop() {
  uint8_t clk = digitalRead(CLK);
  uint8_t dt = digitalRead(DT);
  uint8_t sw = digitalRead(SW);
  Serial.print("CLK: "); 
  Serial.println(clk); 
  Serial.print("DT: "); 
  Serial.println(dt); 
  Serial.print("SW: "); 
  Serial.println(sw); 
  tm1637.dispNumber((uint8_t)0);
  delay(1000); 
  tm1637.switchColon();
  tm1637.dispNumber((uint8_t)1);
  delay(1000); 
  digitalWrite(GREEN_LED, HIGH);
  delay(500);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);
  delay(500);
  digitalWrite(RED_LED, LOW);
}

void clkInterrupt(){
  Serial.println("- CLK Interrupt! -"); 
}

void dtInterrupt(){
  Serial.println("- DT Interrupt! -"); 
}
