# TISBridgeAgent

TISBridgeAgent exposes the existing STIS and TTIS CORBA services to Web clients.
It follows the PABridgeAgent design: commands and snapshots use JSON REST APIs,
while existing `TISComms` notifications are converted to JSON and published to
Kafka. TISAgent does not require any modification.

The bridge obtains its own Authentication session and keeps retrying until a
session is available. Web clients neither create nor pass a Transactive session
ID. Business endpoints return HTTP 503 while authentication is unavailable.

## Run parameters

- `--user-id=<key>`: operator key used by the bridge session.
- `--profile-id=<key>`: profile key used by the bridge session.
- `--user-pwd=<password>`: password used by the bridge session.
- `--rest-port=<port>`: REST port, default `8089`.
- `--kafka-topic-prefix=<prefix>`: Kafka topic prefix, default `tis`.
- `--kafka-servers=<host:port,...>`: Kafka bootstrap servers.
- `--kafka-spool-file=<path>`: optional file fallback for Kafka-style records.

The STIS and TTIS servants are resolved automatically from the database for
the TISBridgeAgent location, using the same discovery mechanism as STISManager
and TTISManager.

The location is read from the TISBridgeAgent entity. The console key is resolved
from the local hostname in the same way as PABridgeAgent.

At least one TIS agent name must be configured. When only one is provided, the
bridge uses that entity name for both the STIS and TTIS CORBA interfaces. When
both are provided, each interface continues to use its explicitly configured
entity name.

## Documents

- `docs/TISBridgeAgent_REST_API.md`
- `docs/TISBridgeAgent_REST_API_EN.md`
- `docs/TISBridgeAgent_Kafka_Integration.md`
- `docs/TISBridgeAgent_Kafka_Integration_EN.md`
