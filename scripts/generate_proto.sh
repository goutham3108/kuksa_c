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
TMP_PROTO_DIR="$(mktemp -d)"

cleanup() {
  rm -rf "$TMP_PROTO_DIR"
}
trap cleanup EXIT

if [[ ! -f "$TYPES_PROTO" || ! -f "$VAL_PROTO" ]]; then
  echo "Could not find expected proto files under: $PROTO_DIR"
  echo "Expected:"
  echo "  - $TYPES_PROTO"
  echo "  - $VAL_PROTO"
  exit 1
fi

# timestamp.proto is imported by kuksa val protos via google.protobuf.Timestamp.
PROTOBUF_INCLUDE_DIR=""
for candidate in /usr/include /usr/local/include; do
  if [[ -f "$candidate/google/protobuf/timestamp.proto" ]]; then
    PROTOBUF_INCLUDE_DIR="$candidate"
    break
  fi
done

if [[ -z "$PROTOBUF_INCLUDE_DIR" ]]; then
  echo "Could not find google/protobuf/timestamp.proto (checked /usr/include and /usr/local/include)."
  echo "Install protobuf development headers (e.g. protobuf-compiler/libprotobuf-dev)."
  exit 1
fi

mkdir -p "$OUT_DIR"

# protobuf-c (protoc-c) does not yet support `optional` fields in proto3.
# Current kuksa-databroker protos use that feature, so we generate from a
# temporary sanitized copy by removing the `optional` keyword.
mkdir -p "$TMP_PROTO_DIR/kuksa/val/v1" "$TMP_PROTO_DIR/google/protobuf"
sed -E 's/^([[:space:]]*)optional[[:space:]]+/\1/' "$TYPES_PROTO" > "$TMP_PROTO_DIR/kuksa/val/v1/types.proto"
sed -E 's/^([[:space:]]*)optional[[:space:]]+/\1/' "$VAL_PROTO" > "$TMP_PROTO_DIR/kuksa/val/v1/val.proto"
cp "$PROTOBUF_INCLUDE_DIR/google/protobuf/timestamp.proto" "$TMP_PROTO_DIR/google/protobuf/timestamp.proto"

echo "Note: proto3 'optional' fields were sanitized for protobuf-c compatibility."

protoc-c \
  -I "$TMP_PROTO_DIR" \
  --c_out="$OUT_DIR" \
  "$TMP_PROTO_DIR/google/protobuf/timestamp.proto" \
  "$TMP_PROTO_DIR/kuksa/val/v1/types.proto" \
  "$TMP_PROTO_DIR/kuksa/val/v1/val.proto"

echo "Generated protobuf-c files in $OUT_DIR"
