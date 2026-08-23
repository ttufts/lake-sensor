# Public Lake Data

This directory contains five-minute summaries from the Lake Monitor. Times are
UTC and files are partitioned by calendar day as `YYYY/MM/YYYY-MM-DD.csv`.

The physical 2026-08-22 deployment began at 21:40 EDT (`2026-08-23T01:40Z`).
Home Assistant's pre-production history was intentionally cleared, so the first
recoverable production record is `2026-08-23T01:48:37Z`.

| Column | Meaning |
|---|---|
| `timestamp_utc` | Start of the five-minute UTC bucket |
| `depth_in` | Median calibrated water depth in inches |
| `temperature_f` | Median calibrated water temperature in °F |
| `battery_v` | Last battery voltage in the bucket |
| `battery_pct` | Last estimated battery percentage in the bucket |
| `loop_ma` | Median pressure-loop current in mA |
| `rssi_dbm` | Median received LoRa RSSI in dBm |
| `snr_db` | Median received LoRa SNR in dB |
| `status_flags` | Last firmware status bitmask; zero is healthy |
| `sample_count` | Number of received packets represented by the row |

Blank measurements mean Home Assistant had not yet recorded an initial value
for that entity. `latest.json` contains the final row from the most recent
export. The full packet-rate history remains in the private Home Assistant and
InfluxDB instances.

The Swarm exporter finalizes and publishes the previous UTC day at 00:10 UTC.
It runs on `valkyrie01`; deployment files are under
`deploy/public-data-exporter/`.
