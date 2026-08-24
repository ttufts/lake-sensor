# Home Assistant

The Heltec gateway publishes retained MQTT state and Home Assistant Discovery.
No manual MQTT sensor YAML is required.

## Entities

- `sensor.lake_monitor_node_1_lake_level`
- `sensor.lake_monitor_node_1_lake_level_inches`
- `sensor.lake_monitor_node_1_temperature`
- `sensor.lake_monitor_node_1_battery_voltage`
- `sensor.lake_monitor_node_1_battery`
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

## Forward readings to the public API

The gateway includes `timestamp_utc` in every MQTT state packet. Home Assistant
can forward that original packet time to the public API independently of its
MQTT entities. This keeps the API credential off the field hardware and avoids
requiring outbound internet access from the IoT VLAN.

1. Add this line to `/config/secrets.yaml`, substituting the real key:

   ```yaml
   lake_api_authorization: "Bearer YOUR_API_KEY"
   ```

2. Merge both top-level sections from `lake-data-forwarding.yaml` into
   `/config/configuration.yaml`. If `rest_command:` or `automation:` already
   exists, merge their children instead of adding duplicate top-level keys.
3. Run **Developer tools → YAML → Check configuration**.
4. Restart Home Assistant. Confirm that `rest_command.publish_lake_reading`
   appears in Developer tools and that the automation's trace reports success.

The MQTT trigger forwards one complete reading per radio packet instead of
firing separately as each discovered entity changes. The API upserts duplicate
timestamps, making a replay of the retained MQTT packet safe.
