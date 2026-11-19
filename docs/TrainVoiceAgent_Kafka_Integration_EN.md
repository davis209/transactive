# TrainVoiceAgent Kafka Integration Guide

This guide describes current event output for Web backends and consumers. Integration with a real Kafka broker, database and trains remains unverified.

## 1 Topic and message format

All events use one topic: <prefix>.events. The default prefix is trainvoice and the default topic is trainvoice.events. Set --kafka-topic-prefix for a different prefix and --kafka-servers for bootstrap.servers. Every record has an empty-string key.

The value is UTF-8 JSON with eventType and payload. Every payload contains trainId, sequenceNumber and sourceDeviceBytes. The latter contains the two raw source-device bytes, not a SIP number. The envelope has no common event ID, timestamp or location.

```json
{
  "eventType": "DriverCallRequested",
  "payload": {
    "trainId": 1,
    "sequenceNumber": 23,
    "sourceDeviceBytes": [
      1,
      2
    ]
  }
}
```

## 2 Event types

| eventType | Train message | Meaning |
| --- | --- | --- |
| AudioFunctionReset | 0x02, result 2 | Audio function reset or interrupted |
| DriverCallRequested | 0x06, value 1 | Driver requests a CCW callback |
| DriverCallCancelled | 0x06, value 0 | Driver cancels the callback |
| PassengerIntercomStateChanged | 0x03 | PI state changed |
| BaggageDetectionChanged | 0x0C | Baggage detection changed |

### AudioFunctionReset

```json
{
  "eventType": "AudioFunctionReset",
  "payload": {
    "trainId": 1,
    "sequenceNumber": 23,
    "sourceDeviceBytes": [
      1,
      2
    ],
    "audioFunction": 10,
    "deviceId": 1,
    "deviceType": 3
  }
}
```

audioFunction is the numeric protocol value; deviceId and deviceType are 0x02 additional fields. Reset does not complete a waiting REST request. The agent publishes the event and does not automatically delete active-call records. The SIP client maintains them through REST.

### PassengerIntercomStateChanged

```json
{
  "eventType": "PassengerIntercomStateChanged",
  "payload": {
    "trainId": 1,
    "sequenceNumber": 24,
    "sourceDeviceBytes": [
      1,
      2
    ],
    "state": 1,
    "selfTestStatus": 0
  }
}
```

state: 0 idle, 1 activated, 2 CCW talk, 3 CCW listening, 4 self-test. selfTestStatus: 0 not testing, 1 running, 2 passed, 3 failed.

### BaggageDetectionChanged

```json
{
  "eventType": "BaggageDetectionChanged",
  "payload": {
    "trainId": 1,
    "sequenceNumber": 25,
    "sourceDeviceBytes": [
      1,
      2
    ],
    "cameraId": 2,
    "detected": true
  }
}
```

detected is true when detected and false when cleared or not detected.

Driver requests and PI activations are also saved as pending requests when possible; Kafka publication is still attempted on database failure. Events are deduplicated by train, type and source device while a pending record exists; retransmission after deletion can create another record.

## 3 Consumption and reliability

Dispatch by eventType and identify the train using trainId. Events may be duplicated or lost. sequenceNumber wraps at 255 and is not a globally unique event ID. Kafka order is per partition; every key is empty and the partition distribution depends on the partitioner. Consumers should handle duplicates and rebuild state on reconnection.

The producer is asynchronous; successful publish means local enqueue, not broker delivery. There is no durable outbox, application-level retry or delivery confirmation handler. Kafka failures are logged and may prevent event delivery. Complete real-environment integration before production deployment.
