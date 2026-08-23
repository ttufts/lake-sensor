# Public Data Export

The public archive is stored under [`data/`](../data/README.md). Home Assistant
and InfluxDB retain full-resolution operational telemetry; GitHub receives
five-minute summaries suitable for sharing and long-term analysis.

## Schedule and recovery

The `lake_data_exporter` Docker Swarm stack runs on `valkyrie01`. It exports the
previous UTC day at 00:10 UTC and also runs once whenever its container starts,
which repairs a missed export after an outage. It pulls before generating data,
commits only when files changed, and pushes through a dedicated mounted SSH
secret.

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
