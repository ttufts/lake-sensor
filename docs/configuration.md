# Configuration Inventory

No live password, token, broker credential, or Meshtastic private configuration
belongs in Git.

| File | Tracked | Purpose |
|---|---:|---|
| `firmware/gateway/include/gateway_config.example.h` | Yes | Safe Wi-Fi/MQTT template with placeholders |
| `firmware/gateway/include/gateway_config_select.h` | Yes | Selects local config when present, otherwise template |
| `firmware/gateway/include/gateway_config.h` | **No** | Active Wi-Fi and MQTT credentials; ignored, mode `600` |
| `firmware/node/include/config.example.h` | Yes | Heltec node defaults, calibration, timing and LoRa settings |
| `firmware/node/include/node_config.h` | Yes | Selects a local override or tracked defaults |
| `firmware/node/include/lake_node_config.h` | **No** | Optional local node/calibration overrides |
| `firmware/**/include/secrets.h` | **No** | Reserved ignored secret file pattern |
| `home-assistant/lake-monitor-dashboard.yaml` | Yes | Native Lovelace dashboard definition, no secrets |
| `home-assistant/configuration-snippets.yaml` | Yes | InfluxDB entity allowlist and recorder notes |
| `home-assistant/lake-data-forwarding.yaml` | Yes | MQTT-to-HTTPS forwarding configuration, no secrets |
| Home Assistant `/config/secrets.yaml` | **No** | Public lake API bearer authorization value |
| `backups/` | **No** | Local RAK/Meshtastic backup that may contain secrets |

`gateway_config.example.h` defines:

- Wi-Fi SSID and password.
- MQTT hostname and port.
- MQTT username and password.
- MQTT topic prefix.
- Stable gateway/client identifier.

The active bench gateway is configured locally for `Dynamite-IOT` and the MQTT
broker already integrated with Home Assistant. Only the non-secret SSID is
documented; the password and broker credentials remain in their existing local
credential sources and the ignored `gateway_config.h`.

Home Assistant Recorder has no restrictive `recorder:` configuration, so its
default behavior records every lake entity. InfluxDB uses an explicit include
list and therefore requires `sensor.lake_monitor_node_1_*`, as preserved in the
tracked snippet. The running HA configuration was backed up before this include
was added and passed `ha core check` before restart.
