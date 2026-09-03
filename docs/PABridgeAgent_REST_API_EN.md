# PABridgeAgent REST API Integration Guide

This document is intended for the Web PA client and third-party development teams integrating with the REST API exposed by `PABridgeAgent`.

`PABridgeAgent` bridges REST calls from Web clients to the existing `PAAgent` CORBA interface. It also subscribes to `PAAgentComms` messages and forwards them to Kafka.

---

## 1. General Conventions

### 1.1 Base URL

The listen port is configured with the `--RestPort` startup parameter. The default is `8088`.

Example:

```text
http://127.0.0.1:8088
```

### 1.2 Data Format

Request and response bodies use JSON.

Recommended request headers:

```http
Content-Type: application/json
Accept: application/json
```

### 1.3 Session Handling

Third-party Web PA clients must not generate or send a `sessionId`.

After startup, `PABridgeAgent` uses the configured operator, profile, location, console, password, and other startup parameters to request a real session ID from the Authentication service. `PABridgeAgent` automatically supplies this session ID to every PAAgent call.

Before `PABridgeAgent` has acquired a session ID:

- `GET /health` returns `status=starting`.
- All other PA REST APIs return HTTP `503`.

---

## 2. Common Responses

### 2.1 Successful Response

Successful requests return HTTP `200`. The response schema depends on the endpoint.

```json
{
  "status": "ok"
}
```

### 2.2 Error Response

Errors use the following JSON structure:

```json
{
  "error": "PABridgeAgent has not acquired an Authentication session yet",
  "status": 503
}
```

| HTTP status | Meaning |
| --- | --- |
| 200 | Request succeeded. |
| 404 | Unknown endpoint. |
| 500 | Internal PABridgeAgent processing error. |
| 502 | PAAgent CORBA call failed. |
| 503 | PABridgeAgent has not acquired an Authentication session. |

---

## 3. Health Check

### GET `/health`

Checks whether `PABridgeAgent` is running.

#### Response: session ready

```json
{
  "status": "ok"
}
```

#### Response: session not ready

```json
{
  "status": "starting"
}
```

Third-party clients should enable PA operation controls only after `/health` returns `status=ok`.

---

## 4. Broadcast Query APIs

### GET `/api/pa/broadcasts`

Returns the current broadcast list.

| Field | Type | Description |
| --- | --- | --- |
| broadcastId | string | Broadcast ID. |
| broadcastType | string | Broadcast type. |

```json
[
  {
    "broadcastId": "StationDva_001",
    "broadcastType": "StationDva"
  }
]
```

### GET `/api/pa/broadcasts/{broadcastId}/config`

Returns the configuration of a specified broadcast.

| Path parameter | Type | Description |
| --- | --- | --- |
| broadcastId | string | Broadcast ID. |

```json
{
  "broadcastId": "StationDva_001",
  "broadcastType": "StationDva",
  "isEventTriggered": false
}
```

### GET `/api/pa/broadcasts/{broadcastId}/progress`

Returns execution progress for a specified broadcast.

```json
{
  "broadcastId": "StationDva_001",
  "broadcastState": "Execution",
  "globalState": 1,
  "localProgresses": [
    {
      "zoneKeyOrTrainId": 1001,
      "localState": 1,
      "overridingSourcePriorityForZonesOnly": 0,
      "errorOccurred": false
    }
  ]
}
```

---

## 5. Broadcast Control APIs

### POST `/api/pa/broadcasts/{broadcastId}/terminate`

Terminates a specified broadcast.

Request body:

```json
{}
```

```json
{
  "broadcastId": "StationDva_001",
  "terminated": true
}
```

### POST `/api/pa/broadcasts/{broadcastId}/remove`

Removes a specified broadcast.

Request body:

```json
{}
```

```json
{
  "broadcastId": "StationDva_001",
  "removed": true
}
```

### POST `/api/pa/broadcasts/{broadcastId}/change-id`

Changes a broadcast ID.

| Field | Type | Required | Description |
| --- | --- | --- | --- |
| toBroadcastId | string | Yes | New broadcast ID. |

```json
{
  "toBroadcastId": "StationDva_002"
}
```

```json
{
  "fromBroadcastId": "StationDva_001",
  "toBroadcastId": "StationDva_002"
}
```

### POST `/api/pa/broadcasts/{broadcastId}/retry-station`

Retries a station broadcast.

| Field | Type | Required | Description |
| --- | --- | --- | --- |
| zones | number[] | Yes | PA zone key list. |

```json
{
  "zones": [1001, 1002]
}
```

```json
{
  "broadcastId": "StationDva_001",
  "accepted": true
}
```

### POST `/api/pa/broadcasts/{broadcastId}/retry-train`

Retries a train broadcast.

| Field | Type | Required | Description |
| --- | --- | --- | --- |
| trains | number[] | Yes | Train ID list. |
| hasOverrideOption | boolean | No | Override option. Default: `false`. |

```json
{
  "trains": [1, 2],
  "hasOverrideOption": false
}
```

---

## 6. Station PA APIs

### POST `/api/pa/station/music`

Starts or switches station background music.

| Field | Type | Required | Description |
| --- | --- | --- | --- |
| musicType | string | Yes | `LocalMusic`, `OccMusic`, `NoOccMusic`, or `NoLocalMusic`. |

```json
{
  "musicType": "OccMusic"
}
```

```json
{
  "accepted": true
}
```

### GET `/api/pa/station/music-status`

Returns the current music state.

```json
{
  "musicType": "OccMusic"
}
```

### POST `/api/pa/station/dva`

Starts a station DVA broadcast.

| Field | Type | Required | Description |
| --- | --- | --- | --- |
| messages | number[] | Yes | DVA message key list. |
| zones | number[] | Yes | PA zone key list. |
| dwellInSecs | number | No | Dwell time in seconds. Default: `0`. |
| hasChime | boolean | No | Play a chime. Default: `false`. |
| startTime | number | No | Start time as Unix time. Default: `0`. |
| stopTime | number | No | Stop time as Unix time. Default: `0`. |
| periodInSecs | number | No | Repeat period in seconds. Default: `0`. |
| isSynchronisedWithTis | boolean | No | Synchronize with TIS. Default: `false`. |
| groupName | string | No | Group name. |

```json
{
  "messages": [1001],
  "zones": [2001, 2002],
  "dwellInSecs": 0,
  "hasChime": true,
  "startTime": 0,
  "stopTime": 0,
  "periodInSecs": 0,
  "isSynchronisedWithTis": false,
  "groupName": ""
}
```

```json
{
  "broadcastId": "StationDva_001"
}
```

### POST `/api/pa/station/live`

Starts a station live broadcast.

| Field | Type | Required | Description |
| --- | --- | --- | --- |
| sourceId | number | Yes | Audio source ID. |
| zones | number[] | Yes | PA zone key list. |
| groupName | string | No | Group name. |

```json
{
  "sourceId": 1,
  "zones": [2001, 2002],
  "groupName": ""
}
```

### POST `/api/pa/station/record-adhoc`

Records an ad hoc message.

| Field | Type | Required | Description |
| --- | --- | --- | --- |
| sourceId | number | Yes | Audio source ID. |
| messageKey | number | Yes | Ad hoc message key. |

```json
{
  "sourceId": 1,
  "messageKey": 1001
}
```

### POST `/api/pa/station/adhoc-type`

Sets the type of an ad hoc message.

| Field | Type | Required | Description |
| --- | --- | --- | --- |
| messageKey | number | Yes | Ad hoc message key. |
| messageType | string | Yes | Message type character, for example `N`. |

```json
{
  "updated": true
}
```

### POST `/api/pa/station/adhoc-label`

Sets the label of an ad hoc message.

| Field | Type | Required | Description |
| --- | --- | --- | --- |
| messageKey | number | Yes | Ad hoc message key. |
| label | string | Yes | Message label. |

```json
{
  "messageKey": 1001,
  "label": "Test message"
}
```

---

## 7. Train PA APIs

### POST `/api/pa/train/dva`

Starts a train DVA broadcast.

| Field | Type | Required | Description |
| --- | --- | --- | --- |
| trains | number[] | Yes | Train ID list. |
| messageKey | number | Yes | DVA message key. |
| libraryVersion | number | Yes | DVA library version. |
| announcementPeriod | number | No | Announcement repeat period. |
| announcementDuration | number | No | Announcement duration. |
| hasOverrideOption | boolean | No | Override option. Default: `false`. |

```json
{
  "trains": [1, 2],
  "messageKey": 1001,
  "libraryVersion": 1,
  "announcementPeriod": 0,
  "announcementDuration": 0,
  "hasOverrideOption": false
}
```

```json
{
  "broadcastId": "TrainDva_001"
}
```

### POST `/api/pa/train/live`

Starts a train live broadcast.

| Field | Type | Required | Description |
| --- | --- | --- | --- |
| trains | number[] | Yes | Train ID list. |
| hasOverrideOption | boolean | No | Override option. Default: `false`. |
| staticGroup | string | No | Static group name. |

```json
{
  "trains": [1, 2],
  "hasOverrideOption": false,
  "staticGroup": ""
}
```

### POST `/api/pa/train-live/{broadcastId}/begin`

Starts a train live broadcast.

Request body:

```json
{}
```

### POST `/api/pa/train-live/{broadcastId}/continue`

Continues a train live broadcast.

Request body:

```json
{}
```

### POST `/api/pa/train-live/{broadcastId}/end`

Ends a train live broadcast.

Request body:

```json
{}
```

---

## 8. Configuration Query APIs

### GET `/api/pa/priority-scheme`

Returns the PA priority scheme as a three-dimensional array.

```json
[
  [
    [1, 2, 3]
  ]
]
```

### GET `/api/pa/config/station-dva-messages`

Returns station DVA message configuration.

| Field | Type | Description |
| --- | --- | --- |
| pKey | number | Primary key. |
| locationKey | number | Location key. |
| id | number | Message ID. |
| label | string | Message label. |
| type | string | Message type. |
| tisMessageTag | number | TIS message tag. |
| tisLibrarySection | number | TIS library section. |
| tisLibraryVersion | number | TIS library version. |

### GET `/api/pa/config/train-dva-messages`

Returns train DVA message configuration.

| Field | Type | Description |
| --- | --- | --- |
| pKeyOrId | number | Primary key or ID. |
| label | string | Message label. |

### GET `/api/pa/config/dva-public-versions`

Returns DVA public-version configuration.

### GET `/api/pa/config/dva-private-versions`

Returns DVA private-version configuration.

### GET `/api/pa/config/zones`

Returns PA zone configuration.

| Field | Type | Description |
| --- | --- | --- |
| pKey | number | PA zone key. |
| locationKey | number | Location key. |
| locationName | string | Location name. |
| id | number | Zone ID. |
| label | string | Zone label. |
| area | string | Area. |

---

## 9. Enumeration Values

### broadcastType

- `StationMusic`
- `StationLive`
- `StationDva`
- `StationRecording`
- `TrainLive`
- `TrainDva`
- `Unknown`

### broadcastState

- `Creation`
- `Execution`
- `Runtime`
- `Termination`
- `PendingRemoval`
- `Unknown`

### musicType

The following values are used in requests and responses:

- `LocalMusic`
- `OccMusic`
- `NoOccMusic`
- `NoLocalMusic`

---

## 10. Integration Recommendations

1. Poll `GET /health` when the Web client starts.
2. Allow PA operations only when `/health` returns `status=ok`.
3. Never send `sessionId` in a request body, URL, or header.
4. Treat HTTP `503` as recoverable and indicate that the PA bridge is waiting for an Authentication session.
5. Handle HTTP `502` as a PAAgent dependency failure; prompt the operator to retry later or contact system support.
6. For APIs requiring `broadcastId`, obtain a valid ID from a broadcast-creation endpoint or `GET /api/pa/broadcasts` first.
