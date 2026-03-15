#!/usr/bin/env bash

set -euo pipefail

paper_pdf_file="${PAPER_ROOT_FILE%.tex}.pdf"

mkdir -p dist
cp "${PAPER_DIR}/${paper_pdf_file}" "dist/${PAPER_ARTIFACT_FILE}"
