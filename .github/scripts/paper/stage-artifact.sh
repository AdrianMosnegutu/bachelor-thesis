#!/usr/bin/env bash

set -euo pipefail

mkdir -p dist
cp "${PAPER_DIR}/${PAPER_PDF_FILE}" "dist/${PAPER_ARTIFACT_FILE}"
