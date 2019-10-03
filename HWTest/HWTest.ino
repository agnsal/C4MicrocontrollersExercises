
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

#include <TM1637Display.h> // https://github.com/avishorp/TM1637


// Signals:
#define SW 4 
#define CLK 2 
#define DT 3 
#define GREEN_LED 5 // HIGH when activated, LOW otherwise
#define RED_LED 6 // HIGH when activated, LOW otherwise

// Display Pins:
#define TM1637_CLK 19
#define TM1637_DIO 18

// Configuration Parameters:
#define BRIGHTNESS_IDLE 0
#define BRIGHTNESS_MAX 255
#define DELAY 2000 // ms

//Global Variables:
TM1637Display tm1637(TM1637_CLK, TM1637_DIO);


void setup() {
  noInterrupts();
  Serial.begin(9600); //Initiate Serial communication.
  pinMode(SW, INPUT);
  pinMode(CLK, INPUT);
  pinMode(SW, INPUT);
  pinMode(RED_LED, OUTPUT); // It corresponds to: DDRD |= _BV(DDD6); // DDRx |= _BV(DDxn), in this case: Port D 6th pin as output.
  pinMode(GREEN_LED, OUTPUT); // It corresponds to: DDRD |= _BV(DDD5);
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
  digitalWrite(GREEN_LED, HIGH); // It corresponds to: PORTD |= _BV(PORTD5);
  delay(DELAY); // It corresponds to: _delay_ms(DELAY);
  digitalWrite(GREEN_LED, LOW); // It corresponds to: PORTD &= ~_BV(PORTD5);
  digitalWrite(RED_LED, HIGH); // It corresponds to: PORTD |= _BV(PORTD6);
  delay(DELAY);
  digitalWrite(RED_LED, LOW); // It corresponds to: PORTD &= ~_BV(PORTD6);
  tm1637.setBrightness(BRIGHTNESS_MAX, true);
  tm1637.showNumberDec(millis(), true);
  delay(DELAY);
  tm1637.setBrightness(BRIGHTNESS_IDLE, true);
}

void clkInterrupt(){
  Serial.println("- CLK Interrupt! -"); 
}

void dtInterrupt(){
  Serial.println("- DT Interrupt! -"); 
}
