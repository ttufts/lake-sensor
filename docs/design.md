# System Design

## Objective

Measure water head over a fixed submerged 4–20 mA transmitter, send a compact
raw-LoRa packet from a battery node, and publish it to MQTT at a mains-powered
house gateway. The system targets lake trends and seasonal response, not wave
shape or survey-grade elevation without field calibration.

## Architecture

```text
protected 1S 18650
  -> Heltec WiFi LoRa 32 V3/V3.2
     -> switched 3.3 V Vext -> ADS1115
                            -> 3.3-to-24 V boost -> 4–20 mA sensor
     -> SX1262 raw LoRa ~915 MHz
        -> mains-powered Heltec gateway -> Wi-Fi -> MQTT
```

The lake node does not use Wi-Fi. It wakes, reads the battery, enables Vext,
waits for the sensor, samples the ADC, disables Vext, transmits, then sleeps.
Turning the boost off before transmitting separates two noisy/high-current
phases and is a firmware invariant.

## Analog model

A 100 Ω, 0.1% shunt produces 0.400–2.000 V for 4–20 mA. With a nominal 0–5 m
sensor:

```text
loop_mA = 1000 * sense_V / shunt_ohms
depth_m = slope * sense_V + offset
nominal slope = 3.125 m/V; nominal offset = -1.25 m
```

The ADS1115 uses single-ended A0, ±4.096 V gain, and 128 SPS. The initial
filter sorts 32 readings, removes four from each end, and averages the rest.
Raw voltage and current remain telemetry fields so calibration can be changed
without losing diagnostic information.

## Protocol version 1

All fields are encoded explicitly in little-endian order; compiler struct
layout is never sent over the air.

| Offset | Size | Field | Unit |
|---:|---:|---|---|
| 0 | 2 | Magic `0x4C4B` | On wire: bytes `4B 4C` (`LK`) |
| 2 | 1 | Version | `1` |
| 3 | 1 | Node ID | Deployment-assigned |
| 4 | 4 | Sequence | Wake-cycle counter |
| 8 | 4 | Depth | Signed millimetres |
| 12 | 2 | Sense voltage | Millivolts |
| 14 | 2 | Loop current | Microamps |
| 16 | 2 | Battery voltage | Millivolts |
| 18 | 2 | Flags | Status mask |

The packet is 20 bytes. The physical-layer CRC remains enabled. Status bits are:

| Bit | Meaning |
|---:|---|
| 0 | Sensor undercurrent (<3.6 mA) |
| 1 | Sensor overcurrent (>21.0 mA) |
| 2 | ADC saturated/invalid |
| 3 | Battery low |
| 4 | ADS1115 missing |
| 5 | Sensor unsettled |
| 6 | Calibration invalid |
| 7 | Retry packet |

## Initial radio settings

- 915.0 MHz placeholder channel; select a compliant final channel after a site
  survey and avoid unnecessary overlap with local Meshtastic use.
- 125 kHz bandwidth, SF9, coding rate 4/5, CRC enabled.
- Private sync word `0x12` and 14 dBm initial transmit power.
- Acknowledgements and up to two retries are planned after the receiver exists.

## Power assumptions

At a 15-minute interval, a conservative planning range is 0.6–1.0 mA average,
or roughly 3–6 months from about 2400 mAh usable capacity. This is not an
acceptance result. Full-cycle energy, boost efficiency, sensor warm-up, and
complete-board sleep current must be measured.

## Known unknowns

- Exact Heltec revision and V3.2 battery-divider behavior.
- Sensor lead polarity, venting, actual warm-up, and repeatability.
- Whether the selected boost starts from Vext at 20 mA loop output.
- Actual sleep current and best radio settings for the path.
- Winter-safe charging strategy.

## Prototype acceptance criteria

- 24.0 V boost output within about ±0.5 V across 4–20 mA load.
- No Heltec reset during boost startup.
- ADS1115 agrees with a calibrated multimeter within 0.5%.
- Fixed-depth repeatability is initially within 1–2 cm after filtering.
- Open/disconnected sensor conditions produce clear faults.
- Vext is off in deep sleep; complete-node sleep current is below 100 µA.
- Reliable actual-path delivery with sequence gaps observable.
- MQTT retains the latest valid reading and does not replace it with invalid data.

