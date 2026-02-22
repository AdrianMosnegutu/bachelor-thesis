#!/usr/bin/env bash
if [[ "${BASH_SOURCE[0]}" != "$0" ]]; then
  echo "Do not source this script. Run: bash paper/scripts/lint.sh" >&2
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

require_cmd chktex
require_cmd codespell

echo "Running codespell on LaTeX/BibTeX/style files under ${PAPER_DIR}"
find "${PAPER_DIR}" -type f \( -name "*.tex" -o -name "*.bib" -o -name "*.sty" \) -print0 \
  | xargs -0 codespell

echo "Running chktex on LaTeX/style files under ${PAPER_DIR}"
find "${PAPER_DIR}" -type f \( -name "*.tex" -o -name "*.sty" \) -print0 \
  | xargs -0 chktex -q
