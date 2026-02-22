#!/usr/bin/env bash
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

require_cmd bibtex-tidy
require_cmd tex-fmt

echo "Formatting LaTeX files under ${PAPER_DIR}"
tex-fmt --recursive -l 100

echo "Formatting bibliography: references.bib"
bibtex-tidy -m --blank-lines 1 --strip-enclosing-braces --sort --merge "${PAPER_DIR}/references.bib"
