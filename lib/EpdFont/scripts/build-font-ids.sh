#!/bin/bash
# Generates font ID #defines (hashes of builtin font headers) for src/fontIds.h
# Requires: Python 3 (stdlib only)

set -euo pipefail

cd "$(dirname "$0")"
exec python3 build_font_ids.py
