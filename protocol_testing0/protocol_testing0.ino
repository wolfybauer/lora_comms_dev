#include <LoRa.h>
#include <Wire.h>
#include <Bounce2.h>

#include "util.h"
/*
#define SEND_TIME 5000       // in millis
#define DISP_TIME 1000
unsigned long curr_time, prev_time = 0;
*/

#define BUTTON_PIN 6
Bounce button;

void setup() {
  Serial.begin(BAUD_RATE);
  lora_init();

  button.attach(BUTTON_PIN, INPUT_PULLUP);
  button.interval(5);

  //Serial.print
}

void loop() {
/*
  curr_time = millis();
  if(curr_time - prev_time >= DISP_TIME) {
    show_data();
  }
  if(curr_time - prev_time >= SEND_TIME) {
    sendRPing();
    prev_time = curr_time;
  }
  */
  button.update();
  if(button.fell()) {
    sendRPing();
  }

  on_rcv(LoRa.parsePacket());

}
