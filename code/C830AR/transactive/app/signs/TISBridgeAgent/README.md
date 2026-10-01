# TISBridgeAgent

TISBridgeAgent exposes the existing STIS and TTIS CORBA services to Web clients.
It follows the PABridgeAgent design: commands and snapshots use JSON REST APIs,
while existing `TISComms` notifications are converted to JSON and published to
Kafka. TISAgent does not require any modification.

The bridge obtains its own Authentication session and keeps retrying until a
session is available. Web clients neither create nor pass a Transactive session
ID. Business endpoints return HTTP 503 while authentication is unavailable.

## Run parameters

- `--stis-agent-name=<entity>`: STIS servant entity name.
- `--ttis-agent-name=<entity>`: TTIS servant entity name.
- `--user-id=<key>`: operator key used by the bridge session.
- `--profile-id=<key>`: profile key used by the bridge session.
- `--user-pwd=<password>`: password used by the bridge session.
- `--rest-port=<port>`: REST port, default `8089`.
- `--kafka-topic-prefix=<prefix>`: Kafka topic prefix, default `tis`.
- `--kafka-servers=<host:port,...>`: Kafka bootstrap servers.
- `--kafka-spool-file=<path>`: optional file fallback for Kafka-style records.

The location is read from the TISBridgeAgent entity. The console key is resolved
from the local hostname in the same way as PABridgeAgent.

## Documents

- `docs/TISBridgeAgent_REST_API.md`
- `docs/TISBridgeAgent_REST_API_EN.md`
- `docs/TISBridgeAgent_Kafka_Integration.md`
- `docs/TISBridgeAgent_Kafka_Integration_EN.md`
