#ifndef UTIL_H
#define UTIL_H

// BSFrance LoRa32u4 v1.3 / Adafruit feather 32u4
#define SCK     15   // GPIO5  -- SX1278's SCK
#define MISO    14   // GPIO19 -- SX1278's MISO
#define MOSI    16   // GPIO27 -- SX1278's MOSI
#define SS      8    // GPIO18 -- SX1278's CS
#define RST     4    // GPIO14 -- SX1278's RESET
#define DI0     7    // GPIO26 -- SX1278's IRQ(Interrupt Request)

// MISC defines
#define BAND      915E6     // radio frequency band USA= 915E6
#define BAUD_RATE 115200      // Serial connection baud rate


// PACKAGE TYPES
#define TYPE_NONE     255
#define TYPE_BROAD    0     // broadcast
#define TYPE_RPING    1     // repeater ping
#define TYPE_RPONG    2     // repeater pong
#define TYPE_PING     3     // node ping
#define TYPE_PONG     4     // node pong
#define TYPE_MSG      5     // text message
#define TYPE_RRMSG    6     // repeater-repeated text message
#define TYPE_NRMSG    7     // node-repeated text message

//
#define PACKET_SIZE 250


// ROUTING DATA
const uint8_t NET_ID = 3;   // 1 byte net address/key (255 reserved for all)
const uint8_t MY_ID = 240;  // 1 byte device address: nodes 0-239, repeaters 240-254 (255 reserved for none/all/broad)
uint8_t MSG_COUNT = 0;
uint8_t LAST_RCV_PING[PACKET_SIZE], LAST_RCV_PONG[PACKET_SIZE],
            LAST_SNT_PING[PACKET_SIZE], LAST_SNT_PONG[PACKET_SIZE];
unsigned long RCV_TIMES[255];
unsigned long t_buf_print;
            

// --- LORA INIT, run in setup --- //
void lora_init() {
  Serial.println("Attempting to start LoRa...");
  LoRa.setPins(SS,RST,DI0); // set correct pins for logic level converters etc
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);              // loop forever if LoRa init fails
  }
  Serial.println("LoRa started successfully!");
}

// --- RPING SEND FUNCTION --- //
void sendRPing() {
  // buffer to be filled+sent
  uint8_t pack_out[PACKET_SIZE];
  // routing data
  pack_out[0] = NET_ID;
  pack_out[1] = TYPE_RPING;
  pack_out[2] = MY_ID;
  pack_out[3] = 255; // (all)
  // time sent, split from 4byte unsigned long into bytes
  unsigned long n = millis();
  unsigned long t_buf = n;
  pack_out[4] = n >> 24;
  pack_out[5] = n >> 16;
  pack_out[6] = n >> 8;
  pack_out[7] = n;
  // signal strength info
  pack_out[8] = LoRa.rssi(); // PINGs send average rssi at time of send
  pack_out[9] = MSG_COUNT;

  LoRa.beginPacket();
  LoRa.write(pack_out, PACKET_SIZE);
  LoRa.endPacket();

  memcpy(LAST_SNT_PING, pack_out, PACKET_SIZE);
  MSG_COUNT++;
  
  Serial.print("PING!-->");
  for(int i=0; i<10; i++) {
    Serial.print(" ");
    if(LAST_SNT_PING[i] < 100) Serial.print("0");
    if(LAST_SNT_PING[i] < 10) Serial.print("0");
    Serial.print(LAST_SNT_PING[i]);
  }
  Serial.print(" SNT:");
  Serial.print(t_buf);
  Serial.println();

}

// --- RPONG SEND FUNCTION --- //
unsigned long sendRPong() {
  // buffer to be filled+sent
  uint8_t pack_out[PACKET_SIZE];
  // routing data
  pack_out[0] = NET_ID;
  pack_out[1] = TYPE_RPONG;
  pack_out[2] = MY_ID;
  pack_out[3] = LAST_RCV_PING[2]; // sent to device id of last rcv ping
  // time sent
  unsigned long n = millis();
  unsigned long t_buf = n;
  pack_out[4] = n >> 24;
  pack_out[5] = n >> 16;
  pack_out[6] = n >> 8;
  pack_out[7] = n;
  // signal strength info
  pack_out[8] = LAST_RCV_PING[8]; // PONGs send rssi of last rcv ping
  pack_out[9] = MSG_COUNT;

  LoRa.beginPacket();
  LoRa.write(pack_out, PACKET_SIZE);
  LoRa.endPacket();

  memcpy(LAST_SNT_PONG, pack_out, PACKET_SIZE);
  MSG_COUNT++;

  
  return t_buf;
}

// --- ON RECEIVE, to be run in loop --- //
void on_rcv(int sz) {
  if(sz == 0) return;             // exit if no packet rcvd
  if(sz != PACKET_SIZE) return;   // exit if wrong packet size

  uint8_t d_buf[PACKET_SIZE];     // buffer to hold incoming data
  
  while(LoRa.available()) {       // get the data
    for(int i=0; i<PACKET_SIZE; i++) {
      d_buf[i] = LoRa.read();
    }
  }

  unsigned long t_buf = millis(); // buffer to hold time rcvd

  if(d_buf[0] != NET_ID && d_buf[0] != 255) return;   // filter by net id
  if(d_buf[3] != MY_ID && d_buf[3] != 255) return;    // filter by my id

  switch(d_buf[1]) {              // do stuff based on msg type
    case TYPE_RPING:
      memmove(LAST_RCV_PING, d_buf, PACKET_SIZE);
      RCV_TIMES[TYPE_RPING] = t_buf;
      unsigned long tp;
      tp = sendRPong();
      
      // print the incoming ping
      Serial.print("<--PING!");
      for(int i=0; i<10; i++) {
        Serial.print(" ");
        if(LAST_RCV_PING[i] < 100) Serial.print("0");
        if(LAST_RCV_PING[i] < 10) Serial.print("0");
        Serial.print(LAST_RCV_PING[i]);
      }
      Serial.print(" RCV:");
      Serial.print(t_buf);
      Serial.println();

      // print the corresponding pong
      Serial.print("PONG!-->");
      for(int i=0; i<10; i++) {
        Serial.print(" ");
        if(LAST_SNT_PONG[i] < 100) Serial.print("0");
        if(LAST_SNT_PONG[i] < 10) Serial.print("0");
        Serial.print(LAST_SNT_PONG[i]);
      }
      Serial.print(" SNT:");
      Serial.print(tp);
      Serial.println();
      
      break;
    case TYPE_RPONG:
      memmove(LAST_RCV_PONG, d_buf, PACKET_SIZE);
      RCV_TIMES[TYPE_RPONG] = t_buf;
      Serial.print("<--PONG!");
      for(int i=0; i<10; i++) {
        Serial.print(" ");
        if(LAST_RCV_PONG[i] < 100) Serial.print("0");
        if(LAST_RCV_PONG[i] < 10) Serial.print("0");
        Serial.print(LAST_RCV_PONG[i]);
      }
      Serial.print(" RCV:");
      Serial.print(t_buf);
      Serial.println();
      break;
    default:
      break;
  }
}

// --- PRINT REPLY PONG --- //

void print_reply_pong(unsigned long t_buf) {
  
}

#endif
