#!/usr/bin/env bash

set -euo pipefail

deb_name="rclone-v${RCLONE_RELEASE_VERSION}-linux-amd64.deb"
base_url="https://downloads.rclone.org/v${RCLONE_RELEASE_VERSION}"

curl -fsSL "${base_url}/${deb_name}" -o /tmp/rclone.deb
curl -fsSL "${base_url}/SHA256SUMS" -o /tmp/SHA256SUMS

expected="$(grep " ${deb_name}$" /tmp/SHA256SUMS | awk '{print $1}')"
actual="$(sha256sum /tmp/rclone.deb | awk '{print $1}')"

if [ -z "${expected}" ]; then
  echo "::error::Could not find checksum for ${deb_name} in published SHA256SUMS"
  exit 1
fi

if [ "${expected}" != "${actual}" ]; then
  echo "::error::Checksum verification failed for ${deb_name}"
  echo "Expected: ${expected}"
  echo "Actual:   ${actual}"
  exit 1
fi

sudo dpkg -i /tmp/rclone.deb
