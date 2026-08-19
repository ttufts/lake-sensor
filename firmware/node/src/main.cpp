#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <RadioLib.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <esp_sleep.h>

#include <algorithm>
#include <cmath>

#include "lake_protocol.h"
#include "measurement.h"
#include "node_config.h"
#include "pins.h"

RTC_DATA_ATTR uint32_t sequence_number = 0;

Adafruit_ADS1115 ads;
SPIClass radio_spi(FSPI);
SX1262 radio = new Module(node_pins::kLoRaNss, node_pins::kLoRaDio1,
                          node_pins::kLoRaReset, node_pins::kLoRaBusy,
                          radio_spi);
OneWire temperature_bus(node_pins::kTemperatureData);
DallasTemperature temperature_sensors(&temperature_bus);

namespace {

uint16_t clamp_u16(float value) {
  if (!std::isfinite(value) || value <= 0.0F) return 0;
  if (value >= 65535.0F) return 65535;
  return static_cast<uint16_t>(std::lround(value));
}

int32_t clamp_i32(float value) {
  if (!std::isfinite(value)) return 0;
  if (value >= 2147483647.0F) return INT32_MAX;
  if (value <= -2147483648.0F) return INT32_MIN;
  return static_cast<int32_t>(std::lround(value));
}

int16_t clamp_i16(float value) {
  if (!std::isfinite(value)) return 0;
  if (value >= 32767.0F) return INT16_MAX;
  if (value <= -32768.0F) return INT16_MIN;
  return static_cast<int16_t>(std::lround(value));
}

uint16_t read_battery_mv() {
  if (!node_config::kEnableV32BatteryDivider) return 0;
  pinMode(node_pins::kBatteryDividerEnable, OUTPUT);
  digitalWrite(node_pins::kBatteryDividerEnable, HIGH);
  delay(5);
  analogReadResolution(12);
  const uint32_t pin_mv = analogReadMilliVolts(node_pins::kBatteryAdc);
  digitalWrite(node_pins::kBatteryDividerEnable, LOW);
  return clamp_u16(pin_mv * node_config::kBatteryDividerRatio *
                   node_config::kBatteryCorrection);
}

void sensor_power(bool enabled) {
  pinMode(node_pins::kVextControl, OUTPUT);
  digitalWrite(node_pins::kVextControl, enabled ? LOW : HIGH);
}

lake::Measurement acquire_measurement(uint16_t& flags, float& temperature_c) {
  sensor_power(true);
  delay(75);
  temperature_sensors.begin();
  const bool has_temperature_sensor = temperature_sensors.getDeviceCount() > 0;
  if (!has_temperature_sensor) {
    flags |= lake::kTemperatureInvalid;
  } else {
    temperature_sensors.setResolution(node_config::kTemperatureResolutionBits);
    temperature_sensors.setWaitForConversion(false);
    temperature_sensors.requestTemperatures();
  }
  Wire.begin(node_pins::kI2cSda, node_pins::kI2cScl);
  if (!ads.begin(node_config::kAds1115Address, &Wire)) {
    flags |= lake::kAds1115Missing;
    Wire.end();
    pinMode(node_pins::kTemperatureData, INPUT);
    sensor_power(false);
    return {};
  }
  ads.setGain(GAIN_ONE);
  ads.setDataRate(RATE_ADS1115_128SPS);
  delay(node_config::kSensorWarmupMs);

  temperature_c = has_temperature_sensor
                      ? temperature_sensors.getTempCByIndex(0)
                      : DEVICE_DISCONNECTED_C;
  if (!std::isfinite(temperature_c) || temperature_c == DEVICE_DISCONNECTED_C ||
      temperature_c < node_config::kTemperatureMinC ||
      temperature_c > node_config::kTemperatureMaxC) {
    flags |= lake::kTemperatureInvalid;
    temperature_c = 0.0F;
  }

  float samples[node_config::kAdcSampleCount];
  for (int i = 0; i < node_config::kAdcSampleCount; ++i) {
    samples[i] = ads.computeVolts(ads.readADC_SingleEnded(0));
    if (node_config::kSerialDebug) Serial.printf("adc[%d]=%.6f V\n", i, samples[i]);
  }
  float raw_samples[node_config::kAdcSampleCount];
  std::copy(samples, samples + node_config::kAdcSampleCount, raw_samples);
  const float mean = lake::trimmed_mean(samples, node_config::kAdcSampleCount,
                                        node_config::kAdcTrimEachEnd);
  const float stddev = lake::sample_stddev(raw_samples,
                                           node_config::kAdcSampleCount, mean);
  Wire.end();
  pinMode(node_pins::kTemperatureData, INPUT);
  pinMode(node_pins::kI2cSda, INPUT);
  pinMode(node_pins::kI2cScl, INPUT);
  sensor_power(false);

  lake::Calibration calibration{};
  calibration.slope_m_per_v = node_config::kCalibrationSlopeMPerV;
  calibration.offset_m = node_config::kCalibrationOffsetM;
  lake::Measurement measurement = lake::convert_measurement(
      mean, stddev, node_config::kShuntOhms, calibration);
  flags |= measurement.flags;
  return measurement;
}

bool transmit(const lake::PacketV1& packet) {
  radio_spi.begin(node_pins::kLoRaSck, node_pins::kLoRaMiso,
                  node_pins::kLoRaMosi, node_pins::kLoRaNss);
  int state = radio.begin(node_config::kLoRaFrequencyMhz,
                          node_config::kLoRaBandwidthKhz,
                          node_config::kLoRaSpreadingFactor,
                          node_config::kLoRaCodingRate,
                          node_config::kLoRaSyncWord,
                          node_config::kLoRaTxPowerDbm);
  if (state != RADIOLIB_ERR_NONE) {
    if (node_config::kSerialDebug) Serial.printf("radio begin failed: %d\n", state);
    return false;
  }
  radio.setCRC(true);
  const auto encoded = lake::encode_packet(packet);
  state = radio.transmit(encoded.data(), encoded.size());
  radio.sleep();
  if (node_config::kSerialDebug) Serial.printf("radio transmit: %d\n", state);
  return state == RADIOLIB_ERR_NONE;
}

void sleep_now() {
  sensor_power(false);
  esp_sleep_enable_timer_wakeup(
      static_cast<uint64_t>(node_config::kSampleIntervalSeconds) * 1000000ULL);
  if (node_config::kSerialDebug) {
    Serial.printf("sleeping for %lu seconds\n",
                  static_cast<unsigned long>(node_config::kSampleIntervalSeconds));
    Serial.flush();
  }
  esp_deep_sleep_start();
}

}  // namespace

void setup() {
  sensor_power(false);
  if (node_config::kSerialDebug) {
    Serial.begin(115200);
    delay(100);
  }
  ++sequence_number;
  uint16_t flags = 0;
  const uint16_t battery_mv = read_battery_mv();
  if (battery_mv != 0 && battery_mv < node_config::kBatteryLowMv) {
    flags |= lake::kBatteryLow;
  }
  float temperature_c = 0.0F;
  const lake::Measurement measurement = acquire_measurement(flags, temperature_c);
  lake::PacketV1 packet{};
  packet.node_id = node_config::kNodeId;
  packet.sequence = sequence_number;
  packet.depth_mm = clamp_i32(measurement.depth_m * 1000.0F);
  packet.sense_mv = clamp_u16(measurement.sense_v * 1000.0F);
  packet.loop_ua = clamp_u16(measurement.loop_ma * 1000.0F);
  packet.battery_mv = battery_mv;
  packet.temperature_centi_c = clamp_i16(temperature_c * 100.0F);
  packet.flags = flags;

  if (node_config::kSerialDebug) {
    Serial.printf("seq=%lu depth=%.3f m loop=%.3f mA water=%.2f C battery=%u mV flags=0x%04x\n",
                  static_cast<unsigned long>(packet.sequence),
                  measurement.depth_m, measurement.loop_ma, temperature_c,
                  packet.battery_mv, packet.flags);
  }
  transmit(packet);
  sleep_now();
}

void loop() {}
