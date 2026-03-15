#!/usr/bin/env bash

set -euo pipefail

export PATH="$HOME/.local/bin:$PATH"

report="$(mktemp codespell-report-XXXX.txt)"
trap 'rm -f "${report}"' EXIT

mapfile -d '' files < <(find "${PAPER_DIR}" -type f \( -name "*.tex" -o -name "*.bib" -o -name "*.sty" \) -print0 | sort -z)
if [ "${#files[@]}" -eq 0 ]; then
  echo "No files found for spellchecking under ${PAPER_DIR}." >> "${GITHUB_STEP_SUMMARY}"
  exit 0
fi

set +e
codespell "${files[@]}" | tee "${report}"
status=${PIPESTATUS[0]}
set -e

if [ "${status}" -eq 0 ]; then
  exit 0
fi

REPORT_PATH="${report}" python3 - <<'PY'
import os
import re
from pathlib import Path

report_path = Path(os.environ["REPORT_PATH"])
repo_url = f"{os.environ.get('GITHUB_SERVER_URL', 'https://github.com')}/{os.environ['REPORT_REPO']}/blob/{os.environ['REPORT_SHA']}"
lines = report_path.read_text().strip().splitlines()

issues = []
for line in lines:
    match = re.match(r"([^:]+):(\d+):\s*(.*)", line)
    if not match:
        continue
    file_path, lineno, rest = match.groups()
    if "==>" in rest:
        wrong, right = (part.strip() for part in rest.split("==>", 1))
    else:
        wrong, right = rest.strip(), ""
    issues.append((file_path, lineno, wrong, right))

if not issues:
    raise SystemExit(0)

with open(os.environ["GITHUB_STEP_SUMMARY"], "a", encoding="utf-8") as summary:
    summary.write(f"## Spellcheck issues found: {len(issues)}\n\n")
    summary.write("| File | Misspelling | Suggestion |\n")
    summary.write("| --- | --- | --- |\n")

    for file_path, lineno, wrong, right in issues:
        link = f"{repo_url}/{file_path}#L{lineno}"
        wrong = wrong.replace("|", "\\|")
        right = (right or "-").replace("|", "\\|")
        summary.write(f"| [{file_path}:{lineno}]({link}) | {wrong} | {right} |\n")
PY

exit 1
