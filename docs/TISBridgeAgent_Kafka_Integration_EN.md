# TISBridgeAgent Kafka Integration Guide

`TISBridgeAgent` subscribes to existing `TISComms` messages for its configured location, converts supported CORBA payloads to JSON, and publishes them to Kafka. Kafka is an asynchronous notification channel; REST remains the command and state-snapshot interface.

Browsers should not connect directly to Kafka. The recommended design is for the third-party Web backend to consume Kafka and forward relevant events to browsers through WebSocket or SSE.

## 1. Topic and Message Format

All seven event types are published to `<prefix>.events`, defaulting to `tis.events`. The prefix is configured using `--kafka-topic-prefix`. A bridge subscribes only to its entity location; use distinct prefixes, such as `tis.occ.events` and `tis.dbg.events`, when the source location must be distinguished.

Each Kafka record has an empty-string key. Its UTF-8 JSON value contains `eventType` and the existing `payload`:

```json
{"eventType":"TrainDisplayResult","payload":{"trainId":1}}
```

Consumers must subscribe to the shared topic and dispatch by `eventType`. Existing consumers of the previous topics and value format must be updated.

## 2. Kafka Keys and Ordering

The Kafka key is an empty string for every event, not a train or message ID. The IDs remain in the payload. Partition selection depends on the configured partitioner; ordering is only guaranteed within a partition, and is not guaranteed across partitions.

## 3. TTIS Events

The examples below show only the `payload` field of the Kafka value. Headings are `eventType` values.

### `TrainDisplayResult`

Final train response for a predefined, free-text, or clear command:

```json
{
  "trainId": 1,
  "timestamp": 1790820001,
  "originalCommand": "TisPredefinedMessageCommand",
  "success": true,
  "errorDetails": ""
}
```

`originalCommand` is one of `TisPredefinedMessageCommand`, `TisFreeTextMessageCommand`, or `TisClearCommand`. Match the event against the REST response using `timestamp`, command type, and train ID. The client Authentication session is not included in the payload.

### `TisTrainDownloadUpdate`

```json
{
  "trainNumber": 1,
  "type": "LibraryDownloadFinish",
  "success": true,
  "errorDetails": ""
}
```

`type` is one of `LibraryDownloadStart`, `LibraryDownloadFinish`, `LibraryUpgrade`, `ScheduleDownloadStart`, `ScheduleDownloadFinish`, or `ScheduleUpgrade`.

### `TisTrainDataVersionUpdate`

```json
{
  "trainNumber": 1,
  "predefinedLibraryVersion": 12,
  "nextPredefinedLibraryVersion": 13,
  "trainTimeScheduleVersion": 8,
  "nextTrainTimeScheduleVersion": 9
}
```

### `TisTrainTimeScheduleChange`

```json
{"timeSchedulePkey":12345,"changeType":"Modified"}
```

`changeType` is `Added`, `Deleted`, or `Modified`.

## 4. STIS/RATIS Events

### `IncomingRATISMessage`

```json
{
  "messageId": 1801,
  "sessionRef": 42,
  "requiresVetting": true,
  "type": "RATIS_IN_NEW"
}
```

This is a notification payload. Retrieve the full text, destination, times, and priority with `GET /api/tis/stis/ratis/{messageId}`.

`sessionRef` is a numeric RATIS Call Banner routing reference, not an Authentication `sessionId`.

### `RATISStatusUpdate`

```json
{"messageId":1801,"sessionRef":42,"status":"APPROVED"}
```

`status` is `APPROVED`, `NOT_APPROVED`, `REJECTED`, or `APPROVE_FAILED`.

### `RATISVetting`

```json
{"enabled":true}
```

This event indicates that OCC manual RATIS vetting mode changed.

## 5. Snapshots and Recovery

Kafka events describe changes and do not guarantee a complete state when a consumer starts. On startup or reconnection, fetch snapshots from:

- `/api/tis/ttis/downloads`
- `/api/tis/ttis/versions`
- `/api/tis/ttis/version-alarms`
- `/api/tis/stis/library-versions`
- `/api/tis/stis/ratis`
- `/api/tis/stis/ratis-vetting`

Then continue consuming Kafka. Consumers should persist offsets and apply updates idempotently by business key.

## 6. File Spool Test Mode

When Kafka is unavailable, `--kafka-spool-file=<path>` writes one record per line:

```json
{
  "topic":"tis.events",
  "key":"",
  "payload":{"eventType":"TrainDisplayResult","payload":{"trainId":1,"timestamp":1790820001,"originalCommand":"TisPredefinedMessageCommand","success":true,"errorDetails":""}}
}
```

This mode is intended for integration testing and does not provide Kafka partition, offset, acknowledgement, or retry semantics.

## 7. Current Kafka Configuration Boundary

As with `PABridgeAgent`, the current implementation directly configures only `bootstrap.servers`. TLS, SASL, broker and topic ACLs, retry policy, and production monitoring must be supplied by the deployment environment or added as future bridge configuration.
