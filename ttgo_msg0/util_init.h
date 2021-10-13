#ifndef UTIL_INIT_H
#define UTIL_INIT_H

#include "util_gfx.h"

// TTGO LoRa32  [LORA] pin defines (VSPI)
#define LORA_SCK    5    // GPIO5  -- SX1278's SCK
#define LORA_MISO   19   // GPIO19 -- SX1278's MISO
#define LORA_MOSI   27   // GPIO27 -- SX1278's MOSI
#define LORA_SS     18   // GPIO18 -- SX1278's CS
#define RST         23   // GPIO14 -- SX1278's RESET
#define DI0         26   // GPIO26 -- SX1278's IRQ(Interrupt Request)

// TTGO LoRa32  [SD CARD] pin defines (HSPI)
#define SD_SCK    14
#define SD_MISO   2
#define SD_MOSI   15
#define SD_SS     13

// uninitilialized pointers to SPI objects
SPIClass * lora_spi = NULL;
SPIClass * sd_spi   = NULL;

// CARDKB STUFF
#define CARDKB_ADDRESS 0x5F
String keysIn = "";
bool sendText = false;




// --- FULL INIT FUNCTIONS --- //

void init_spi() {
  // SPI init
  lora_spi = new SPIClass(VSPI); // virtual spi
  sd_spi   = new SPIClass(HSPI); // hardware spi
  lora_spi->begin(LORA_SCK,LORA_MISO,LORA_MOSI,LORA_SS);
  sd_spi->begin(SD_SCK,SD_MISO,SD_MOSI,SD_SS);
  pinMode(LORA_SS, OUTPUT);
  pinMode(SD_SS, OUTPUT);
}

void init_lora(long freq) {
  // LoRa init
  LoRa.setPins(SS,RST,DI0); // set correct pins for logic level converters etc
  if (!LoRa.begin(freq)) {
    Serial.println("Starting LoRa failed!");
    while (1);              // loop forever if LoRa init fails
  }
}

void init_sd() {
  // SD init
  if(!SD.begin(SD_SS, *sd_spi)) {
    Serial.println("\nSD read/write init FAILED!");
    //while(1);
  }
  Serial.print("SD card size:");
  Serial.println(SD.cardSize());
}

void init_oled() {
  // OLED pre-init routine
  pinMode(16,OUTPUT);
  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(100); 
  digitalWrite(16, HIGH);   // while OLED is running, must set GPIO16 to high
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();

  display.drawBitmap(47, 15, WOLF_BMP, 32, 32, 1);
  display.setTextSize(1);
  display.setTextColor(WHITE, BLACK);
  display.setTextWrap(false);

  display.display();

  delay(2000);
}


#endif
