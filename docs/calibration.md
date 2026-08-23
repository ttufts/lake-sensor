# Calibration and Field Validation

## Installed pressure calibration

The installed two-wire 0–5 m transmitter was calibrated on 2026-08-22 with a
100 Ω shunt and continuously powered MT3608. The cap cavity must be flooded.

| Applied water depth | Loop current |
|---:|---:|
| Dry, initial | 3.711 mA |
| 3 in | 3.984 mA |
| 4 in | 4.056 mA |
| 5 in | 4.175 mA |
| 7 in | 4.315 mA |
| Dry after warm-up | 3.785 mA |

The warmed least-squares fit used by `rak_mock_node` is:

```text
zero current = 3.771 mA
5 m span     = 15.125 mA
depth_m      = clamp((loop_uA - 3771) / 15125, 0, 1) * 5
```

The initial real-lake check reported 15.47 inches at a manually measured depth
of about 16 inches. Preserve `sense_v` and `loop_ma` telemetry so future fits
can be evaluated without losing the raw signal.

The transmitter's zero changes during warm-up. This fit assumes it remains
continuously powered. Recalibrate after adding pressure-loop power switching.

## How to repeat pressure calibration

1. Power the complete node long enough to reach its normal operating state.
2. Remove, drain, and dry the probe; record at least five dry readings.
3. Flood the protective cap and install it underwater.
4. Fix the diaphragm elevation and measure water height from the diaphragm.
5. Record several stable readings at multiple depths spanning the intended
   range; do not calibrate from the already-converted depth entity.
6. Fit loop current versus known depth, inspect residuals, and repeat the dry
   point to quantify hysteresis.
7. Update `kLoopMinUa` and `kLoopSpanUa`, build, flash, and validate a point not
   used in the fit.

## Installed temperature calibration

The installed DS18B20-compatible probe read about +5 °C in a dense ice slurry
and +4.5 °C against two co-located Shelly references. Firmware therefore
applies:

```text
kTemperatureOffsetC = -5.0 °C
```

The first field check was about 81 °F by reference and 81.5 °F from the node.
This large correction indicates a low-cost compatible or defective probe; test
replacement probes rather than copying the offset automatically.

For an ice point, pack ice around the whole metal capsule, add only enough water
to fill gaps, stir, and keep the probe off the vessel walls. Keep cable splices
dry. Compare again near expected lake temperatures before accepting a probe.

## Installation datum

Fix the pressure diaphragm to a stable dock member, post, or lakebed fixture.
Record its elevation relative to a permanent benchmark. If the sensor moves,
the reported water depth changes even when lake surface elevation does not.
