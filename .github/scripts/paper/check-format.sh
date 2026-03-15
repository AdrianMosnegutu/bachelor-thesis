#!/usr/bin/env bash

set -euo pipefail

mapfile -d '' files < <(find "${PAPER_DIR}" -type f -name "*.tex" -print0 | sort -z)
if [ "${#files[@]}" -eq 0 ]; then
  echo "No .tex files found under ${PAPER_DIR}." >> "${GITHUB_STEP_SUMMARY}"
  exit 0
fi

repo_url="${GITHUB_SERVER_URL}/${REPORT_REPO}/blob/${REPORT_SHA}"
temp_dir="$(mktemp -d)"
trap 'rm -rf "${temp_dir}"' EXIT

needs_format=()
for file in "${files[@]}"; do
  rel_path="${file#"${PAPER_DIR}"/}"
  temp_file="${temp_dir}/${rel_path}"

  mkdir -p "$(dirname "${temp_file}")"
  cp "${file}" "${temp_file}"
  tex-fmt --wraplen "${WRAP_LENGTH}" "${temp_file}" >/dev/null

  if ! cmp -s "${file}" "${temp_file}"; then
    needs_format+=("${file}")
  fi
done

if [ "${#needs_format[@]}" -eq 0 ]; then
  exit 0
fi

{
  echo "## Formatting issues found: ${#needs_format[@]}"
  echo
  echo "The following files need formatting:"
  echo
  for file in "${needs_format[@]}"; do
    echo "- [${file}](${repo_url}/${file})"
  done
} >> "${GITHUB_STEP_SUMMARY}"

exit 1
