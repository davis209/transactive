# TrainVoiceAgent REST API Integration Guide

This guide describes the current implementation for Web SIP clients and integrators. Real database, Kafka, train and failover integration has not been validated; the API is not production-ready.

## 1 Connectivity and conventions

The default address is http://127.0.0.1:8094. Configure --rest-bind-address and --rest-port as needed. Requests and responses use JSON; POST requests use Content-Type: application/json. The request-body limit is 1 MiB. The server has no caller authentication or TLS; place it behind a trusted gateway. Configure CORS at the gateway for cross-origin browser access. Paths are matched exactly; do not append query strings.

Train commands are sent only in Control mode after UDP binds. Train IDs range from 1 to 254. Each POST requires a positive integer carId and exactly one enabled TCCU configured with a valid IP, port and protocol device ID (for example 0x0201 is TCCU index 1).

| Method | Path | Purpose |
| --- | --- | --- |
| GET | /health | UDP readiness |
| GET | /api/train-voice/trains | Trains, cars, devices and SIP extensions |
| GET | /api/train-voice/active-calls | Current call records |
| GET | /api/train-voice/call-requests | List pending driver and passenger requests |
| DELETE | /api/train-voice/call-requests/{requestId} | Delete a pending request |
| POST | /api/train-voice/active-calls | Create an active call |
| DELETE | /api/train-voice/active-calls/{callId} | Delete an active call |
| POST | /api/train-voice/trains/{trainId}/audio-requests | Send 0x01 and wait for 0x02 |
| POST | /api/train-voice/trains/{trainId}/pi-commands | Send 0x04 PI command to the TCCU |
| POST | /api/train-voice/trains/{trainId}/live-text | Send 0x25 live text |
| POST | /api/train-voice/trains/{trainId}/md-commands | Send raw 0x0A bytes |
| POST | /api/train-voice/trains/{trainId}/train-information | Send 0x0B once |

## 2 GET /health

HTTP 200 with {"status":"ok"} means Control mode and a bound UDP socket. Otherwise HTTP 503 returns {"status":"starting","reason":"train communication not ready"}. This is not a database or Kafka readiness check.

## 3 GET /api/train-voice/trains

Returns enabled trains, cars and devices with SIP extensions. Empty trains or cars have empty arrays. IP addresses and ports are not exposed.

```json
{
  "trains": [
    {
      "trainId": 1,
      "trainName": "Train 01",
      "cars": [
        {
          "carId": 101,
          "carName": "Car 101",
          "position": 1,
          "devices": [
            {
              "deviceKey": 3,
              "deviceType": "PI",
              "deviceName": "PI 1",
              "protocolDeviceId": 769,
              "sipExtension": "1002"
            }
          ]
        }
      ]
    }
  ]
}
```

protocolDeviceId is the integer stored in configuration; this example is illustrative. Database errors return 503.

## 4 GET /api/train-voice/active-calls

Returns current records. startedAt is a database-session local-time string without a timezone.

```json
{
  "calls": [
    {
      "callId": "call-001",
      "trainId": 1,
      "carId": 101,
      "deviceKey": 3,
      "audioFunction": 10,
      "sipExtension": "1002",
      "sipCallId": "sip-001",
      "clientId": "web-001",
      "startedAt": "2026-11-19 10:30:00"
    }
  ]
}
```

### 4.1 Create an active call

After establishing a SIP call, the SIP client calls POST /api/train-voice/active-calls with a client-generated lowercase hyphenated UUID callId. Reuse the same ID for retries.

```json
{
  "callId": "550e8400-e29b-41d4-a716-446655440000",
  "trainId": 1,
  "carId": 101,
  "deviceKey": 3,
  "audioFunction": 10,
  "sipExtension": "1002",
  "sipCallId": "sip-call-001",
  "clientId": "web-001"
}
```

All fields are required. trainId is 1–254; carId and deviceKey are 1–4294967295; audioFunction is 2, 4, 6, 10 or 11. Strings must be nonempty and cannot contain backslashes or control characters. sipExtension, sipCallId and clientId are limited to 64, 255 and 128 UTF-8 bytes respectively. Database foreign keys require valid train/car/device relationships. callId identifies the business record; sipCallId is the SIP Call-ID.

HTTP 200 returns {"callId":"550e8400-e29b-41d4-a716-446655440000","status":"active"}. A retry with the same ID and content succeeds; different content returns 409. Database errors return 503.

### 4.2 Delete an active call

When the SIP call ends, call DELETE /api/train-voice/active-calls/{callId}. HTTP 204 has no response body; an absent record also returns 204. An invalid ID returns 400. The client must serialize create and delete for each call and handle retries and recovery, preventing a delayed create after deletion.


### 4.3 List and delete pending call requests

GET /api/train-voice/call-requests returns pending driver and passenger requests. requestId is a decimal string to preserve 64-bit precision in JavaScript; receivedAt uses the database session timezone. sourceDeviceId is the protocol device ID (deviceType shifted left by 8 bits plus deviceIndex); for example 770 is PI 0x0302, which can be matched against protocolDeviceId in the train device list to find its SIP extension.

```json
{
  "requests": [
    {"requestId":"1","trainId":1,"requestType":"PASSENGER","sourceDeviceId":770,"deviceType":3,"deviceIndex":2,"receivedAt":"2026-11-19 10:30:00"}
  ]
}
```

DELETE /api/train-voice/call-requests/{requestId} returns 204 whether the record exists or not; an invalid or out-of-range unsigned 64-bit ID returns 400. Driver cancellation and PI state changes do not automatically delete records; the SIP client decides when to delete them.

A driver 0x06 request value 1 or PI 0x03 state 1 is saved by train, request type and source device. Repeat events do not create additional rows while one is pending. A retransmission after deletion may create a new record. Kafka publication is still attempted if the database write fails; there is no durable retry.

## 5 POST /api/train-voice/trains/{trainId}/audio-requests

Sends 0x01 and matches 0x02 by train ID, sequence, function and source device. Waits up to five seconds.

```json
{
  "carId": 101,
  "function": 10,
  "start": true,
  "additional": 1
}
```

| Field | Type | Meaning |
| --- | --- | --- |
| carId | Positive integer | Target car |
| function | Integer | 2 live PA; 4 prerecorded PA; 6 driver call; 10 passenger intercom; 11 listening |
| start | Boolean | true starts; false stops |
| additional | Integer | Optional, default 0; range 0–255. Message ID for 4, PI ID for 10 and 11; must be 0 for 2 and 6 |

HTTP 200 reports a parsed train response; result is PROCEED or REJECTED. calleeDeviceId and calleeDeviceType are integers. sipExtension is included only if both identifiers are nonzero and uniquely match a configured device.

```json
{
  "trainId": 1,
  "carId": 101,
  "function": 10,
  "result": "PROCEED",
  "calleeDeviceId": 1,
  "calleeDeviceType": 3,
  "sipExtension": "1002"
}
```

PROCEED does not mean the SIP call is established. HTTP 504 means the train outcome is unknown; avoid blind retries. A 0x02 Reset is handled as a Kafka event rather than the synchronous response.

## 6 POST /api/train-voice/trains/{trainId}/live-text

Sends UTF-8 text of at most 255 encoded bytes. Empty text stops display. Sent only on request.

```json
{
  "carId": 101,
  "text": "Hello"
}
```

HTTP 202 returns {"status":"SENT","confirmed":false}: UDP was submitted, but train receipt and display are not confirmed.

## 7 POST /api/train-voice/trains/{trainId}/md-commands

Sends raw 0x0A fields do0 and do1, both integers in 0–255. Bit meanings remain pending protocol confirmation. It is sent only on request.

```json
{
  "carId": 101,
  "do0": 0,
  "do1": 0
}
```

HTTP 202 returns {"status":"SENT","confirmed":false}; execution is not confirmed.

## 8 POST /api/train-voice/trains/{trainId}/train-information

Sends 0x0B once; no periodic send. The request supplies local time and the formation. trainId comes from the URL; carId is used only to look up the target TCCU. The timestamp has no timezone and its values are encoded without timezone conversion. carIds contains one to three distinct car IDs in formation order, each 1–255; the agent pads missing positions with zero.

```json
{
  "carId": 101,
  "timestamp": "2026-05-15T20:30:40",
  "carIds": [101, 102]
}
```

HTTP 202 returns {"status":"SENT","confirmed":false}; train execution is not confirmed. timestamp must be a valid date in YYYY-MM-DDTHH:MM:SS format with year 2000–2099. Invalid fields return 400.

## 9 POST /api/train-voice/trains/{trainId}/pi-commands

Sends 0x04 to the selected car TCCU, which selects and forwards to the PI. Both the UDP destination and the protocol-header destination are the TCCU. No PI ID is sent in this request.

```json
{
  "carId": 101,
  "command": 1
}
```

command is an integer: 0 reset, 1 answer, 2 start listening, 3 end listening, 10 start self-test. The payload contains the command and a zero reserved byte. Unsupported values return 400. HTTP 202 returns {"status":"SENT","confirmed":false}; UDP send does not confirm TCCU forwarding or PI execution.

## 10 Status codes and limitations

| HTTP | Meaning |
| --- | --- |
| 200 | Query succeeded or audio approval/rejection received |
| 202 | UDP submitted; train outcome unconfirmed |
| 400 | Invalid path parameter, JSON or fields |
| 404 | Unknown POST subpath or unavailable TCCU configuration |
| 409 | Multiple TCCUs, or callId already exists with different content |
| 502 | Invalid audio response |
| 503 | Database, UDP or business service unavailable; other unimplemented paths may also return 503 |
| 504 | Train audio response timed out; outcome unknown |

The SIP client creates and deletes active-call records through REST. The agent does not automatically delete on Reset; it publishes Reset events to Kafka without durable retry. Simulation tests do not establish real train interoperability.
