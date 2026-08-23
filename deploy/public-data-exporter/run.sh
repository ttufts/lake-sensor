#!/bin/sh
set -eu

repo_dir=${REPO_DIR:-/state/repo}
repo_url=${REPO_URL:-git@github.com:ttufts/lake-sensor.git}
run_hour_utc=${RUN_HOUR_UTC:-0}
run_minute_utc=${RUN_MINUTE_UTC:-10}

export HA_TOKEN_FILE=${HA_TOKEN_FILE:-/run/secrets/lake_data_ha_token}
export GIT_SSH_COMMAND="ssh -i /run/secrets/lake_data_github_ssh_key -o IdentitiesOnly=yes -o StrictHostKeyChecking=yes"

mkdir -p "$(dirname "$repo_dir")"
if [ ! -d "$repo_dir/.git" ]; then
  git clone "$repo_url" "$repo_dir"
fi

cd "$repo_dir"
git config user.name "Lake Data Exporter"
git config user.email "lake-data-exporter@tufts.network"

export_once() {
  git pull --rebase
  /usr/local/bin/export_public_data.py --output-root data
  git add data
  if ! git diff --cached --quiet; then
    git commit -m "data: publish completed UTC day"
    git push origin main
  fi
}

# Run once at startup to repair a missed prior-day export.
export_once

while true; do
  delay=$(python3 - "$run_hour_utc" "$run_minute_utc" <<'PY'
from datetime import datetime, timedelta, timezone
import sys
now = datetime.now(timezone.utc)
target = now.replace(hour=int(sys.argv[1]), minute=int(sys.argv[2]), second=0, microsecond=0)
if target <= now:
    target += timedelta(days=1)
print(max(1, int((target - now).total_seconds())))
PY
)
  sleep "$delay"
  export_once
done
