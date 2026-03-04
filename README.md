# kuksa_c

C client example for Eclipse KUKSA Databroker gRPC APIs.

## What this provides

- `kuksa-c-cli get <host:port> <path> [token]`
- `kuksa-c-cli set <host:port> <path> <value> [token]`

The client calls:

- `/kuksa.val.v1.VAL/Get`
- `/kuksa.val.v1.VAL/Set`

using native gRPC C core and protobuf-c message encoding.

## Prerequisites

- gRPC C library (`grpc` pkg-config package)
- protobuf-c (`protobuf-c` pkg-config package)
- `protoc` and `protoc-c`
- KUKSA databroker proto files from <https://github.com/eclipse-kuksa/kuksa-databroker>

## 1) Generate C protobuf types from KUKSA protos

```bash
./scripts/generate_proto.sh /path/to/kuksa-databroker
```

This creates generated files under `generated/kuksa/val/v1`.

## 2) Build

```bash
cmake -S . -B build
cmake --build build -j
```

## 3) Run

Set a value:

```bash
./build/kuksa-c-cli set localhost:55555 Vehicle.Speed 40 <token>
```

Read a value:

```bash
./build/kuksa-c-cli get localhost:55555 Vehicle.Speed <token>
```

## Notes

- This example focuses on `Get` and `Set` so you can migrate from CLI-based flows to direct gRPC calls.
- You can extend the same pattern to `Subscribe`, `Actuate`, and metadata queries by adding request/response message handling for those RPC methods.
