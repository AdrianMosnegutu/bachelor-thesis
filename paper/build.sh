#!/usr/bin/env bash

# Build the PDF article
latexmk -pdf main.tex

# Clean build artefacts, except the PDF article
latexmk -c
