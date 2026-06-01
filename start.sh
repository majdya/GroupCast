#!/usr/bin/env bash
set -euo pipefail

PORT=${1:-8888}

echo "==> Building server..."
make server

echo "==> Starting server on port $PORT..."
./server "$PORT"
