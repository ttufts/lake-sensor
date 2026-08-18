#include "measurement.h"

#include <algorithm>
#include <cmath>

#include "lake_protocol.h"

namespace lake {

float trimmed_mean(float* samples, std::size_t count, std::size_t trim_each_end) {
  if (samples == nullptr || count == 0 || trim_each_end * 2 >= count) return NAN;
  std::sort(samples, samples + count);
  float sum = 0.0F;
  for (std::size_t i = trim_each_end; i < count - trim_each_end; ++i) {
    sum += samples[i];
  }
  return sum / static_cast<float>(count - 2 * trim_each_end);
}

float sample_stddev(const float* samples, std::size_t count, float mean) {
  if (samples == nullptr || count < 2) return 0.0F;
  float squared_error = 0.0F;
  for (std::size_t i = 0; i < count; ++i) {
    const float error = samples[i] - mean;
    squared_error += error * error;
  }
  return std::sqrt(squared_error / static_cast<float>(count - 1));
}

Measurement convert_measurement(float sense_v, float stddev_v,
                                float shunt_ohms,
                                const Calibration& calibration) {
  Measurement result{};
  result.sense_v = sense_v;
  result.sample_stddev_v = stddev_v;
  if (!std::isfinite(sense_v) || sense_v < 0.0F || sense_v > 3.1F ||
      !std::isfinite(shunt_ohms) || shunt_ohms <= 0.0F) {
    result.flags |= kAdcInvalid;
    return result;
  }
  result.loop_ma = 1000.0F * sense_v / shunt_ohms;
  if (result.loop_ma < 3.6F) result.flags |= kSensorUndercurrent;
  if (result.loop_ma > 21.0F) result.flags |= kSensorOvercurrent;
  if (!std::isfinite(calibration.slope_m_per_v) ||
      !std::isfinite(calibration.offset_m) || calibration.slope_m_per_v <= 0.0F) {
    result.flags |= kCalibrationInvalid;
    return result;
  }
  result.depth_m = calibration.slope_m_per_v * sense_v + calibration.offset_m;
  return result;
}

}  // namespace lake

