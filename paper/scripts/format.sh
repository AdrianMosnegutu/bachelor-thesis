#!/usr/bin/env bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/lib/common.sh"
ensure_not_sourced "${BASH_SOURCE[0]}" "Run: bash scripts/format.sh" || return 1

set -euo pipefail

PAPER_DIR="$(project_root_from_script "${BASH_SOURCE[0]}")"
require_cmds bibtex-tidy tex-fmt

echo "Formatting LaTeX files under ${PAPER_DIR}"
mapfile -d '' tex_files < <(collect_files "${PAPER_DIR}" -name "*.tex")
if [ "${#tex_files[@]}" -gt 0 ]; then
  tex-fmt --wraplen 120 "${tex_files[@]}"
else
  echo "No .tex files found under ${PAPER_DIR}"
fi

echo "Formatting bibliography: references.bib"
bibtex-tidy -m --blank-lines 1 --strip-enclosing-braces --sort --merge "${PAPER_DIR}/references.bib"
