#pragma once

#include <cstdint>

namespace node_config {
constexpr uint8_t kNodeId = 1;
constexpr uint8_t kAds1115Address = 0x48;
constexpr float kShuntOhms = 100.0F;
constexpr float kCalibrationSlopeMPerV = 3.125F;
constexpr float kCalibrationOffsetM = -1.25F;
constexpr uint32_t kSensorWarmupMs = 3000;
constexpr uint32_t kSampleIntervalSeconds = 15U * 60U;
constexpr int kAdcSampleCount = 32;
constexpr int kAdcTrimEachEnd = 4;
constexpr uint8_t kTemperatureResolutionBits = 12;
constexpr float kTemperatureMinC = -20.0F;
constexpr float kTemperatureMaxC = 50.0F;
constexpr bool kEnableV32BatteryDivider = true;
constexpr float kBatteryDividerRatio = 4.9F;
constexpr float kBatteryCorrection = 1.0F;
constexpr uint16_t kBatteryLowMv = 3400;
constexpr float kLoRaFrequencyMhz = 915.0F;
constexpr float kLoRaBandwidthKhz = 125.0F;
constexpr uint8_t kLoRaSpreadingFactor = 9;
constexpr uint8_t kLoRaCodingRate = 5;
constexpr int8_t kLoRaTxPowerDbm = 14;
constexpr uint8_t kLoRaSyncWord = 0x12;
constexpr bool kSerialDebug = true;
}  // namespace node_config
