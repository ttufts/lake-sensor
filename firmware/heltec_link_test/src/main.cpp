#include <Arduino.h>
#include <RadioLib.h>
#include <SPI.h>

#include "pins.h"

namespace {
constexpr uint8_t kPing = 1;
constexpr uint8_t kAck = 2;
constexpr size_t kFrameSize = 7;

SPIClass radio_spi(FSPI);
SX1262 radio = new Module(node_pins::kLoRaNss, node_pins::kLoRaDio1,
                          node_pins::kLoRaReset, node_pins::kLoRaBusy,
                          radio_spi);

bool decode(const uint8_t* frame, uint8_t type, uint32_t& seq) {
  if (frame[0] != 'L' || frame[1] != 'T' || frame[2] != type) return false;
  seq = 0;
  for (int i = 0; i < 4; ++i) seq |= static_cast<uint32_t>(frame[3 + i]) << (8 * i);
  return true;
}

void encode(uint8_t* frame, uint8_t type, uint32_t seq) {
  frame[0] = 'L'; frame[1] = 'T'; frame[2] = type;
  for (int i = 0; i < 4; ++i) frame[3 + i] = static_cast<uint8_t>(seq >> (8 * i));
}
}

void setup() {
  pinMode(node_pins::kVextControl, OUTPUT);
  digitalWrite(node_pins::kVextControl, HIGH);
  Serial.begin(115200);
  delay(1000);
  Serial.println("Heltec US915 receiver/ACK link test");
  radio_spi.begin(node_pins::kLoRaSck, node_pins::kLoRaMiso,
                  node_pins::kLoRaMosi, node_pins::kLoRaNss);
  const int state = radio.begin(915.0F, 125.0F, 9, 5, 0x12, 14);
  Serial.print("SX1262 init: "); Serial.println(state);
  if (state != RADIOLIB_ERR_NONE) while (true) delay(1000);
  radio.setCRC(true);
}

void loop() {
  uint8_t frame[kFrameSize]{};
  const int state = radio.receive(frame, sizeof(frame), 5000);
  if (state == RADIOLIB_ERR_RX_TIMEOUT) return;
  uint32_t sequence = 0;
  if (state != RADIOLIB_ERR_NONE || !decode(frame, kPing, sequence)) {
    Serial.print("RX rejected state="); Serial.println(state);
    return;
  }
  const float rssi = radio.getRSSI();
  const float snr = radio.getSNR();
  uint8_t ack[kFrameSize];
  encode(ack, kAck, sequence);
  delay(25);
  const int ack_state = radio.transmit(ack, sizeof(ack));
  Serial.print("RX seq="); Serial.print(sequence);
  Serial.print(" rssi="); Serial.print(rssi);
  Serial.print(" snr="); Serial.print(snr);
  Serial.print(" ACK state="); Serial.println(ack_state);
}

