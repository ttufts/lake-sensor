# Public Data Delivery

## Home Assistant HTTP forwarding

The Heltec gateway publishes each accepted LoRa reading to MQTT with a UTC
`timestamp_utc`. Home Assistant consumes the same MQTT packet used by its
entities and forwards it to the external HTTPS endpoint. This keeps TLS and the
API credential off the gateway while avoiding the IoT VLAN's outbound internet
restriction. The credential is held only in Home Assistant's `secrets.yaml`.

The payload fields are `timestamp_utc`, `depth_in`, `temperature_f`,
`battery_v`, `battery_pct`, `loop_ma`, `rssi_dbm`, `snr_db`, `status_flags`,
and `sample_count`.

See [`home-assistant/lake-data-forwarding.yaml`](../home-assistant/lake-data-forwarding.yaml)
for the `rest_command` and MQTT-triggered automation, and
[`home-assistant/README.md`](../home-assistant/README.md) for installation.

## Retired GitHub exporter

The public archive is stored under [`data/`](../data/README.md). Home Assistant
and InfluxDB retain full-resolution operational telemetry; GitHub receives
five-minute summaries suitable for sharing and long-term analysis.

## Schedule and recovery

The retained `lake_data_exporter` Docker Swarm stack on `valkyrie01` previously exported the
previous UTC day at 00:10 UTC and also ran once whenever its container started,
which repairs a missed export after an outage. It pulls before generating data,
commits only when files changed, and pushes through a dedicated mounted SSH
secret. Its service is intentionally scaled to zero after HA HTTP forwarding
was selected. Restore it with:

```sh
docker service scale lake_data_exporter_exporter=1
```

## Secrets

The stack expects these external Docker secrets:

- `lake_data_ha_token`: a Home Assistant long-lived access token.
- `lake_data_github_ssh_key`: an SSH key authorized to write the GitHub repo.

Secret values never belong in Git, images, compose environment variables, or
logs. Rotate a secret by creating a versioned replacement, updating the stack,
then deleting the unreferenced old secret.

## Manual export

Set `HA_TOKEN` or `HA_TOKEN_FILE`, then run:

```sh
python3 scripts/export_public_data.py \
  --start 2026-08-23T01:40:00Z \
  --end 2026-08-24T00:00:00Z
```

The exporter uses packet sequence timestamps as the sample clock and carries
forward the most recently recorded value for each measurement. It requests one
hour of lookback history so a value that did not change at midnight is still
available for the first bucket.
