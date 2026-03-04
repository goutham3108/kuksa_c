# kuksa_c

C client example for Eclipse KUKSA Databroker gRPC APIs.

## Does this work?

Yes, **if** all prerequisites are installed and protobuf-c sources are generated first.
Out of the box this repo is a scaffold and will not compile until you:

1. install gRPC C and protobuf-c development packages,
2. install `protoc` and `protoc-c`,
3. generate Kuksa protobuf-c files via `scripts/generate_proto.sh`.

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

> Note: upstream Kuksa proto files use `proto3 optional`, which is not supported
> by `protoc-c` yet. The generation script automatically creates a temporary
> sanitized copy (removing the `optional` keyword) so codegen can proceed.


## Common WSL fix for `protobuf-c` pkg-config error

If CMake fails with:

```
Package 'protobuf-c', required by 'virtual:world', not found
```

it usually means your distro publishes the pkg-config module as `libprotobuf-c`
instead of `protobuf-c`. The CMake config in this repo now checks both names.

You can verify with:

```bash
pkg-config --modversion libprotobuf-c || pkg-config --modversion protobuf-c
```

If `grep -n "pkg_.*PROTOBUF_C" CMakeLists.txt` still shows only `REQUIRED protobuf-c`, your local branch is outdated; pull latest branch changes before configuring.

## 2) Build

```bash
cmake -S . -B build
cmake --build build -j
```

If generation has not been run yet, CMake now fails with a clear error telling you which generated file is missing.

## 3) Run

Set a value:

```bash
./build/kuksa-c-cli set localhost:55555 Vehicle.Speed 40 "$TOKEN"
```

Read a value:

```bash
./build/kuksa-c-cli get localhost:55555 Vehicle.Speed "$TOKEN"
```

## Notes

- This example focuses on `Get` and `Set` so you can migrate from CLI-based flows to direct gRPC calls.
- You can extend the same pattern to `Subscribe`, `Actuate`, and metadata queries by adding request/response message handling for those RPC methods.
- Do not type `<token>` literally in the shell (`<` and `>` are redirection operators); use a real JWT string or a variable such as `$TOKEN`.
