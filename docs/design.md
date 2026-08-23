# Current System Design

## Architecture

```text
Lake node
  protected 1S 1100 mAh Li-ion
    -> RAK19003 + RAK4631
       -> ADS1115 A0 <- 100 Ω shunt <- 4–20 mA pressure loop <- 24 V MT3608
       -> DS18B20-compatible probe on RAK19003 RXD / P0.19
       -> SX1262 raw LoRa, 915 MHz

House gateway
  USB-powered Heltec WiFi LoRa 32 V3
    -> receives/ACKs LoRa packets
    -> Dynamite-IOT Wi-Fi
    -> retained MQTT JSON
    -> Home Assistant Discovery, Recorder, InfluxDB/Grafana
```

The active RAK production firmware target retains the historical PlatformIO
name `rak_mock_node`. It now reads the real ADS1115 pressure channel,
DS18B20-compatible probe, and RAK19003 battery ADC.

## Analog channel

The 100 Ω shunt converts loop current to voltage:

```text
loop_mA = sense_V * 10
```

ADS1115 A0 is single-ended at address `0x48`, gain ±4.096 V, 128 SPS, with
eight samples averaged per packet. The working field calibration is documented
in [calibration.md](calibration.md).

## Radio and packet protocol

- 915.0 MHz, 125 kHz, SF9, CR 4/5, sync word `0x12`, CRC, 14 dBm.
- Explicit 22-byte little-endian version-1 packet.
- Node ID, sequence, depth mm, sense mV, loop µA, battery mV, temperature
  centi-°C, and status flags.
- Gateway returns an ACK and publishes retained state to
  `lake-monitor/node/1/state`.

## MQTT JSON

The current gateway publishes:

```json
{
  "node_id": 1,
  "sequence": 3681,
  "depth_m": 0.393,
  "depth_in": 15.47,
  "sense_v": 0.496,
  "loop_ma": 4.960,
  "battery_v": 4.163,
  "battery_pct": 97,
  "temperature_c": 27.50,
  "flags": 0,
  "rssi_dbm": -46.0,
  "snr_db": 10.75
}
```

Battery percentage is a voltage-based estimate and will fall rapidly from
surface charge when USB is removed. Voltage is the authoritative diagnostic.

## Status flags

| Bit | Meaning |
|---:|---|
| 0 | Pressure-loop undercurrent |
| 1 | Pressure-loop overcurrent |
| 2 | ADC invalid/saturated |
| 3 | Battery below 3.4 V |
| 4 | ADS1115 unavailable |
| 5 | Sensor unsettled |
| 6 | Calibration invalid |
| 7 | Retry packet |
| 8 | Temperature missing/invalid |

## Known limitations

- Continuous sensor/boost power limits the 1100 mAh battery to about one day.
- No node-staleness MQTT watchdog yet.
- Pressure accuracy depends on flooding the protective cap.
- The installed temperature probe needs an unusually large `-5 °C` offset.
- The enclosure and cable system are prototype-grade until long-term immersion,
  condensation, UV, and winter tests are complete.
