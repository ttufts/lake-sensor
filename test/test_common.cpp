#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>

#include "lake_protocol.h"
#include "measurement.h"

namespace {
bool near(float actual, float expected, float tolerance = 0.0001F) {
  return std::fabs(actual - expected) <= tolerance;
}
}

int main() {
  lake::PacketV1 source{7, 0x12345678U, -125, 400, 4000, 3912,
                        lake::kSensorUndercurrent};
  const auto bytes = lake::encode_packet(source);
  static_assert(bytes.size() == 20);
  assert(bytes[0] == 0x4B && bytes[1] == 0x4C);
  lake::PacketV1 decoded{};
  assert(lake::decode_packet(bytes.data(), bytes.size(), decoded));
  assert(decoded.node_id == source.node_id);
  assert(decoded.sequence == source.sequence);
  assert(decoded.depth_mm == source.depth_mm);
  assert(decoded.battery_mv == source.battery_mv);
  assert(!lake::decode_packet(bytes.data(), bytes.size() - 1, decoded));

  float samples[] = {99.0F, 1.0F, 1.1F, 0.9F, -50.0F};
  assert(near(lake::trimmed_mean(samples, 5, 1), 1.0F));
  const lake::Calibration nominal{};
  auto mid = lake::convert_measurement(1.2F, 0.001F, 100.0F, nominal);
  assert(near(mid.loop_ma, 12.0F));
  assert(near(mid.depth_m, 2.5F));
  assert(mid.flags == 0);
  auto open = lake::convert_measurement(0.2F, 0.0F, 100.0F, nominal);
  assert(open.flags & lake::kSensorUndercurrent);
  auto rail = lake::convert_measurement(3.2F, 0.0F, 100.0F, nominal);
  assert(rail.flags & lake::kAdcInvalid);
  std::cout << "common tests passed\n";
}
