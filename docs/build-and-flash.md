# Build, Flash, and Recovery Guide

## Prerequisites

- RAKwireless WisBlock RAK4631 on the compact RAK19003 base board.
- Heltec WiFi LoRa 32 V3/V3.2, US 902–928 MHz version.
- A suitable 900/915 MHz antenna attached to **each** radio before power-up.
- USB data cables and PlatformIO Core (`pio`) on macOS or Linux.
- Python and a C++17 compiler for the native tests.

From the repository root, install or verify PlatformIO and enumerate boards:

```sh
pio --version
pio device list
```

Typical macOS ports in the validated setup are:

```text
Heltec V3: /dev/cu.usbserial-0001
RAK4631:   /dev/cu.usbmodem11421201
```

Names can change after reconnect or DFU. Always confirm them with
`pio device list` instead of assuming the examples are current.

## Configuration before building

The gateway must have a local configuration:

```sh
cp firmware/gateway/include/gateway_config.example.h \
  firmware/gateway/include/gateway_config.h
```

Edit `gateway_config.h` with the Wi-Fi SSID/password, MQTT host/port and MQTT
username/password. This file is ignored by Git and should remain private. The
validated local file uses mode `600`:

```sh
chmod 600 firmware/gateway/include/gateway_config.h
git check-ignore firmware/gateway/include/gateway_config.h
```

The current installation uses Wi-Fi SSID `Dynamite-IOT`. Its password and the
Home Assistant MQTT credentials are intentionally absent from Git.

The legacy Heltec lake-node target builds with tracked example defaults. To
override them locally:

```sh
cp firmware/node/include/config.example.h \
  firmware/node/include/lake_node_config.h
```

See [configuration.md](configuration.md) for the complete inventory.

## Test and build everything

Run the native protocol/measurement tests, then build each active target:

```sh
./scripts/run_host_tests.sh
pio run -e gateway
pio run -e rak_mock_node
pio run -e lake_node
pio run -e heltec_diagnostic
pio run -e heltec_link_test
pio run -e rak_link_test
```

The two primary bench targets are:

| Environment | Board | Purpose |
|---|---|---|
| `gateway` | Heltec V3 | LoRa receiver, ACK, Wi-Fi, MQTT and HA Discovery |
| `rak_mock_node` | RAK4631/RAK19003 | Production ADS1115 pressure, DS18B20 temperature, and battery telemetry (historical environment name) |

## Flash the working bench system

Flash the Heltec gateway:

```sh
pio run -e gateway -t upload --upload-port /dev/cu.usbserial-0001
pio device monitor --port /dev/cu.usbserial-0001 --baud 115200
```

Expected gateway messages include:

```text
mqtt connect: ok
rx node=1 seq=... rssi=... snr=... flags=0x0000
ack seq=... state=0
Home Assistant discovery published for node 1
mqtt publish lake-monitor/node/1/state: ok
```

Flash the production RAK lake node (the environment retains its historical
`rak_mock_node` name):

```sh
pio run -e rak_mock_node -t upload \
  --upload-port /dev/cu.usbmodem11421201
pio device monitor --port /dev/cu.usbmodem11421201 --baud 115200
```

Expected output includes ADS1115 readiness, pressure current/depth, temperature,
battery, transmit, and ACK values. The RAK enters its USB bootloader during upload; a temporary port
disconnect/reappearance is normal.

The validated production assembly and exact RAK19003/ADS1115 net map are in
[hardware-build.md](hardware-build.md) and [wiring.md](wiring.md). The old
[mock-sensors.md](mock-sensors.md) page is retained only as historical bench
documentation.

## Diagnostics and isolated radio test

The Heltec diagnostic image initializes hardware and exercises a short sleep
cycle without transmitting:

```sh
pio run -e heltec_diagnostic -t upload \
  --upload-port /dev/cu.usbserial-0001
```

The isolated bidirectional raw-LoRa test uses:

```sh
pio run -e heltec_link_test -t upload \
  --upload-port /dev/cu.usbserial-0001
pio run -e rak_link_test -t upload \
  --upload-port /dev/cu.usbmodem11421201
```

Both link-test images use 915.0 MHz, 125 kHz, SF9, CR 4/5, sync word `0x12`,
CRC, and 14 dBm. Do not transmit without antennas.

## RAK firmware recovery

Flashing `rak_mock_node` replaces the RAK application, including Meshtastic.
Before the first replacement, the prior Meshtastic configuration and metadata
were backed up under local `backups/`; that directory is ignored because the
configuration may contain secrets. Restoring Meshtastic requires reflashing a
compatible RAK4631 Meshtastic image and then importing the saved configuration.

## Home Assistant verification

MQTT state is retained at:

```text
lake-monitor/node/1/state
```

The gateway publishes retained discovery configs under:

```text
homeassistant/sensor/lake_node_1/+/config
```

In Home Assistant, open **Settings → Devices & services → MQTT → Devices →
Lake Monitor Node 1**. The version-controlled dashboard definition and database
allowlist are in [../home-assistant/README.md](../home-assistant/README.md).
