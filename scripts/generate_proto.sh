#!/usr/bin/env bash
set -euo pipefail

if ! command -v protoc >/dev/null 2>&1; then
  echo "protoc is required"
  exit 1
fi
if ! command -v protoc-c >/dev/null 2>&1; then
  echo "protoc-c is required (protobuf-c generator)"
  exit 1
fi

if [[ $# -ne 1 ]]; then
  echo "Usage: $0 <path-to-kuksa-databroker-repo>"
  exit 1
fi

KUKSA_REPO=$1
PROTO_DIR="$KUKSA_REPO/proto"
OUT_DIR="$(cd "$(dirname "$0")/.." && pwd)/generated"

mkdir -p "$OUT_DIR"

protoc-c \
  -I "$PROTO_DIR" \
  --c_out="$OUT_DIR" \
  "$PROTO_DIR/kuksa/val/v1/types.proto" \
  "$PROTO_DIR/kuksa/val/v1/val.proto"

echo "Generated protobuf-c files in $OUT_DIR"
