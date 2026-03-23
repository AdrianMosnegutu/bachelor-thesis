#!/usr/bin/env bash

set -euo pipefail

compiler_dir="${COMPILER_DIR:-code/compiler}"
build_dir="${COMPILER_BUILD_DIR:-build/compiler-ci}"
build_type="${COMPILER_BUILD_TYPE:-Debug}"
generator="${CMAKE_GENERATOR:-}"
report_dir="${COMPILER_REPORT_DIR:-${build_dir}/.ci-reports}"
fetchcontent_base_dir="${COMPILER_FETCHCONTENT_BASE_DIR:-}"
ccache_dir="${CCACHE_DIR:-}"

mkdir -p "${report_dir}"

if [ -n "${ccache_dir}" ]; then
  mkdir -p "${ccache_dir}"
fi

configure_log="${report_dir}/configure.log"
build_log="${report_dir}/build.log"
phase_file="${report_dir}/build-phase.txt"

configure_args=(
  -S "${compiler_dir}"
  -B "${build_dir}"
  -D CMAKE_BUILD_TYPE="${build_type}"
  -D BUILD_TESTING=ON
  -D CMAKE_EXPORT_COMPILE_COMMANDS=ON
)

if [ -z "${generator}" ] && command -v ninja >/dev/null 2>&1; then
  generator="Ninja"
fi

if [ -n "${generator}" ]; then
  configure_args+=(-G "${generator}")
fi

if [ -n "${COMPILER_GTEST_SOURCE_DIR:-}" ]; then
  configure_args+=(-D "COMPILER_GTEST_SOURCE_DIR=${COMPILER_GTEST_SOURCE_DIR}")
fi

if [ -n "${fetchcontent_base_dir}" ]; then
  mkdir -p "${fetchcontent_base_dir}"
  configure_args+=(-D "FETCHCONTENT_BASE_DIR=${fetchcontent_base_dir}")
fi

if command -v ccache >/dev/null 2>&1; then
  configure_args+=(
    -D CMAKE_CXX_COMPILER_LAUNCHER=ccache
    -D CMAKE_C_COMPILER_LAUNCHER=ccache
  )
fi

printf 'configure\n' > "${phase_file}"
if ! cmake "${configure_args[@]}" 2>&1 | tee "${configure_log}"; then
  exit 1
fi

printf 'build\n' > "${phase_file}"
if command -v ccache >/dev/null 2>&1; then
  ccache --zero-stats >/dev/null 2>&1 || true
fi

if ! cmake --build "${build_dir}" --config "${build_type}" 2>&1 | tee "${build_log}"; then
  exit 1
fi

if command -v ccache >/dev/null 2>&1; then
  {
    echo
    echo "ccache statistics:"
    ccache --show-stats || true
  } | tee -a "${build_log}" >/dev/null
fi
