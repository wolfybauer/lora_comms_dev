#include <sd_defines.h>
#include <sd_diskio.h>
#include <SD.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "util_init.h"
//#include "util_gfx.h"

// MISC defines
#define BAND      915E6     // radio frequency band USA= 915E6
#define BAUD_RATE 115200      // Serial connection baud rate, for debugging etc


void setup() {
  
  // Serial connection init
  Serial.begin(BAUD_RATE);
  while (!Serial);          // loop forever if serial connection fails
  Serial.println();
  Serial.println("Sender");

  
  init_spi();
  init_lora(BAND);
  init_sd();
  init_oled();
}

void loop() {
  getKeyboardInput();
  checkForSend();

  display.clearDisplay();
  drawUI();
  drawOutbox(keysIn);
  display.display();

}

void test_sd_write(const char * filename, String &s) {
  
  //digitalWrite(SD_SS, LOW);
  //digitalWrite(LORA_SS, HIGH);
  File my_file = SD.open(filename, FILE_WRITE);

  if(!my_file) {
    Serial.println("ERROR writing to test.txt");
    return;
  }

  if(my_file.print(s)) {
    Serial.println("File written.");
  } else {
    Serial.println("Write failed.");
  }
  my_file.close();
  //digitalWrite(SD_SS, HIGH);
  //digitalWrite(LORA_SS, LOW);
}

void test_sd_read() {
  File my_file = SD.open("SD_TEST.txt");
  if(my_file) {
    Serial.println("SD_TEST.txt sez:");

    while(my_file.available()) {
      Serial.write(my_file.read());
    }

    my_file.close();
  } else {
    Serial.println("ERROR reading from SD_TEST.txt");
  }
}

void getKeyboardInput() {
  
  uint8_t key;
  Wire.requestFrom(CARDKB_ADDRESS, 1);
  if (!Wire.available()) {
    return;
  } else {
    key = Wire.read();
  }

  
  if(key != 0) { // redundant??
    // ENTER KEY
    if(key == 0x0D) {
      Serial.println(" ENTER");
      sendText = true;
      return;
    }
    // BACKSPACE KEY
    else if(key == 0x08) {
      keysIn.remove(keysIn.length() - 1);
      Serial.println("[BACKSPACE]");
      Serial.print(keysIn);
    }
      // ALL OTHER KEYS
    else /*if(key < 0x0D || key > 0x0D)*/ {
      keysIn += (char)key;
      Serial.print((char)key);
    }
  }
}

void checkForSend() {
  if(sendText == true) {
    const char * nm = "/test.txt";
    //const char * ki = keysIn;
    test_sd_write(nm, keysIn);
    //Serial.print("\nSAVED TO SD @ TEST_cardkb_sd.txt");
    keysIn = "";
    sendText = false;
  }
}
