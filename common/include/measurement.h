#pragma once

#include <cstddef>
#include <cstdint>

namespace lake {

struct Calibration {
  float slope_m_per_v = 3.125F;
  float offset_m = -1.25F;
};

struct Measurement {
  float sense_v = 0.0F;
  float loop_ma = 0.0F;
  float depth_m = 0.0F;
  float sample_stddev_v = 0.0F;
  uint16_t flags = 0;
};

float trimmed_mean(float* samples, std::size_t count, std::size_t trim_each_end);
float sample_stddev(const float* samples, std::size_t count, float mean);
Measurement convert_measurement(float sense_v, float stddev_v,
                                float shunt_ohms,
                                const Calibration& calibration);

}  // namespace lake

