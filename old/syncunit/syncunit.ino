#include <SPI.h>
#include <RH_RF95.h>
#include <math.h>

// for feather m0
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

#if defined(ESP8266)
/* for ESP w/featherwing */
#define RFM95_CS 2    // "E"
#define RFM95_RST 16  // "D"
#define RFM95_INT 15  // "B"

#elif defined(ESP32)
/* ESP32 feather w/wing */
#define RFM95_RST 27  // "A"
#define RFM95_CS 33   // "B"
#define RFM95_INT 12  //  next to A

#elif defined(NRF52)
/* nRF52832 feather w/wing */
#define RFM95_RST 7   // "A"
#define RFM95_CS 11   // "B"
#define RFM95_INT 31  // "C"

#elif defined(TEENSYDUINO)
/* Teensy 3.x w/wing */
#define RFM95_RST 9  // "A"
#define RFM95_CS 10  // "B"
#define RFM95_INT 4  // "C"
#endif

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0
// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

const int NUM_UNITS = 11;
const int PER_UNIT_DELAY = 200;
const int TIMEOUT_DELAY = 500;

uint8_t currentState[NUM_UNITS + 1];

void setup() {

  for (int i = 0; i <= NUM_UNITS; i++) {
    currentState[i] = 0;
  }

  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  delay(100);

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);


  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: ");
  Serial.println(RF95_FREQ);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
}

void loop() {

  // Buffer is size 4 to allow for future functionality
  uint8_t buf[4];
  uint8_t len = sizeof(buf);

  for (int i = 0; i < NUM_UNITS; i++) {

    // Prepare state for sending
    currentState[NUM_UNITS] = i;
    rf95.send(currentState, NUM_UNITS + 1);
    rf95.waitPacketSent();

    if (rf95.waitAvailableTimeout(TIMEOUT_DELAY) && rf95.recv(buf, &len)) {
      // Update state
      if (currentState[i] != buf[0]) Serial.print("Detected toggle on box "); Serial.println(i);
      currentState[i] = buf[0];
    } else {
      // Skip
      Serial.print("Unable to reach box "); Serial.println(i);
    }

    delay(PER_UNIT_DELAY);
  }
}
