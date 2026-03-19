#!/usr/bin/env bash

set -euo pipefail

paper_pdf_file="${PAPER_ROOT_FILE%.tex}.pdf"

mkdir -p "${PAPER_PAGES_DIR}"
cp "${PAPER_DIR}/${paper_pdf_file}" "${PAPER_PAGES_DIR}/${PAPER_ARTIFACT_FILE}"

cat > "${PAPER_PAGES_DIR}/index.html" <<EOF
<!doctype html>
<html lang="en">
  <head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>Thesis Paper</title>
    <style>
      :root {
        color-scheme: light;
        font-family: Georgia, "Times New Roman", serif;
      }

      body {
        margin: 0;
        min-height: 100vh;
        display: grid;
        place-items: center;
        background: #f4f0e8;
        color: #1d1a17;
      }

      main {
        width: min(720px, calc(100vw - 32px));
        padding: 32px;
        border: 1px solid #d7cfbf;
        background: #fffdf8;
        box-shadow: 0 20px 60px rgba(29, 26, 23, 0.08);
      }

      h1 {
        margin-top: 0;
        font-size: clamp(2rem, 3vw, 2.75rem);
      }

      p {
        line-height: 1.6;
      }

      a {
        color: #6f2f1f;
      }
    </style>
  </head>
  <body>
    <main>
      <h1>Thesis Paper</h1>
      <p>The latest PDF build for this repository is published here.</p>
      <p><a href="./${PAPER_ARTIFACT_FILE}">Open the PDF</a></p>
    </main>
  </body>
</html>
EOF
