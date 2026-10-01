# TISBridgeAgent Kafka Integration Guide

`TISBridgeAgent` subscribes to existing `TISComms` messages for its configured location, converts supported CORBA payloads to JSON, and publishes them to Kafka. Kafka is an asynchronous notification channel; REST remains the command and state-snapshot interface.

Browsers should not connect directly to Kafka. The recommended design is for the third-party Web backend to consume Kafka and forward relevant events to browsers through WebSocket or SSE.

## 1. Topic Names

The default `--kafka-topic-prefix` is `tis`. Topic names use:

```text
<prefix>.<TISComms message name>
```

The seven default topics are:

```text
tis.TrainDisplayResult
tis.TisTrainDownloadUpdate
tis.TisTrainDataVersionUpdate
tis.TisTrainTimeScheduleChange
tis.IncomingRATISMessage
tis.RATISStatusUpdate
tis.RATISVetting
```

Each bridge subscribes only to its entity location. Use a distinct prefix per location when several instances publish to one cluster, for example `tis.occ` and `tis.dbg`. This produces topics such as `tis.occ.TrainDisplayResult`.

## 2. Kafka Keys

The key is a separate Kafka record field and is not part of the topic name.

| Topic suffix | Key |
|---|---|
| `TrainDisplayResult` | `trainId` |
| `TisTrainDownloadUpdate` | `trainNumber` |
| `TisTrainDataVersionUpdate` | `trainNumber` |
| `TisTrainTimeScheduleChange` | `timeSchedulePkey` |
| `IncomingRATISMessage` | `messageId` |
| `RATISStatusUpdate` | `messageId` |
| `RATISVetting` | Empty string |

Records with the same key normally reach the same partition, preserving partition order for one train or RATIS message. Consumers must read the topic, key, and JSON value.

## 3. TTIS Events

### `tis.TrainDisplayResult`

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

`originalCommand` is one of `TisPredefinedMessageCommand`, `TisFreeTextMessageCommand`, or `TisClearCommand`. Match the event against the REST response using `timestamp`, command type, and train ID. The bridge Authentication session is not included in the payload.

### `tis.TisTrainDownloadUpdate`

```json
{
  "trainNumber": 1,
  "type": "LibraryDownloadFinish",
  "success": true,
  "errorDetails": ""
}
```

`type` is one of `LibraryDownloadStart`, `LibraryDownloadFinish`, `LibraryUpgrade`, `ScheduleDownloadStart`, `ScheduleDownloadFinish`, or `ScheduleUpgrade`.

### `tis.TisTrainDataVersionUpdate`

```json
{
  "trainNumber": 1,
  "predefinedLibraryVersion": 12,
  "nextPredefinedLibraryVersion": 13,
  "trainTimeScheduleVersion": 8,
  "nextTrainTimeScheduleVersion": 9
}
```

### `tis.TisTrainTimeScheduleChange`

```json
{"timeSchedulePkey":12345,"changeType":"Modified"}
```

`changeType` is `Added`, `Deleted`, or `Modified`.

## 4. STIS/RATIS Events

### `tis.IncomingRATISMessage`

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

### `tis.RATISStatusUpdate`

```json
{"messageId":1801,"sessionRef":42,"status":"APPROVED"}
```

`status` is `APPROVED`, `NOT_APPROVED`, `REJECTED`, or `APPROVE_FAILED`.

### `tis.RATISVetting`

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
  "topic":"tis.TrainDisplayResult",
  "key":"1",
  "payload":{"trainId":1,"timestamp":1790820001,"originalCommand":"TisPredefinedMessageCommand","success":true,"errorDetails":""}
}
```

This mode is intended for integration testing and does not provide Kafka partition, offset, acknowledgement, or retry semantics.

## 7. Current Kafka Configuration Boundary

As with `PABridgeAgent`, the current implementation directly configures only `bootstrap.servers`. TLS, SASL, broker and topic ACLs, retry policy, and production monitoring must be supplied by the deployment environment or added as future bridge configuration.
