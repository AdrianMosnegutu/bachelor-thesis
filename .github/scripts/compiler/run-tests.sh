#!/usr/bin/env bash

set -euo pipefail

build_dir="${COMPILER_BUILD_DIR:-build/compiler-ci}"
build_type="${COMPILER_BUILD_TYPE:-Debug}"
report_dir="${COMPILER_REPORT_DIR:-${build_dir}/.ci-reports}"

mkdir -p "${report_dir}"

ctest \
  --test-dir "${build_dir}" \
  --build-config "${build_type}" \
  --output-on-failure \
  --output-log "${report_dir}/ctest.log" \
  --output-junit "${report_dir}/ctest-junit.xml"
