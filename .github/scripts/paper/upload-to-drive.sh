#!/usr/bin/env bash

set -euo pipefail

rclone copy "dist/${PAPER_ARTIFACT_FILE}" gdrive: \
  --drive-root-folder-id "${GDRIVE_FOLDER_ID}" \
  --drive-upload-cutoff 1G \
  -v

{
  echo "## Thesis deployment complete"
  echo
  echo "PDF uploaded successfully."
  echo
  echo "Google Drive folder: [Open folder](https://drive.google.com/drive/folders/${GDRIVE_FOLDER_ID})"
} >> "${GITHUB_STEP_SUMMARY}"
