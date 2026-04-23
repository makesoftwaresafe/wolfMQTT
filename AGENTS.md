# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Overview

wolfMQTT is an MQTT client library written in C for embedded systems (GPLv3). It supports MQTT v3.1.1 and v5.0 protocols, MQTT-SN for sensor networks, a lightweight embedded broker, and integrates with wolfSSL for TLS support.

## Build Commands

### Standard Build (Linux/macOS)
```bash
./autogen.sh          # Required if cloned from GitHub
./configure           # See --help for options
make
sudo make install
```

### Run Tests
```bash
make check            # Runs all tests with local mosquitto broker
```

### Individual Test Scripts
```bash
./scripts/client.test       # Main MQTT client tests (QoS 0-2, TLS)
./scripts/nbclient.test     # Non-blocking client tests
./scripts/multithread.test  # Multi-threading tests
./scripts/firmware.test     # Firmware update tests
./scripts/broker.test       # Broker tests (no external broker needed)
./scripts/stress.test       # Stress testing (requires --enable-stress)
```

### CMake Build
```bash
mkdir build && cd build
cmake .. -DWITH_WOLFSSL=/path/to/wolfssl/install/   # Use installed wolfSSL
# OR
cmake .. -DWITH_WOLFSSL_TREE=/path/to/wolfssl/      # Use source tree
cmake --build .
```

### Common Configure Options
```bash
--enable-tls        # TLS support (default: enabled)
--enable-v5         # MQTT v5.0 support
--enable-sn         # MQTT-SN (Sensor Network) support
--enable-nonblock   # Non-blocking I/O support
--enable-mt         # Multi-threading support
--enable-websocket  # WebSocket support (requires libwebsockets)
--enable-curl       # libcurl backend support
--enable-broker     # Lightweight broker implementation
--enable-all        # Enable all features (incompatible with --enable-curl and --enable-stress)
--enable-debug      # Debug mode (--enable-debug=verbose or --enable-debug=trace)
--enable-stress     # Stress testing (e.g., --enable-stress=t7,p8 for 7 threads, 8 pubs)
--disable-tls       # Disable TLS for testing without wolfSSL
```

### Running Examples
```bash
./examples/mqttclient/mqttclient -?    # Show help with available options
./examples/mqttclient/mqttclient -h localhost -p 1883   # Connect to local broker
./examples/mqttclient/mqttclient -h localhost -t -p 8883  # TLS connection
```

## Architecture

### Layered Design (bottom to top, in /src/)

1. **mqtt_socket.c** - Transport layer: network callbacks via `MqttNet` struct, TLS integration, timeouts
2. **mqtt_packet.c** - Packet encode/decode for all MQTT packet types (v3.1.1 and v5.0)
3. **mqtt_client.c** - High-level client API: `MqttClient_Init`, `Connect`, `Publish`, `Subscribe`, `WaitMessage`, `Disconnect`. Handles multi-threading (mutex/semaphore) and non-blocking state machines
4. **mqtt_sn_client.c / mqtt_sn_packet.c** - MQTT-SN protocol (UDP transport, gateway discovery)
5. **mqtt_broker.c** - Lightweight embedded broker: client management, subscription routing (with wildcards), QoS 0-2, retained messages, LWT, authentication

### Public Headers (in /wolfmqtt/)

- `mqtt_types.h` - Type definitions, error codes (`MQTT_CODE_*`), platform abstractions
- `mqtt_client.h` - Client API declarations
- `mqtt_packet.h` - Packet structures
- `mqtt_socket.h` - Network interface
- `mqtt_broker.h` - Broker API
- `mqtt_sn_client.h` / `mqtt_sn_packet.h` - MQTT-SN API
- `options.h` - Auto-generated build configuration (do not edit directly)

### Examples (in /examples/)

- `mqttclient/` - Full-featured reference client (best starting template)
- `mqttsimple/` - Standalone BSD sockets client
- `nbclient/` - Non-blocking I/O example
- `multithread/` - Multi-threaded publish/subscribe
- `firmware/` - Firmware update (fwpush/fwclient)
- `aws/`, `azure/`, `wiot/` - Cloud platform integrations
- `sn-client/` - MQTT-SN client
- `websocket/` - WebSocket client
- `pub-sub/` - Simple mqtt-pub and mqtt-sub utilities

### Shared Example Code

- `examples/mqttnet.c` - Network callback reference implementation
- `examples/mqttport.c` - Platform abstraction layer
- `examples/mqttexample.c` - Common example utilities

### Broker (src/mqtt_broker.c)

Lightweight broker for embedded use with configurable limits:
- `BROKER_MAX_CLIENTS` (default 8), `BROKER_MAX_SUBS` (default 32), `BROKER_MAX_RETAINED` (default 16)
- `BROKER_RX_BUF_SZ` / `BROKER_TX_BUF_SZ` (default 4096)
- Features can be individually disabled: `--disable-broker-retained`, `--disable-broker-will`, `--disable-broker-wildcards`, `--disable-broker-auth`, `--disable-broker-log`

## Key Compile Macros

```c
ENABLE_MQTT_TLS              // TLS support
WOLFMQTT_V5                  // MQTT v5.0
WOLFMQTT_SN                  // MQTT-SN protocol
WOLFMQTT_BROKER              // Broker implementation
ENABLE_MQTT_WEBSOCKET        // WebSocket support
ENABLE_MQTT_CURL             // libcurl backend
WOLFMQTT_NONBLOCK            // Non-blocking I/O
WOLFMQTT_MULTITHREAD         // Multi-threading
WOLFMQTT_DYN_PROP            // Dynamic property allocation (v5.0)
WOLFMQTT_PROPERTY_CB         // Property callback (v5.0)
WOLFMQTT_DISCONNECT_CB       // Disconnect callback
WOLFMQTT_STATIC_MEMORY       // Zero-malloc mode
DEBUG_WOLFMQTT               // Debug mode
WOLFMQTT_DEBUG_CLIENT        // Verbose client logging
WOLFMQTT_DEBUG_SOCKET        // Verbose socket logging
```

## Testing

Most tests require a local mosquitto broker. The CI uses `bubblewrap` for network isolation. `broker.test` is self-contained (no external broker needed).

To skip external broker tests:
```bash
WOLFMQTT_NO_EXTERNAL_BROKER_TESTS=1 ./configure --enable-all
make check
```

Test certificates are in `/certs/` (RSA and ECC variants).
Broker test config: `/scripts/broker_test/mosquitto.conf`

## Code Style

Uses `.clang-format` with LLVM base style:
- Tab indentation (4-space tabs)
- K&R inspired style

## Test Integrity
Never modify, delete, skip, or weaken tests to make them pass.
Never fabricate, adjust, or derive expected values from the code under test just to force a pass; fixed expected values are acceptable when they come from an independent oracle, such as committed test vectors or other externally verified results.
A passing test suite achieved by changing the tests (not the implementation) is not a passing result.
Fix the code. If the code cannot be fixed within scope, escalate.

Do not use the code under test as its only oracle where an independent oracle is required, especially for crypto, KDFs, canonical encodings, and other security-sensitive transformations. In those cases, tests should use known external test vectors, cross-validation against an independent implementation, or bit-exact comparison against a trusted reference path. For example, a test that only encrypts with function A and decrypts with function A is insufficient to validate the correctness of the cryptographic primitive.

Roundtrip/property tests are still acceptable where they match the behavior being validated, such as encode/decode or serialize/parse flows already used elsewhere in this repository, but they should not be the sole oracle when stronger independent validation is needed.

## No Fabrication
Never report status, results, or completion that does not reflect work actually performed.
If you are uncertain whether a step succeeded, say so explicitly. Do not paper over uncertainty with confident-sounding output.

## Exit Code Discipline
Every shell command's exit code must be checked.
Never proceed after a silent failure.
A command that failed and was ignored is not a completed step.

### Test vector discipline
Never derive test vectors from the code under test. Acceptable oracles:
- OpenSSL (`openssl kdf`, `openssl enc`, `openssl pkcs8 ... | sha256sum`)
- pyca/cryptography (`pkcs12.load_key_and_certificates`, `private_bytes(DER, PKCS8, NoEncryption)`)
- Bouncy Castle test vectors
Compute once, hardcode as `hex!(...)` literals or committed binary fixtures. Tests must be fully offline.
For PKCS12KDF: use `hexpass:` (BMP/UTF-16BE + null terminator), NOT `pass:` (raw ASCII).

## Dependencies

- **wolfSSL** - Required for TLS support
- **libwebsockets** - Optional, for WebSocket support
- **libcurl** - Optional, for curl backend
- **mosquitto** - For running tests
