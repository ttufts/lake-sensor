#include <Arduino.h>
#include <Adafruit_ADS1X15.h>
#include <RadioLib.h>
#include <SPI.h>
#include <Wire.h>

#include <cmath>

#include "lake_protocol.h"

namespace {
constexpr uint8_t kNodeId = 1;
// RAK19003 J7 RXD, confirmed on this base/core as P0.19. Serial2 is unused.
constexpr uint8_t kTemperaturePin = PIN_SERIAL2_RX;
constexpr uint8_t kAds1115Address = 0x48;
constexpr uint8_t kAds1115Channel = 0;
constexpr uint8_t kAdcSamples = 8;
constexpr float kShuntOhms = 100.0F;
// Bench calibration on 2026-08-22 using dry, 3, 4, 5, and 7 inch points.
// This fit assumes the pressure transmitter remains continuously powered.
constexpr float kLoopMinUa = 3771.0F;
constexpr float kLoopSpanUa = 15125.0F;
constexpr float kLoopUndercurrentUa = 3600.0F;
constexpr float kLoopOvercurrentUa = 21000.0F;
constexpr float kDepthRangeMm = 5000.0F;
// This probe read about +5 C in an ice slurry and +4.5 C against two
// co-located Shelly references near room temperature.
constexpr float kTemperatureOffsetC = -5.0F;
constexpr float kBatteryMvPerCount = 1.73F * 0.73242188F;
constexpr uint8_t kBatterySamples = 8;
constexpr uint16_t kBatteryLowMv = 3400;
constexpr uint8_t kAckType = 2;
constexpr size_t kAckSize = 7;

SPIClass radio_spi(NRF_SPIM1, 45, 43, 44);
SX1262 radio = new Module(42, 47, 38, 46, radio_spi);
Adafruit_ADS1115 ads;
uint32_t sequence = 0;
bool ads_available = false;

bool one_wire_reset() {
  pinMode(kTemperaturePin, OUTPUT);
  digitalWrite(kTemperaturePin, LOW);
  delayMicroseconds(480);
  pinMode(kTemperaturePin, INPUT_PULLUP);
  delayMicroseconds(70);
  const bool present = digitalRead(kTemperaturePin) == LOW;
  delayMicroseconds(410);
  return present;
}

void one_wire_write_bit(bool bit) {
  pinMode(kTemperaturePin, OUTPUT);
  digitalWrite(kTemperaturePin, LOW);
  delayMicroseconds(bit ? 6 : 60);
  pinMode(kTemperaturePin, INPUT_PULLUP);
  delayMicroseconds(bit ? 64 : 10);
}

bool one_wire_read_bit() {
  pinMode(kTemperaturePin, OUTPUT);
  digitalWrite(kTemperaturePin, LOW);
  delayMicroseconds(3);
  pinMode(kTemperaturePin, INPUT_PULLUP);
  delayMicroseconds(10);
  const bool bit = digitalRead(kTemperaturePin);
  delayMicroseconds(53);
  return bit;
}

void one_wire_write_byte(uint8_t value) {
  for (uint8_t i = 0; i < 8; ++i) {
    one_wire_write_bit(value & 1U);
    value >>= 1;
  }
}

uint8_t one_wire_read_byte() {
  uint8_t value = 0;
  for (uint8_t i = 0; i < 8; ++i) value |= one_wire_read_bit() << i;
  return value;
}

uint8_t one_wire_crc8(const uint8_t* data, uint8_t length) {
  uint8_t crc = 0;
  while (length--) {
    uint8_t in = *data++;
    for (uint8_t i = 0; i < 8; ++i) {
      const uint8_t mix = (crc ^ in) & 1U;
      crc >>= 1;
      if (mix) crc ^= 0x8CU;
      in >>= 1;
    }
  }
  return crc;
}

bool read_temperature(float& temperature_c) {
  if (!one_wire_reset()) return false;
  one_wire_write_byte(0xCC);  // Skip ROM; exactly one probe is connected.
  one_wire_write_byte(0x44);  // Start conversion (externally powered mode).
  delay(750);
  if (!one_wire_reset()) return false;
  one_wire_write_byte(0xCC);
  one_wire_write_byte(0xBE);  // Read scratchpad.
  uint8_t scratchpad[9];
  for (uint8_t& byte : scratchpad) byte = one_wire_read_byte();
  if (one_wire_crc8(scratchpad, 8) != scratchpad[8]) return false;
  const int16_t raw = static_cast<int16_t>(
      static_cast<uint16_t>(scratchpad[0]) |
      (static_cast<uint16_t>(scratchpad[1]) << 8));
  temperature_c = raw / 16.0F + kTemperatureOffsetC;
  return temperature_c >= -20.0F && temperature_c <= 50.0F;
}

bool init_ads1115() {
  if (!ads.begin(kAds1115Address, &Wire)) return false;
  ads.setGain(GAIN_ONE);  // +/-4.096 V: safe for a 0-3.3 V potentiometer.
  ads.setDataRate(RATE_ADS1115_128SPS);
  return true;
}

uint16_t read_battery_mv() {
  uint32_t sum = 0;
  for (uint8_t i = 0; i < kBatterySamples; ++i) sum += analogRead(WB_A0);
  return static_cast<uint16_t>(lroundf(
      (sum / static_cast<float>(kBatterySamples)) * kBatteryMvPerCount));
}

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
  Serial.println("RAK DS18B20 node startup");
  analogReference(AR_INTERNAL_3_0);
  analogReadResolution(12);
  analogRead(WB_A0);  // Discard the first sample while the ADC settles.
  pinMode(kTemperaturePin, INPUT_PULLUP);
  Serial.printf("DS18B20: %s\n", one_wire_reset() ? "found" : "not found");
  Wire.begin();
  ads_available = init_ads1115();
  Serial.printf("ADS1115 0x%02X: %s\n", kAds1115Address,
                ads_available ? "ready" : "not found");
  radio_spi.begin();
  const int state = radio.begin(915.0F, 125.0F, 9, 5, 0x12, 14, 8, 1.6F,
                                false);
  Serial.printf("RAK mock-node radio init: %d\n", state);
  if (state != RADIOLIB_ERR_NONE) while (true) delay(1000);
  radio.setCRC(true);
}

void loop() {
  // Re-probe so corrected bench wiring starts working without another reset.
  if (!ads_available) ads_available = init_ads1115();
  int16_t pot_raw = 0;
  float pot_volts = 0.0F;
  if (ads_available) {
    int32_t raw_sum = 0;
    for (uint8_t i = 0; i < kAdcSamples; ++i) {
      raw_sum += ads.readADC_SingleEnded(kAds1115Channel);
    }
    pot_raw = static_cast<int16_t>(raw_sum / kAdcSamples);
    pot_volts = ads.computeVolts(pot_raw);
  }
  const float loop_ua = pot_volts * 1000000.0F / kShuntOhms;
  const float position = constrain((loop_ua - kLoopMinUa) / kLoopSpanUa,
                                   0.0F, 1.0F);
  float temperature_c = NAN;
  const bool temperature_valid = read_temperature(temperature_c);
  const uint16_t battery_mv = read_battery_mv();
  lake::PacketV1 packet{};
  packet.node_id = kNodeId;
  packet.sequence = ++sequence;
  packet.depth_mm = lroundf(position * kDepthRangeMm);
  packet.loop_ua = constrain(lroundf(loop_ua), 0L, 65535L);
  packet.sense_mv = constrain(lroundf(pot_volts * 1000.0F), 0L, 65535L);
  packet.battery_mv = battery_mv;
  if (!ads_available) packet.flags |= lake::kAds1115Missing;
  if (ads_available && loop_ua < kLoopUndercurrentUa) {
    packet.flags |= lake::kSensorUndercurrent;
  }
  if (ads_available && loop_ua > kLoopOvercurrentUa) {
    packet.flags |= lake::kSensorOvercurrent;
  }
  if (battery_mv != 0 && battery_mv < kBatteryLowMv) {
    packet.flags |= lake::kBatteryLow;
  }
  if (temperature_valid) {
    packet.temperature_centi_c = lroundf(temperature_c * 100.0F);
  } else {
    packet.flags |= lake::kTemperatureInvalid;
  }
  const auto bytes = lake::encode_packet(packet);
  int state = radio.transmit(bytes.data(), bytes.size());
  Serial.printf("tx seq=%lu ads=%s adc=%d sense=%.4fV loop=%.3fmA depth=%.3fm temp=%.2f battery=%umV flags=0x%04x state=%d",
                static_cast<unsigned long>(packet.sequence),
                ads_available ? "ready" : "missing", pot_raw, pot_volts,
                packet.loop_ua / 1000.0F, packet.depth_mm / 1000.0F,
                temperature_c, packet.battery_mv, packet.flags, state);
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
