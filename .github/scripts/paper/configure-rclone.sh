#!/usr/bin/env bash

set -euo pipefail

mkdir -p ~/.config/rclone
cat > ~/.config/rclone/rclone.conf <<EOF
[gdrive]
type = drive
client_id = ${GDRIVE_CLIENT_ID}
client_secret = ${GDRIVE_CLIENT_SECRET}
token = ${GDRIVE_TOKEN}
scope = drive
EOF
