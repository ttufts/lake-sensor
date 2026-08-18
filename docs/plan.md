# Delivery Plan

## Milestone 1 — Repository and deterministic core

- Establish PlatformIO node/gateway environments and configuration conventions.
- Specify and unit-test version-1 packet serialization.
- Implement unit-testable filtering, current conversion, calibration, and flags.
- Document wiring, safety, design, calibration, and deployment.

Exit: host tests pass and node firmware compiles when PlatformIO is available.

## Milestone 2 — Bench sensor acquisition

- Validate GPIO36/Vext behavior and boost startup with a dummy load.
- Confirm ADS1115 address, gain, rate, voltage accuracy, and noisy-sample output.
- Measure sensor warm-up and implement a settling diagnostic.
- Validate V3/V3.2 battery-divider handling and calibration.

Exit: no reset at maximum loop load; readings agree with a meter within 0.5%;
open/overcurrent/rail faults are repeatable.

## Milestone 3 — LoRa link and reliability

- Implement gateway packet receiver/decoder and serial diagnostics.
- Add ACK schema, receive window, retry limit, and retry flag.
- Log RSSI/SNR, duplicates, malformed packets, and sequence gaps.
- Range-test and select compliant frequency/SF/power settings.

Exit: reliable delivery over the real lake-to-house path with diagnosable loss.

## Milestone 4 — MQTT gateway

- Add externalized Wi-Fi/MQTT configuration and reconnection state machines.
- Publish retained QoS-1 state and separate availability/diagnostic topics.
- Reject malformed packets and avoid overwriting the latest valid state with
  invalid sensor readings.
- Document Home Assistant/InfluxDB integration and gateway operations.

Exit: broker restarts and Wi-Fi interruptions recover without manual action.

## Milestone 5 — Power characterization

- Measure sleep current with Vext off and energy for the complete wake cycle.
- Optimize warm-up, diagnostics, ACK window, and radio settings from measurements.
- Verify USB charging and protected-cell polarity; create a measured runtime model.

Exit: sleep below 100 µA and a documented runtime projection from measured data.

## Milestone 6 — Calibration and field pilot

- Fit and retain a multi-point calibration dataset and residual analysis.
- Install fixed probe/stilling well and establish a manual benchmark.
- Monitor drift, condensation/venting, packet loss, and battery voltage.

Exit: 1–2 cm initial fixed-depth repeatability and acceptable manual-gauge
agreement for trend monitoring.

## Deferred hardening

Surge protection, winter-safe solar charging, enclosure environmental testing,
remote configuration, and deciding whether LoRaWAN/Meshtastic integration has a
long-term advantage all follow the functional field pilot.

