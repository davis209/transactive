# TISBridgeAgent REST API Integration Guide

This API is intended for Web STIS Manager, Web TTIS Manager, and third-party backend integration. `TISBridgeAgent` converts JSON REST calls to the existing TISAgent CORBA interfaces. Asynchronous results are delivered through Kafka.

## 1. Conventions

- Default port: `8089`, configurable with `--rest-port`.
- Content-Type: `application/json`.
- Web clients do not create or submit a Transactive `sessionId`. The bridge owns the Authentication session.
- `GET /health` remains available before authentication. It returns `{"status":"starting"}` until the session is ready; business endpoints return HTTP `503`.
- TTIS display calls are asynchronous. A successful REST response means the request was accepted. Final per-train results arrive on `TrainDisplayResult`.

Common error:

```json
{"error":"TISAgent CORBA call failed","status":502}
```

| HTTP status | Meaning |
|---|---|
| 200 | Successful or accepted |
| 404 | Unknown endpoint |
| 500 | JSON, conversion, or internal bridge error |
| 502 | TISAgent CORBA call failed |
| 503 | Authentication session is not available |

## 2. Health

`GET /health`

```json
{"status":"ok"}
```

## 3. STIS Queries

| Method | Path | Description |
|---|---|---|
| GET | `/api/tis/stis/library-versions` | STIS/TTIS library versions, synchronisation state, and schedule version |
| GET | `/api/tis/stis/ratis` | All pending RATIS messages |
| GET | `/api/tis/stis/ratis/{messageId}` | One RATIS message |
| GET | `/api/tis/stis/ratis-vetting` | Current RATIS vetting mode |
| POST | `/api/tis/stis/current-display/query` | Current message on one PID |

Current-display request:

```json
{
  "destination": {
    "station": "DBG",
    "levels": ["Platform"],
    "pids": ["PDP01"]
  }
}
```

## 4. STIS Commands

| Method | Path | Description |
|---|---|---|
| POST | `/api/tis/stis/display/predefined` | Submit a predefined station message |
| POST | `/api/tis/stis/display/free-text` | Submit a free-text station message |
| POST | `/api/tis/stis/display/clear` | Clear PID messages |
| POST | `/api/tis/stis/pid/control` | Turn a PID on or off |
| POST | `/api/tis/stis/pid/lock` | Lock or unlock a PID |
| POST | `/api/tis/stis/library/station/upgrade` | Upgrade the station library |
| POST | `/api/tis/stis/library/train/upgrade` | Upgrade the train library |

Destination arrays use this form:

```json
"destinations": [
  {"station":"DBG","levels":["Platform"],"pids":["PDP01","PDP02"]}
]
```

Predefined request:

```json
{
  "destinations": [{"station":"DBG","levels":["Platform"],"pids":["PDP01"]}],
  "librarySection": "NORMAL_SECTION",
  "libraryVersion": 12,
  "messageTag": 101,
  "startTime": "20261001100000",
  "endTime": "20261001103000",
  "priority": 3
}
```

A free-text request supports `messageContent`, `displayMode`, `scrollSpeed`, `repeatInterval`, `displayTime`, `justification`, `plasmaFontType`, `plasmaFontSize`, `plasmaFontColour`, `plasmaBackgroundColour`, `ledFontSize`, `ledIntensity`, `ledFontColour`, and `ledBackgroundColour`, in addition to the common destination and time fields.

PID control and locking:

```json
{"destination":"DBG.TIS.Platform.PDP01","command":"TURN_ON"}
```

```json
{"destination":"DBG.TIS.Platform.PDP01","locked":true}
```

Library upgrade requests use `{"version":13}`.

## 5. RATIS

| Method | Path | Description |
|---|---|---|
| POST | `/api/tis/stis/ratis` | Submit a RATIS message |
| POST | `/api/tis/stis/ratis/{messageId}/vetting-response` | Approve or reject a RATIS message |
| POST | `/api/tis/stis/ratis-vetting` | Enable or disable manual vetting |

Submit RATIS:

```json
{
  "messageContent": "Service delay at DBG",
  "priority": 3,
  "tag": "OPS-001",
  "destination": "DBG",
  "startTime": "20261001100000",
  "endTime": "20261001103000",
  "type": "RATIS_OUT_NEW",
  "overridable": false,
  "vetting": true
}
```

Vetting response: `{"approved":true,"priority":3,"content":"Approved message text"}`.

Vetting mode: `{"enabled":true}`.

## 6. TTIS Queries

| Method | Path | Description |
|---|---|---|
| GET | `/api/tis/ttis/downloads` | Current train-download snapshot |
| GET | `/api/tis/ttis/versions` | Message-library and schedule versions by train |
| GET | `/api/tis/ttis/version-alarms` | Version-mismatch alarm state by train |

## 7. TTIS Commands

| Method | Path | Description |
|---|---|---|
| POST | `/api/tis/ttis/display/predefined` | Submit a predefined train message |
| POST | `/api/tis/ttis/display/free-text` | Submit a free-text train message |
| POST | `/api/tis/ttis/display/clear` | Clear train PID messages |
| POST | `/api/tis/ttis/message-library/download` | Download the next message library |
| POST | `/api/tis/ttis/message-library/upgrade` | Upgrade to the downloaded library |
| POST | `/api/tis/ttis/time-schedule/download` | Download the current schedule |
| POST | `/api/tis/ttis/time-schedule/upgrade` | Upgrade to the downloaded schedule |
| POST | `/api/tis/ttis/time-schedule/change` | Notify other clients of a schedule change |

Predefined request:

```json
{
  "trains": [1,2],
  "pids": [0],
  "libraryVersion": 12,
  "librarySection": "NORMAL_SECTION",
  "messageId": 5,
  "priority": 2,
  "startTime": "10:00",
  "endTime": "10:30",
  "timestamp": 1790820001
}
```

`pids` contains numeric `EPIDSelection` values: `0` is the entire train, `1..3` select all PIDs in one car, and `4..15` select an individual car PID.

A free-text request uses `trains`, `pids`, `message`, `fontSize`, `justification`, `intensity`, `displayMode`, `priority`, `startTime`, `endTime`, `repeatInterval`, and `timestamp`.

Clear request:

```json
{"trains":[1,2],"pids":[0],"clearType":0,"timestamp":1790820002}
```

Download and upgrade requests use `{"trains":[1,2]}`.

Schedule-change notification:

```json
{"timeSchedulePkey":12345,"changeType":"Modified"}
```

Display endpoints return `accepted` and the effective `timestamp`. Match this timestamp, command type, and train number against the Kafka `TrainDisplayResult` event.
