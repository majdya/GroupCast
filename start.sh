#!/usr/bin/env bash
set -euo pipefail

PORT=${1:-8888}



echo "==> Starting server on port $PORT..."
./server1 "$PORT"
