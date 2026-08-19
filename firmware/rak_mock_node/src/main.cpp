#include <Arduino.h>
#include <DHT.h>
#include <RadioLib.h>
#include <SPI.h>

#include <cmath>

#include "lake_protocol.h"

namespace {
constexpr uint8_t kNodeId = 1;
constexpr uint8_t kDhtPin = WB_IO1;
// RAK19007 J11 exposes AIN1 (the older RAK5005-O exposed AIN0).
constexpr uint8_t kPotPin = WB_A1;
constexpr uint8_t kAckType = 2;
constexpr size_t kAckSize = 7;

SPIClass radio_spi(NRF_SPIM1, 45, 43, 44);
SX1262 radio = new Module(42, 47, 38, 46, radio_spi);
DHT dht(kDhtPin, DHT22);
uint32_t sequence = 0;

bool valid_ack(const uint8_t* ack, uint32_t expected) {
  if (ack[0] != 'L' || ack[1] != 'T' || ack[2] != kAckType) return false;
  uint32_t decoded = 0;
  for (int i = 0; i < 4; ++i) decoded |= static_cast<uint32_t>(ack[3 + i]) << (8 * i);
  return decoded == expected;
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(1500);
  dht.begin();
  analogReadResolution(12);
  radio_spi.begin();
  const int state = radio.begin(915.0F, 125.0F, 9, 5, 0x12, 14, 8, 1.6F,
                                false);
  Serial.printf("RAK mock-node radio init: %d\n", state);
  if (state != RADIOLIB_ERR_NONE) while (true) delay(1000);
  radio.setCRC(true);
}

void loop() {
  const uint16_t pot_raw = analogRead(kPotPin);
  const float position = pot_raw / 4095.0F;
  const float temperature_c = dht.readTemperature();
  lake::PacketV1 packet{};
  packet.node_id = kNodeId;
  packet.sequence = ++sequence;
  packet.depth_mm = lroundf(position * 5000.0F);
  packet.loop_ua = lroundf(4000.0F + position * 16000.0F);
  packet.sense_mv = lroundf(packet.loop_ua / 10.0F);  // 100-ohm shunt.
  packet.battery_mv = 3900;
  if (std::isfinite(temperature_c)) {
    packet.temperature_centi_c = lroundf(temperature_c * 100.0F);
  } else {
    packet.flags |= lake::kTemperatureInvalid;
  }
  const auto bytes = lake::encode_packet(packet);
  int state = radio.transmit(bytes.data(), bytes.size());
  Serial.printf("tx seq=%lu pot=%u depth=%.3f temp=%.2f state=%d",
                static_cast<unsigned long>(packet.sequence), pot_raw,
                packet.depth_mm / 1000.0F, temperature_c, state);
  if (state == RADIOLIB_ERR_NONE) {
    uint8_t ack[kAckSize]{};
    state = radio.receive(ack, sizeof(ack), 1000);
    if (state == RADIOLIB_ERR_NONE && valid_ack(ack, packet.sequence)) {
      Serial.printf(" ack rssi=%.1f snr=%.2f\n", radio.getRSSI(), radio.getSNR());
    } else {
      Serial.printf(" no-ack state=%d\n", state);
    }
  } else {
    Serial.println();
  }
  radio.sleep();
  delay(5000);
}
