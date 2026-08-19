#include <Arduino.h>
#include <RadioLib.h>
#include <SPI.h>

namespace {
constexpr uint8_t kPing = 1;
constexpr uint8_t kAck = 2;
constexpr size_t kFrameSize = 7;

SPIClass radio_spi(NRF_SPIM1, 45, 43, 44);  // MISO P1.13, SCK P1.11, MOSI P1.12
SX1262 radio = new Module(42, 47, 38, 46, radio_spi);  // NSS, DIO1, reset, busy
uint32_t sequence = 0;

void encode(uint8_t* frame, uint8_t type, uint32_t seq) {
  frame[0] = 'L'; frame[1] = 'T'; frame[2] = type;
  for (int i = 0; i < 4; ++i) frame[3 + i] = static_cast<uint8_t>(seq >> (8 * i));
}

bool matches(const uint8_t* frame, uint8_t type, uint32_t seq) {
  if (frame[0] != 'L' || frame[1] != 'T' || frame[2] != type) return false;
  uint32_t decoded = 0;
  for (int i = 0; i < 4; ++i) decoded |= static_cast<uint32_t>(frame[3 + i]) << (8 * i);
  return decoded == seq;
}
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("RAK4631 -> Heltec US915 link test");
  radio_spi.begin();
  const int state = radio.begin(915.0F, 125.0F, 9, 5, 0x12, 14, 8, 1.6F, false);
  Serial.print("SX1262 init: "); Serial.println(state);
  if (state != RADIOLIB_ERR_NONE) while (true) delay(1000);
  radio.setCRC(true);
}

void loop() {
  uint8_t ping[kFrameSize];
  encode(ping, kPing, ++sequence);
  int state = radio.transmit(ping, sizeof(ping));
  Serial.print("TX seq="); Serial.print(sequence); Serial.print(" state="); Serial.print(state);
  if (state == RADIOLIB_ERR_NONE) {
    uint8_t ack[kFrameSize]{};
    state = radio.receive(ack, sizeof(ack), 1000);
    if (state == RADIOLIB_ERR_NONE && matches(ack, kAck, sequence)) {
      Serial.print(" ACK rssi="); Serial.print(radio.getRSSI());
      Serial.print(" snr="); Serial.println(radio.getSNR());
    } else {
      Serial.print(" no-ack state="); Serial.println(state);
    }
  } else {
    Serial.println();
  }
  radio.sleep();
  delay(3000);
}

