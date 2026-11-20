# TISBridgeAgent

TISBridgeAgent exposes the existing STIS and TTIS CORBA services to Web clients.
It follows the PABridgeAgent design: commands and snapshots use JSON REST APIs,
while existing `TISComms` notifications are converted to JSON and published to
Kafka. TISAgent does not require any modification.

Clients obtain a TransActive Authentication session separately and send it in the
`X-Transactive-Session-Id` header on every TIS REST request. `/health` does not
require a session. A missing header returns HTTP 401. The bridge forwards the
request session to TISAgent operations that require it; it does not log in,
validate, or terminate client sessions. Use HTTPS through a trusted TLS reverse
proxy and protect session IDs.

## Run parameters

- `--rest-port=<port>`: REST port, default `8089`.
- `--kafka-topic-prefix=<prefix>`: Kafka topic prefix, default `tis`; events use `<prefix>.events`, an empty key, and a JSON value with `eventType` and `payload`.
- `--kafka-servers=<host:port,...>`: Kafka bootstrap servers.
- `--kafka-spool-file=<path>`: optional file fallback for Kafka-style records.

The STIS and TTIS servants are resolved automatically from the database for
the TISBridgeAgent location, using the same discovery mechanism as STISManager
and TTISManager.

The location is read from the TISBridgeAgent entity.

## Documents

- `docs/TISBridgeAgent_REST_API.md`
- `docs/TISBridgeAgent_REST_API_EN.md`
- `docs/TISBridgeAgent_Kafka_Integration.md`
- `docs/TISBridgeAgent_Kafka_Integration_EN.md`
