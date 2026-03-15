#!/usr/bin/env bash

set -euo pipefail

build_dir="${PAPER_DIR}/out/build/ci"

mkdir -p "${build_dir}"
rm -f "${build_dir}/latexmk-output.log" "${build_dir}/latexmk-exit-code.txt"

(
  cd "${PAPER_DIR}"

  set +e
  latexmk \
    -pdf \
    -file-line-error \
    -halt-on-error \
    -interaction=nonstopmode \
    -Werror \
    -logfilewarninglist \
    "${PAPER_ROOT_FILE}" 2>&1 | tee "out/build/ci/latexmk-output.log"
  build_rc=${PIPESTATUS[0]}
  set -e

  printf '%s\n' "${build_rc}" > "out/build/ci/latexmk-exit-code.txt"
)
