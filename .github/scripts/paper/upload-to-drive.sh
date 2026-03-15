#!/usr/bin/env bash

set -euo pipefail

artifact_path="dist/${PAPER_ARTIFACT_FILE}"
log_file="$(mktemp)"
response_file="$(mktemp)"

python3 <<'PY' > "${response_file}.query"
import json
import os
import urllib.parse

folder_id = os.environ["GDRIVE_FOLDER_ID"]
name = os.environ["PAPER_ARTIFACT_FILE"]
safe_name = name.replace("'", "\\'")
query = (
    "trashed = false and "
    f"'{folder_id}' in parents and "
    f"name = '{safe_name}'"
)
params = {
    "q": query,
    "fields": "files(id,name)",
    "supportsAllDrives": "true",
    "includeItemsFromAllDrives": "true",
}
print(urllib.parse.urlencode(params))
PY

query_string="$(cat "${response_file}.query")"
auth_header="Authorization: Bearer ${GCP_ACCESS_TOKEN}"

set +e
curl -fsS \
  -H "${auth_header}" \
  "https://www.googleapis.com/drive/v3/files?${query_string}" \
  > "${response_file}" 2> "${log_file}"
curl_exit_code=$?
set -e

if [ "${curl_exit_code}" -ne 0 ]; then
  {
    echo "## Thesis deployment failed"
    echo
    echo "Google Drive file lookup failed before upload."
    echo
    echo "Check the deploy step logs for the exact API error."
  } >> "${GITHUB_STEP_SUMMARY}"
  cat "${log_file}" >&2
  exit "${curl_exit_code}"
fi

existing_file_id="$(
  python3 - "${response_file}" <<'PY'
import json
import sys

with open(sys.argv[1], "r", encoding="utf-8") as handle:
    payload = json.load(handle)

files = payload.get("files", [])
print(files[0]["id"] if files else "")
PY
)"

if [ -n "${existing_file_id}" ]; then
  upload_url="https://www.googleapis.com/upload/drive/v3/files/${existing_file_id}?uploadType=media&supportsAllDrives=true"
  request_method="PATCH"
else
  metadata_file="$(mktemp)"
  python3 <<'PY' > "${metadata_file}"
import json
import os

payload = {
    "name": os.environ["PAPER_ARTIFACT_FILE"],
    "parents": [os.environ["GDRIVE_FOLDER_ID"]],
}
print(json.dumps(payload))
PY

  boundary="paper-upload-boundary"
  multipart_body="$(mktemp)"
  {
    printf -- '--%s\r\n' "${boundary}"
    printf 'Content-Type: application/json; charset=UTF-8\r\n\r\n'
    cat "${metadata_file}"
    printf '\r\n--%s\r\n' "${boundary}"
    printf 'Content-Type: application/pdf\r\n\r\n'
    cat "${artifact_path}"
    printf '\r\n--%s--\r\n' "${boundary}"
  } > "${multipart_body}"

  upload_url="https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart&supportsAllDrives=true"
  request_method="POST"
fi

set +e
if [ "${request_method}" = "PATCH" ]; then
  curl -fsS -X PATCH \
    -H "${auth_header}" \
    -H "Content-Type: application/pdf" \
    --data-binary @"${artifact_path}" \
    "${upload_url}" \
    > "${response_file}" 2> "${log_file}"
  curl_exit_code=$?
else
  curl -fsS -X POST \
    -H "${auth_header}" \
    -H "Content-Type: multipart/related; boundary=${boundary}" \
    --data-binary @"${multipart_body}" \
    "${upload_url}" \
    > "${response_file}" 2> "${log_file}"
  curl_exit_code=$?
fi
set -e

if [ "${curl_exit_code}" -ne 0 ]; then
  {
    echo "## Thesis deployment failed"
    echo
    echo "Google Drive upload failed."
    echo
    echo "Check the deploy step logs for the exact API error."
  } >> "${GITHUB_STEP_SUMMARY}"
  cat "${log_file}" >&2
  exit "${curl_exit_code}"
fi

{
  echo "## Thesis deployment complete"
  echo
  echo "PDF uploaded successfully."
  echo
  echo "Google Drive folder: [Open folder](https://drive.google.com/drive/folders/${GDRIVE_FOLDER_ID})"
} >> "${GITHUB_STEP_SUMMARY}"
