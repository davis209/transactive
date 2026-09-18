# PABridgeAgent

PABridgeAgent bridges the existing PAAgent interface for Web PA clients.

It does not require PAAgent code changes. It subscribes to the existing `PAAgentComms`
messages and exposes selected PAAgent CORBA methods as JSON REST endpoints.
JSON parsing and serialisation are handled by the bundled `src/json.hpp`.

## Run parameters

- `--pa-agent-name=<entity>`: PAAgent entity name used for CORBA resolution. This is required unless the legacy `OccPaAgent` run parameter supplies the entity name.
- The PAAgent location is read from the bridge agent's entity configuration and is used to subscribe to local PAAgent messages and request the bridge session.
- `--user-id=<key>`: required operator/user key used to request the bridge session.
- `--profile-id=<key>`: required profile key used to request the bridge session.
- The console/workstation key is resolved from the local hostname. The hostname must
  match exactly one undeleted `Console` entity's `address` in the database.
- `--user-pwd=<password>`: required password used to authenticate the bridge session.
- `--rest-port=<port>`: REST listen port; default `8088`.
- `--kafka-topic-prefix=<prefix>`: Kafka topic prefix; default `pa`.
- `--kafka-servers=<host:port,...>`: enables Kafka publishing through
  `code/C830AR/cots/librdkafka`.
- `--kafka-spool-file=<path>`: optional fallback that writes Kafka-style records to a file
  when no Kafka bootstrap server is configured.

PABridgeAgent keeps retrying until it obtains a real Authentication session and
uses that session for all PAAgent calls. Web PA clients must not generate or pass
`sessionId`. Before the session is ready, `/health` reports `starting` and the
PA REST APIs return `503`.

## REST endpoints

- `GET /health`
- `GET /api/pa/broadcasts`
- `GET /api/pa/broadcasts/{broadcastId}/config`
- `GET /api/pa/broadcasts/{broadcastId}/progress`
- `POST /api/pa/broadcasts/{broadcastId}/terminate`
- `POST /api/pa/broadcasts/{broadcastId}/remove`
- `POST /api/pa/broadcasts/{broadcastId}/change-id`
- `POST /api/pa/broadcasts/{broadcastId}/retry-station`
- `POST /api/pa/broadcasts/{broadcastId}/retry-train`
- `POST /api/pa/station/music`
- `GET /api/pa/station/music-status`
- `POST /api/pa/station/dva`
- `POST /api/pa/station/live`
- `POST /api/pa/station/record-adhoc`
- `POST /api/pa/station/adhoc-type`
- `POST /api/pa/station/adhoc-label`
- `POST /api/pa/train/dva`
- `POST /api/pa/train/live`
- `POST /api/pa/train-live/{broadcastId}/begin`
- `POST /api/pa/train-live/{broadcastId}/continue`
- `POST /api/pa/train-live/{broadcastId}/end`
- `GET /api/pa/priority-scheme`
- `GET /api/pa/config/station-dva-messages`
- `GET /api/pa/config/train-dva-messages`
- `GET /api/pa/config/dva-public-versions`
- `GET /api/pa/config/dva-private-versions`
- `GET /api/pa/config/zones`

## PAAgentComms forwarded to Kafka

- `BroadcastProgressUpdate`
- `CurrentBroadcastsUpdate`
- `DvaMessagesUpdate`
- `DvaPublicVersionsUpdate`
- `DvaPrivateVersionsUpdate`
- `PrioritySchemeUpdate`
