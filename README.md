# LoRa Lake-Level Monitor

Battery-powered lake-level monitoring with a RAKwireless WisBlock RAK4631 lake
node and a Heltec WiFi LoRa 32 V3/V3.2 gateway, a
4–20 mA submersible pressure transmitter, a waterproof DS18B20 water-temperature
probe, and a house-side LoRa/MQTT gateway.

The lake node wakes every 15 minutes, powers the sensor and ADS1115 through the
Heltec's switched Vext rail, filters the pressure reading, transmits a compact
binary LoRa packet, and returns to deep sleep. The gateway phase will decode the
packet and publish retained JSON state to MQTT.

## Status

Early bench firmware. The initial node implementation includes:

- A verified bidirectional 915 MHz RAK4631-to-Heltec bench link with sequence
  numbers, ACKs, RSSI, and SNR diagnostics.
- Heltec gateway firmware that validates packets, acknowledges them, joins
  Wi-Fi, and publishes retained JSON state and availability over MQTT.
- RAK mock-node firmware using a potentiometer for depth and a DHT22 for live
  temperature while the production sensors are unavailable.

- Vext power sequencing on GPIO36.
- ADS1115 acquisition on GPIO4/GPIO5 with trimmed-mean filtering.
- 4–20 mA conversion, calibration, and fault flags.
- DS18B20 water-temperature acquisition during the pressure warm-up window.
- V3.2 battery-divider sampling behind a compile-time setting.
- Explicit, little-endian version-1 packet serialization.
- SX1262 transmission through RadioLib and timer deep sleep.
- Native host tests for protocol and measurement logic.

Hardware validation is required before connecting the pressure sensor. In
particular, adjust and load-test the boost converter at 24.0 V first, verify the
sensor polarity, attach a 915 MHz antenna before transmitting, and confirm the
Heltec board revision.

## Hardware baseline

| Function | Part / setting |
|---|---|
| Lake controller/radio | RAKwireless WisBlock RAK4631, US915 version |
| Bench gateway | Heltec WiFi LoRa 32 V3/V3.2, US 902–928 MHz version |
| ADC | ADS1115 at `0x48`, ±4.096 V, 128 SPS |
| Current shunt | 100 Ω, 0.1% |
| Sensor supply | 3.3 V Vext to adjustable 24 V boost converter |
| Sensor | Two-wire, loop-powered 4–20 mA, nominal 0–5 m |
| Temperature | Externally powered waterproof DS18B20 probe on GPIO6 |
| Battery | One protected 1S 18650 |
| Initial interval | 15 minutes |

See [docs/wiring.md](docs/wiring.md) before assembling hardware and
[docs/design.md](docs/design.md) for system decisions and acceptance criteria.
The complete parts list and current sourcing notes are in [docs/bom.md](docs/bom.md).
The temporary DHT22 and potentiometer setup is in
[docs/mock-sensors.md](docs/mock-sensors.md).

## Build and test

Install [PlatformIO](https://platformio.org/), then:

```sh
pio run -e lake_node
./scripts/run_host_tests.sh
```

With only a Heltec V3 connected, the non-transmitting diagnostic image can be
built, flashed, and monitored with:

```sh
pio run -e heltec_diagnostic -t upload --upload-port /dev/cu.usbserial-0001
pio device monitor --port /dev/cu.usbserial-0001 --baud 115200
```

It tests identity, Vext switching, SX1262 initialization/sleep, and an optional
10-second timer deep-sleep cycle. It never sends an RF packet.

With antennas attached to both boards, the bidirectional link test can be
built and flashed with:

```sh
pio run -e heltec_link_test -t upload --upload-port /dev/cu.usbserial-0001
pio run -e rak_link_test -t upload --upload-port /dev/cu.usbmodem11421201
pio device monitor --port /dev/cu.usbserial-0001 --baud 115200
pio device monitor --port /dev/cu.usbmodem11421201 --baud 115200
```

Serial device names may change after reconnecting. The RAK transmits numbered
test frames every three seconds; the Heltec receives and acknowledges them.
See [docs/link-test.md](docs/link-test.md) for settings and the validated result.

For the MQTT gateway, copy the example configuration and fill in local values:

```sh
cp firmware/gateway/include/gateway_config.example.h \
  firmware/gateway/include/gateway_config.h
pio run -e gateway
pio run -e gateway -t upload --upload-port /dev/cu.usbserial-0001
```

`gateway_config.h` is ignored by Git. Valid node packets are published as
retained JSON to `lake-monitor/node/<node-id>/state`; gateway status is retained
at `lake-monitor/gateway/<gateway-id>/availability`.

The firmware defaults are intentionally explicit in
[`firmware/node/include/config.example.h`](firmware/node/include/config.example.h).
Copy it to `lake_node_config.h` only when local overrides are needed;
`lake_node_config.h` is ignored by Git. The source builds with the example
defaults when no override exists.

## Repository map

```text
common/                  Shared packet and measurement code
firmware/node/           Battery-node firmware
firmware/gateway/        Gateway placeholder for the next firmware milestone
firmware/rak_link_test/  RAK4631 LoRa transmitter/ACK test
firmware/heltec_link_test/ Heltec LoRa receiver/ACK test
firmware/rak_mock_node/   Potentiometer/DHT22 sensor simulator
docs/                    Design, wiring, calibration, deployment, and plan
scripts/                 Host test and later calibration/power tools
test/                    Native C++ tests
```

## Roadmap

1. Bench-validate Vext, boost startup, ADS1115 readings, and warm-up time.
2. Complete and range-test the LoRa protocol with ACK/retry and a gateway.
3. Publish validated readings to MQTT with Wi-Fi recovery and diagnostics.
4. Measure full-cycle energy, calibrate the sensor, and deploy outdoors.

The detailed, issue-oriented roadmap is in [docs/plan.md](docs/plan.md).

## Safety

Use a protected cell, verify the SH1.25 battery-lead polarity with a multimeter,
never connect cells in series to the Heltec, and never connect the boost's 24 V
output to the Heltec or ADS1115. Keep the sensor vent dry and above flood level
if the transmitter is vented.
