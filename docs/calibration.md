# Calibration

## Bench calibration

Record ADS1115 voltage at known water heads, ideally 0, 0.5, 1, 2, 3, 4, and
5 m. Let each point settle and record water temperature. Fit:

```text
depth_m = slope_m_per_v * sense_v + offset_m
```

Inspect residuals before considering nonlinear correction; unexpected
nonlinearity is more likely a sensor, installation, vent, or power problem.
Copy the accepted coefficients into a local `config.h` derived from
`config.example.h` and preserve the calibration dataset outside firmware.

## Installation datum

Fix the probe to land, a post, or a lakebed anchor—not a floating dock. Record
its elevation relative to a permanent benchmark and take a simultaneous manual
staff-gauge reading. Surface elevation is fixed sensor elevation plus measured
water head.

Compare weekly during initial deployment and recalibrate after the sensor is
moved, replaced, buried, or its vent arrangement changes.

## Temperature validation

Record the DS18B20's unique ROM address before installation. Compare candidate
probes in well-stirred ice water and beside a trusted reference at room
temperature. Record offsets and reject unstable outliers. Treat the DS18B20's
85 °C power-on value as invalid. Repeat validation after cable or encapsulation
repairs.
