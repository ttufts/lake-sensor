# RAK4631–Heltec LoRa Link Test

## Board roles

- RAKwireless WisBlock RAK4631: lake-node candidate and test-frame transmitter.
- Heltec WiFi LoRa 32 V3: gateway candidate, receiver, and ACK transmitter.
- GPS: deferred and left disconnected for this test.

Both boards must have suitable 900/915 MHz antennas attached before either
firmware is powered or flashed.

## Radio settings

The two test images use raw LoRa at 915.0 MHz, 125 kHz bandwidth, SF9, coding
rate 4/5, sync word `0x12`, 14 dBm transmit power, and an eight-symbol preamble.
The seven-byte bench frame contains `LT`, a message type, and a little-endian
32-bit sequence number. Type 1 is a test transmission and type 2 is its ACK.

## Build, flash, and monitor

```sh
pio run -e heltec_link_test
pio run -e rak_link_test
pio run -e heltec_link_test -t upload --upload-port /dev/cu.usbserial-0001
pio run -e rak_link_test -t upload --upload-port /dev/cu.usbmodem11421201
pio device monitor --port /dev/cu.usbserial-0001 --baud 115200
pio device monitor --port /dev/cu.usbmodem11421201 --baud 115200
```

Confirm actual ports with `pio device list`; macOS names can change. Flashing
the RAK replaces its application firmware. Its prior Meshtastic configuration
was backed up locally on 2026-08-18 and is deliberately excluded from Git
because it may contain credentials.

## Validated result — 2026-08-18

With both boards connected by USB on the same bench, frames 4 through 10 were
observed on both consoles. All seven were transmitted and acknowledged with
radio state 0. The Heltec reported -28 to -29 dBm RSSI and 11.00 to 11.75 dB
SNR. The RAK received ACKs at -30 to -31 dBm RSSI and 10.75 to 11.50 dB SNR.

This confirms basic SPI/radio pin assignments and bidirectional compatibility.
It is not a range, interference, duty-cycle, or packet-loss qualification.
Next, reuse the link for the version-1 sensor packet, add retry/duplicate logic,
then test at increasing separation and across the actual lake-to-house path.
