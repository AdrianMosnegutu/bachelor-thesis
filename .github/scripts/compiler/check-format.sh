#!/usr/bin/env bash

set -euo pipefail

compiler_dir="${COMPILER_DIR:-code/compiler}"

mapfile -d '' files < <(
  find "${compiler_dir}" \
    \( -path "${compiler_dir}/build" -o -path "${compiler_dir}/cmake-build-debug" -o -path "${compiler_dir}/.idea" \) -prune -o \
    -type f \( -name "*.c" -o -name "*.cc" -o -name "*.cpp" -o -name "*.cxx" -o -name "*.h" -o -name "*.hh" -o -name "*.hpp" \) \
    -print0 | sort -z
)

if [ "${#files[@]}" -eq 0 ]; then
  echo "No C++ source files found under ${compiler_dir}."
  exit 0
fi

needs_format=()
for file in "${files[@]}"; do
  if ! clang-format --dry-run --Werror "${file}" >/dev/null 2>&1; then
    needs_format+=("${file}")
  fi
done

if [ "${#needs_format[@]}" -eq 0 ]; then
  echo "Formatting check passed for ${#files[@]} file(s)."
  exit 0
fi

if [ -n "${GITHUB_STEP_SUMMARY:-}" ]; then
  repo_url="${GITHUB_SERVER_URL:-https://github.com}/${REPORT_REPO:-}/${REPORT_SHA:+blob/${REPORT_SHA}}"
  {
    echo "## Formatting issues found: ${#needs_format[@]}"
    echo
    echo "The following files need `clang-format`:"
    echo
    for file in "${needs_format[@]}"; do
      if [ -n "${REPORT_REPO:-}" ] && [ -n "${REPORT_SHA:-}" ]; then
        echo "- [${file}](${repo_url}/${file})"
      else
        echo "- ${file}"
      fi
    done
  } >> "${GITHUB_STEP_SUMMARY}"
fi

printf 'Formatting issues found in:\n' >&2
printf ' - %s\n' "${needs_format[@]}" >&2
exit 1
