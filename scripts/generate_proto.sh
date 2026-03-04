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
TYPES_PROTO="$PROTO_DIR/kuksa/val/v1/types.proto"
VAL_PROTO="$PROTO_DIR/kuksa/val/v1/val.proto"

if [[ ! -f "$TYPES_PROTO" || ! -f "$VAL_PROTO" ]]; then
  echo "Could not find expected proto files under: $PROTO_DIR"
  echo "Expected:"
  echo "  - $TYPES_PROTO"
  echo "  - $VAL_PROTO"
  exit 1
fi

mkdir -p "$OUT_DIR"

protoc-c \
  -I "$PROTO_DIR" \
  --c_out="$OUT_DIR" \
  "$TYPES_PROTO" \
  "$VAL_PROTO"

echo "Generated protobuf-c files in $OUT_DIR"
