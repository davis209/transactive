# PABridgeAgent

PABridgeAgent bridges the existing PAAgent interface for Web PA clients.

It does not require PAAgent code changes. It subscribes to the existing `PAAgentComms`
messages and exposes selected PAAgent CORBA methods as JSON REST endpoints.
JSON parsing and serialisation are handled by `code/C830AR/cots/nlohmann-json/json.hpp`.

## Run parameters

- `--PaAgentEntityName=<entity>`: PAAgent entity name used for CORBA resolution.
- `--PaAgentLocationKey=<key>`: location/region key used to subscribe to local PAAgent messages.
- `--PaBridgeUserKey=<key>` or `--PaBridgeOperatorKey=<key>`: operator/user key used to request the bridge session.
- `--PaBridgeProfileKey=<key>`: profile key used to request the bridge session.
- `--PaBridgeLocationKey=<key>`: location key used to request the bridge session. Defaults to `--LocationKey` or the PAAgent location key when available.
- `--PaBridgeConsoleId=<key>`: console/workstation key used to request the bridge session.
- `--PaBridgePassword=<password>`: password used to authenticate the bridge session.
- `--RestPort=<port>`: REST listen port, default `8088`.
- `--KafkaTopicPrefix=<prefix>`: topic prefix, default `paagent`.
- `--KafkaBootstrapServers=<host:port,...>`: enables Kafka publishing through
  `code/C830AR/cots/librdkafka`.
- `--KafkaSpoolFile=<path>`: optional fallback that writes Kafka-style records to a file
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
