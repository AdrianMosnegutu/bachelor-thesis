#!/usr/bin/env bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/lib/common.sh"
ensure_not_sourced "${BASH_SOURCE[0]}" "Run: bash scripts/build.sh" || return 1

set -euo pipefail

PAPER_DIR="$(project_root_from_script "${BASH_SOURCE[0]}")"
require_cmds latexmk

cd "${PAPER_DIR}"

latexmk -pdf paper.tex
latexmk -c

if command -v open >/dev/null 2>&1; then
  open paper.pdf
fi
