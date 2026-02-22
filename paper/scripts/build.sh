#!/usr/bin/env bash
if [[ "${BASH_SOURCE[0]}" != "$0" ]]; then
  echo "Do not source this script. Run: bash paper/scripts/build.sh" >&2
  return 1
fi

set -euo pipefail

# Resolve the paper directory relative to this script so it works from any cwd.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PAPER_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

require_cmd() {
  if ! command -v "$1" >/dev/null 2>&1; then
    echo "Missing required command: $1" >&2
    exit 1
  fi
}

require_cmd latexmk

cd "${PAPER_DIR}"

# Build the PDF article
latexmk -pdf main.tex

# Clean build artefacts, except the PDF article
latexmk -c
