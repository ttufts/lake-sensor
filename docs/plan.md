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
- Validate DS18B20 discovery, readings, conversion timing, disconnect behavior,
  and lack of back-powering after Vext shuts off.
- Measure sensor warm-up and implement a settling diagnostic.
- Validate V3/V3.2 battery-divider handling and calibration.

Exit: no reset at maximum loop load; readings agree with a meter within 0.5%;
open/overcurrent/rail faults are repeatable.

## Milestone 3 — LoRa link and reliability

- [x] Establish a bidirectional raw-LoRa bench link from the RAK4631 lake-node
  candidate to the Heltec gateway candidate.
- [x] Verify sequence-numbered frames, ACKs, RSSI, and SNR on both consoles.
- Implement gateway packet receiver/decoder and serial diagnostics.
- Add ACK schema, receive window, retry limit, and retry flag.
- Log RSSI/SNR, duplicates, malformed packets, and sequence gaps.
- Range-test and select compliant frequency/SF/power settings.

Exit: reliable delivery over the real lake-to-house path with diagnosable loss.

The RAK4631 is now the preferred lake-node controller because it omits the
unneeded display and offers a modular, low-power platform. The existing Heltec
node implementation remains useful as a reference and fallback while sensor
acquisition is ported to the RAK. GPS integration is explicitly deferred until
the core sensor, radio, and power path are validated; a fixed installation does
not need a continuous GPS energy cost.

## Milestone 4 — MQTT gateway

- Add externalized Wi-Fi/MQTT configuration and reconnection state machines.
- Decode and publish water temperature in state and diagnostic payloads.
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
- Validate temperature at ice and room-temperature points; retain ROM address
  and measured offset.
- Install fixed probe/stilling well and establish a manual benchmark.
- Monitor drift, condensation/venting, packet loss, and battery voltage.

Exit: 1–2 cm initial fixed-depth repeatability and acceptable manual-gauge
agreement for trend monitoring.

## Deferred hardening

Surge protection, winter-safe solar charging, enclosure environmental testing,
remote configuration, and deciding whether LoRaWAN/Meshtastic integration has a
long-term advantage all follow the functional field pilot.
