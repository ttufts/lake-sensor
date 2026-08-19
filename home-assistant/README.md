# Home Assistant

The Heltec gateway publishes retained MQTT state and Home Assistant Discovery.
No manual MQTT sensor YAML is required.

## Entities

- `sensor.lake_monitor_node_1_lake_level`
- `sensor.lake_monitor_node_1_temperature`
- `sensor.lake_monitor_node_1_battery_voltage`
- `sensor.lake_monitor_node_1_loop_current`
- `sensor.lake_monitor_node_1_lora_rssi`
- `sensor.lake_monitor_node_1_lora_snr`
- `sensor.lake_monitor_node_1_status_flags`
- `sensor.lake_monitor_node_1_sequence`

The production dashboard is preserved in `lake-monitor-dashboard.yaml`. It uses
only built-in cards: current gauges, separate 72-hour raw graphs, separate
30-day min/mean/max statistics graphs, and a diagnostic entity list.

Home Assistant may display length and temperature in the instance's preferred
units (for example feet and °F) even though MQTT carries meters and °C.

`configuration-snippets.yaml` records the InfluxDB allowlist needed for Grafana.
Merge the snippet into the existing `influxdb:` block; do not create a second
top-level block. Run `ha core check` before restarting Home Assistant Core.
