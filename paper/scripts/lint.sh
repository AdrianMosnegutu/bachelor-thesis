#!/usr/bin/env bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "${SCRIPT_DIR}/lib/common.sh"
ensure_not_sourced "${BASH_SOURCE[0]}" "Run: bash scripts/lint.sh"  || return 1

set -euo pipefail

PAPER_DIR="$(project_root_from_script "${BASH_SOURCE[0]}")"
require_cmds chktex codespell tex-fmt
chktex_args=(-q)

if [ -f "${PAPER_DIR}/.chktexrc" ]; then
  chktex_args+=(-l "${PAPER_DIR}/.chktexrc")
fi

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

echo "Running chktex on root document and style files under ${PAPER_DIR}"

# Lint the root document so chktex resolves nested \\input trees relative to paper.tex.
chktex "${chktex_args[@]}" -I1 "${PAPER_DIR}/paper.tex"

style_file="${PAPER_DIR}/style.sty"
if [ -f "${style_file}" ]; then
  chktex "${chktex_args[@]}" "${style_file}"
fi
