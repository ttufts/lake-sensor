# LoRa Lake-Level Monitor

A field-tested lake monitor built around a RAKwireless RAK4631/RAK19003 sensor
node and a Heltec WiFi LoRa 32 V3 house gateway. The lake node measures water
depth, water temperature, battery state, and radio diagnostics. The gateway
receives raw 915 MHz LoRa packets and publishes retained MQTT state plus Home
Assistant Discovery records over Wi-Fi.

## Current status

The complete system is operating in the lake. A 2026-08-22 field check measured
about 16 inches with a ruler while the node reported 15.47 inches; reference
water temperature was about 81 °F while the node reported 81.5 °F. A full
USB-disconnected battery endurance run began at 21:42 EDT using the current
1100 mAh protected Li-ion pack.

Current limitations:

- The pressure loop and 24 V boost remain continuously powered, so this is an
  endurance-test build rather than the final low-power design.
- The pressure transmitter's atmospheric reference must remain open and dry.
  A membrane enclosure vent is planned before fully sealing the box.
- Calibration constants are specific to the installed pressure and temperature
  probes and must be rechecked after component replacement.

## Hardware

| Function | Installed hardware |
|---|---|
| Lake controller/radio | RAK4631 US915 on RAK19003 base |
| Gateway | Heltec WiFi LoRa 32 V3/V3.2, US915 |
| Pressure ADC | ADS1115 at `0x48`, A0 input |
| Pressure loop | 0–5 m two-wire 4–20 mA transmitter, 100 Ω shunt |
| Loop supply | MT3608 adjusted to 24.0 V, inline with three-wire pressure harness |
| Temperature | Waterproof DS18B20-compatible probe with 5.1 kΩ pull-up |
| Battery | Protected 1100 mAh 1S Li-ion on the RAK19003 battery port |
| Telemetry | Raw LoRa → Heltec → Wi-Fi/MQTT → Home Assistant/InfluxDB |

Start with the [complete hardware build walkthrough](docs/hardware-build.md),
then consult the [exact wiring/net map](docs/wiring.md), [BOM](docs/bom.md), and
[calibration record](docs/calibration.md). The perfboard diagram in the build
guide reflects the current soldered layout; its legacy `DHT22` and `POT` labels
are mapped to the production DS18B20 and pressure harness in the accompanying
text.

## Firmware

The production RAK image currently uses the historical PlatformIO environment
name `rak_mock_node`; it now reads the real ADS1115 pressure channel,
DS18B20-compatible probe, and battery ADC. The house gateway uses `gateway`.

```sh
pio run -e rak_mock_node
pio run -e gateway
./scripts/run_host_tests.sh
```

Flash with the USB device paths shown by `pio device list`:

```sh
pio run -e rak_mock_node -t upload --upload-port /dev/cu.usbmodemXXXX
pio run -e gateway -t upload --upload-port /dev/cu.usbserial-XXXX
```

The RAK may require a reset or double-reset to expose its bootloader. Always
attach both 915 MHz antennas before operating the radios. See
[build and flashing instructions](docs/build-and-flash.md) for recovery,
configuration, and serial-monitor details.

## MQTT and Home Assistant

The gateway publishes node state to:

```text
lake-monitor/node/1/state
```

Its JSON includes calibrated depth in metres and inches, temperature, battery
voltage and percentage, pressure-loop current, LoRa RSSI/SNR, flags, and packet
sequence. It also publishes retained Home Assistant MQTT Discovery messages.
Reproducible dashboard and InfluxDB configuration are under
[`home-assistant/`](home-assistant/).

## Public data

Five-minute production summaries are available under [`data/`](data/README.md).
The first recoverable record follows the intentional Home Assistant test-data
purge at `2026-08-23T01:48:37Z`. A Docker Swarm exporter publishes each completed
UTC day to GitHub; see [public-data operations](docs/public-data.md).

Direct links after the GitHub mirror is published:

- [Latest exported reading](https://github.com/ttufts/lake-sensor/blob/main/data/latest.json)
- [August 23, 2026 data](https://github.com/ttufts/lake-sensor/blob/main/data/2026/08/2026-08-23.csv)

## Repository map

```text
common/                         Shared packet and measurement code
firmware/rak_mock_node/         Production RAK4631 lake-node firmware
firmware/gateway/               Heltec Wi-Fi/MQTT gateway firmware
firmware/*_test/                Hardware and radio diagnostic images
docs/                           Build, wiring, calibration, and operations docs
home-assistant/                 Dashboard and recorder/InfluxDB configuration
data/                           Public five-minute telemetry archive
deploy/public-data-exporter/    Docker Swarm exporter
scripts/                        Exporter and host-test tools
test/                           Native protocol/measurement tests
```

## Safety

Verify every battery JST's polarity with a multimeter before connection. Use a
protected 1S cell, attach antennas before transmitting, and never connect the
boost converter's 24 V output to the RAK, ADS1115, or temperature probe. Keep
every conductor splice separately insulated. Keep the transmitter's atmospheric
reference dry, unpinched, and connected to outside pressure through a proper
hydrophobic enclosure vent.
