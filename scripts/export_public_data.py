#!/usr/bin/env python3
"""Export Lake Monitor Home Assistant history as compact public CSV data."""

from __future__ import annotations

import argparse
import bisect
import csv
import json
import os
import statistics
import tempfile
import urllib.parse
import urllib.request
from collections import defaultdict
from datetime import datetime, timedelta, timezone
from pathlib import Path


ENTITIES = {
    "sequence": "sensor.lake_monitor_node_1_sequence",
    "depth_in": "sensor.lake_monitor_node_1_lake_level_inches",
    "temperature_f": "sensor.lake_monitor_node_1_temperature",
    "battery_v": "sensor.lake_monitor_node_1_battery_voltage",
    "battery_pct": "sensor.lake_monitor_node_1_battery",
    "loop_ma": "sensor.lake_monitor_node_1_loop_current",
    "rssi_dbm": "sensor.lake_monitor_node_1_lora_rssi",
    "snr_db": "sensor.lake_monitor_node_1_lora_snr",
    "status_flags": "sensor.lake_monitor_node_1_status_flags",
}

FIELDS = [
    "timestamp_utc",
    "depth_in",
    "temperature_f",
    "battery_v",
    "battery_pct",
    "loop_ma",
    "rssi_dbm",
    "snr_db",
    "status_flags",
    "sample_count",
]


def parse_time(value: str) -> datetime:
    parsed = datetime.fromisoformat(value.replace("Z", "+00:00"))
    if parsed.tzinfo is None:
        raise ValueError("timestamps must include a UTC offset")
    return parsed.astimezone(timezone.utc)


def iso_z(value: datetime) -> str:
    return value.astimezone(timezone.utc).replace(microsecond=0).isoformat().replace("+00:00", "Z")


def read_token() -> str:
    token_file = os.getenv("HA_TOKEN_FILE")
    if token_file:
        return Path(token_file).read_text(encoding="utf-8").strip()
    token = os.getenv("HA_TOKEN")
    if not token:
        raise RuntimeError("set HA_TOKEN or HA_TOKEN_FILE")
    return token


def fetch_history(base_url: str, token: str, start: datetime, end: datetime) -> dict[str, list[tuple[datetime, str]]]:
    # Include a lookback so unchanged values can be carried into the first bucket.
    query_start = start - timedelta(hours=1)
    params = urllib.parse.urlencode(
        {
            "filter_entity_id": ",".join(ENTITIES.values()),
            "end_time": iso_z(end),
            "minimal_response": "",
            "no_attributes": "",
        }
    )
    url = f"{base_url.rstrip('/')}/api/history/period/{iso_z(query_start)}?{params}"
    request = urllib.request.Request(
        url,
        headers={"Authorization": f"Bearer {token}", "Accept": "application/json"},
    )
    with urllib.request.urlopen(request, timeout=60) as response:
        payload = json.load(response)

    reverse = {entity: name for name, entity in ENTITIES.items()}
    result: dict[str, list[tuple[datetime, str]]] = defaultdict(list)
    for series in payload:
        entity_id = None
        for item in series:
            entity_id = item.get("entity_id", entity_id)
            name = reverse.get(entity_id)
            changed = item.get("last_changed") or item.get("last_updated")
            state = item.get("state")
            if name and changed and state not in (None, "unknown", "unavailable"):
                result[name].append((parse_time(changed), state))
    for values in result.values():
        values.sort(key=lambda value: value[0])
    return result


def value_at(series: list[tuple[datetime, str]], timestamp: datetime) -> float | None:
    if not series:
        return None
    times = [item[0] for item in series]
    index = bisect.bisect_right(times, timestamp) - 1
    if index < 0:
        return None
    try:
        return float(series[index][1])
    except ValueError:
        return None


def rounded(value: float | None, digits: int) -> str:
    return "" if value is None else f"{value:.{digits}f}"


def aggregate(history: dict[str, list[tuple[datetime, str]]], start: datetime, end: datetime) -> list[dict[str, str | int]]:
    samples: dict[datetime, list[dict[str, float | None]]] = defaultdict(list)
    for timestamp, _sequence in history.get("sequence", []):
        if not start <= timestamp < end:
            continue
        bucket = timestamp.replace(minute=(timestamp.minute // 5) * 5, second=0, microsecond=0)
        samples[bucket].append(
            {name: value_at(history.get(name, []), timestamp) for name in ENTITIES if name != "sequence"}
        )

    rows: list[dict[str, str | int]] = []
    for bucket in sorted(samples):
        packet_rows = samples[bucket]

        def median(name: str) -> float | None:
            values = [row[name] for row in packet_rows if row[name] is not None]
            return statistics.median(values) if values else None

        def last(name: str) -> float | None:
            values = [row[name] for row in packet_rows if row[name] is not None]
            return values[-1] if values else None

        flags = last("status_flags")
        rows.append(
            {
                "timestamp_utc": iso_z(bucket),
                "depth_in": rounded(median("depth_in"), 2),
                "temperature_f": rounded(median("temperature_f"), 2),
                "battery_v": rounded(last("battery_v"), 3),
                "battery_pct": rounded(last("battery_pct"), 0),
                "loop_ma": rounded(median("loop_ma"), 3),
                "rssi_dbm": rounded(median("rssi_dbm"), 1),
                "snr_db": rounded(median("snr_db"), 2),
                "status_flags": "" if flags is None else str(int(flags)),
                "sample_count": len(packet_rows),
            }
        )
    return rows


def atomic_write(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile("w", encoding="utf-8", dir=path.parent, delete=False) as handle:
        handle.write(content)
        temp_path = Path(handle.name)
    temp_path.replace(path)


def write_rows(root: Path, rows: list[dict[str, str | int]]) -> list[Path]:
    grouped: dict[str, list[dict[str, str | int]]] = defaultdict(list)
    for row in rows:
        grouped[str(row["timestamp_utc"])[:10]].append(row)

    written: list[Path] = []
    for day, day_rows in grouped.items():
        year, month, _ = day.split("-")
        path = root / year / month / f"{day}.csv"
        from io import StringIO

        buffer = StringIO()
        writer = csv.DictWriter(buffer, fieldnames=FIELDS, lineterminator="\n")
        writer.writeheader()
        writer.writerows(day_rows)
        atomic_write(path, buffer.getvalue())
        written.append(path)

    if rows:
        atomic_write(root / "latest.json", json.dumps(rows[-1], indent=2) + "\n")
        written.append(root / "latest.json")
    return written


def default_window() -> tuple[datetime, datetime]:
    today = datetime.now(timezone.utc).replace(hour=0, minute=0, second=0, microsecond=0)
    return today - timedelta(days=1), today


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--start", help="inclusive ISO-8601 time; defaults to previous UTC day")
    parser.add_argument("--end", help="exclusive ISO-8601 time; defaults to current UTC day start")
    parser.add_argument("--output-root", default="data")
    parser.add_argument("--ha-url", default=os.getenv("HA_URL", "https://smart.tufts.network"))
    args = parser.parse_args()

    if bool(args.start) != bool(args.end):
        parser.error("--start and --end must be supplied together")
    start, end = (parse_time(args.start), parse_time(args.end)) if args.start else default_window()
    if end <= start:
        parser.error("--end must be later than --start")

    history = fetch_history(args.ha_url, read_token(), start, end)
    rows = aggregate(history, start, end)
    if not rows:
        raise RuntimeError(f"no sequence samples found from {iso_z(start)} to {iso_z(end)}")
    paths = write_rows(Path(args.output_root), rows)
    print(f"exported {len(rows)} five-minute rows from {iso_z(start)} to {iso_z(end)}")
    for path in paths:
        print(path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
