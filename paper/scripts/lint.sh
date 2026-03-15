#!/usr/bin/env bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/lib/common.sh"
ensure_not_sourced "${BASH_SOURCE[0]}" "Run: bash scripts/lint.sh"  || return 1

set -euo pipefail

PAPER_DIR="$(project_root_from_script "${BASH_SOURCE[0]}")"
require_cmds chktex codespell tex-fmt

echo "Running tex-fmt check with 120-character wrapping on LaTeX files under ${PAPER_DIR}"
mapfile -d '' tex_files < <(collect_files "${PAPER_DIR}" -name "*.tex")
if [ "${#tex_files[@]}" -gt 0 ]; then
  tex-fmt --check --wraplen 120 "${tex_files[@]}"
else
  echo "No .tex files found under ${PAPER_DIR}"
fi

echo "Running codespell on LaTeX/BibTeX/style files under ${PAPER_DIR}"
collect_files "${PAPER_DIR}" -name "*.tex" -o -name "*.bib" -o -name "*.sty" \
  | xargs -0 codespell

echo "Running chktex on LaTeX/style files under ${PAPER_DIR}"
collect_files "${PAPER_DIR}" -name "*.tex" -o -name "*.sty" \
  | xargs -0 chktex -q -l "${PAPER_DIR}/.chktexrc"
