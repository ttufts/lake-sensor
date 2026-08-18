#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace lake {

constexpr uint16_t kPacketMagic = 0x4C4B;  // "LK" on the wire, little-endian.
constexpr uint8_t kPacketVersion = 1;
constexpr std::size_t kPacketV1Size = 20;

enum StatusFlag : uint16_t {
  kSensorUndercurrent = 1U << 0,
  kSensorOvercurrent = 1U << 1,
  kAdcInvalid = 1U << 2,
  kBatteryLow = 1U << 3,
  kAds1115Missing = 1U << 4,
  kSensorUnsettled = 1U << 5,
  kCalibrationInvalid = 1U << 6,
  kRetryPacket = 1U << 7,
};

struct PacketV1 {
  uint8_t node_id = 0;
  uint32_t sequence = 0;
  int32_t depth_mm = 0;
  uint16_t sense_mv = 0;
  uint16_t loop_ua = 0;
  uint16_t battery_mv = 0;
  uint16_t flags = 0;
};

using EncodedPacketV1 = std::array<uint8_t, kPacketV1Size>;

EncodedPacketV1 encode_packet(const PacketV1& packet);
bool decode_packet(const uint8_t* data, std::size_t size, PacketV1& packet);

}  // namespace lake
