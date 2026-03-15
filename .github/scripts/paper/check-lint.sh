#!/usr/bin/env bash

set -euo pipefail

out="$(mktemp)"
trap 'rm -f "${out}"' EXIT

mapfile -d '' files < <(find "${PAPER_DIR}" -type f \( -name "*.tex" -o -name "*.sty" \) -print0 | sort -z)
if [ "${#files[@]}" -eq 0 ]; then
  echo "No LaTeX files found for linting under ${PAPER_DIR}." >> "${GITHUB_STEP_SUMMARY}"
  exit 0
fi

set +e
printf '%s\0' "${files[@]}" \
  | xargs -0 chktex -q -l "${PAPER_DIR}/.chktexrc" -f $'%k|%f|%l|%c|%m\n' 2>&1 \
  | tee "${out}"
set -e

errors="$(grep -c '^Error|' "${out}" || true)"
warnings="$(grep -c '^Warning|' "${out}" || true)"

if [ "${warnings}" -gt 0 ] || [ "${errors}" -gt 0 ]; then
  echo "## Lint summary: ${warnings} warning(s), ${errors} error(s)" >> "${GITHUB_STEP_SUMMARY}"

  REPORT_PATH="${out}" python3 - <<'PY'
import os
from pathlib import Path

report_path = Path(os.environ["REPORT_PATH"])
repo_url = f"{os.environ.get('GITHUB_SERVER_URL', 'https://github.com')}/{os.environ['REPORT_REPO']}/blob/{os.environ['REPORT_SHA']}"

rows = []
for raw_line in report_path.read_text().splitlines():
    parts = raw_line.split("|", 4)
    if len(parts) != 5:
        continue

    severity, file_path, line_no, _column, message = parts
    if message.endswith("\\n"):
        message = message[:-2]
    rows.append((severity, file_path, line_no, message))

if not rows:
    raise SystemExit(0)

with open(os.environ["GITHUB_STEP_SUMMARY"], "a", encoding="utf-8") as summary:
    summary.write("\n| File | Description | Severity |\n")
    summary.write("| --- | --- | --- |\n")

    for severity, file_path, line_no, message in rows:
        link = f"{repo_url}/{file_path}#L{line_no}"
        message = message.replace("|", "\\|")
        summary.write(f"| [{file_path}:{line_no}]({link}) | {message} | {severity.lower()} |\n")
PY
fi

if [ "${warnings}" -gt 0 ] || [ "${errors}" -gt 0 ]; then
  exit 1
fi
