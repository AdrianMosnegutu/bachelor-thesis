#!/usr/bin/env bash

ensure_not_sourced() {
  local script_path="$1"
  local hint="${2:-Run it as an executable script instead.}"

  if [[ "${script_path}" != "$0" ]]; then
    echo "Do not source this script. ${hint}" >&2
    return 1
  fi
}

project_root_from_script() {
  local script_path="$1"
  cd "$(dirname "${script_path}")/.." && pwd
}

require_cmd() {
  local cmd="$1"
  if ! command -v "${cmd}" >/dev/null 2>&1; then
    echo "Missing required command: ${cmd}" >&2
    exit 1
  fi
}

require_cmds() {
  local cmd
  for cmd in "$@"; do
    require_cmd "${cmd}"
  done
}

collect_files() {
  local root_dir="$1"
  shift
  find "${root_dir}" -type f \( "$@" \) -print0
}

command_supports_flag() {
  local cmd="$1"
  local flag="$2"
  "${cmd}" --help 2>&1 | grep -q -- "${flag}"
}
