# TrainVoiceAgent

This is an initial implementation, not an operational agent. It contains the
Generic Agent integration skeleton, REST listener, Kafka event publisher,
PAPIS frame encoding and decoding, UDP socket, request correlation, event
conversion and Radio_Cd-backed train/device and active-call database access.
REST listens on 127.0.0.1 by default; /health reports HTTP 503 while train
communication is not ready. This health check covers UDP readiness, not the
database or Kafka. The server has no caller authentication or TLS; protect it
with a trusted gateway. Configure the bind address with --rest-bind-address
and the port with --rest-port (default 8094). The Kafka producer uses --kafka-servers
and --kafka-topic-prefix (default trainvoice); events use <prefix>.events and
an empty record key. Successful publication means queued locally, not delivered.

Train transport uses UDP unicast; both Monitor and Control instances bind port 61002, with
--train-bind-address (default 0.0.0.0). Set each TCCU's IP, port and 16-bit
protocol device ID in the database; for example 0x0201 denotes TCCU index 1.
Only replies from a configured TCCU IP for the given train are accepted.
Both operation modes provide the same REST and UDP functionality. Mode changes
do not close the UDP socket or cancel outstanding train requests. If two
instances run on the same host, configure different local bind addresses (and
REST endpoints); the same UDP address and port cannot be shared. Only Control
publishes Kafka events; Monitor still processes UDP and writes pending call
requests to the database. Events received while Monitor is active are not
buffered for later publication. Overlapping Control instances, retries and
failover can still result in duplicate or missed Kafka notifications.

GET /api/train-voice/trains and GET /api/train-voice/active-calls read the
database. SIP clients create calls with POST /api/train-voice/active-calls and
delete them with DELETE /api/train-voice/active-calls/{callId}. The client supplies
a lowercase UUID callId. An identical create retry succeeds; different content
with the same ID returns 409. Delete is idempotent. Clients must serialize create
and delete for each call. POST /api/train-voice/trains/{trainId}/audio-requests accepts
{"carId":101,"function":2,"start":true,"additional":0}; supported
functions are 2, 4, 6, 10 and 11. It waits up to five seconds for 0x02; a
timeout means the outcome is unknown. POST /api/train-voice/trains/{trainId}/live-text
accepts {"carId":101,"text":"Hello"}; UTF-8 text is limited to 255 bytes,
and an empty string stops display. HTTP 202 means only that UDP was sent.
POST /api/train-voice/trains/{trainId}/md-commands accepts
{"carId":101,"do0":0,"do1":0}; the bit meanings are pending protocol
confirmation. It sends only on REST request; no periodic 0x0A is generated.
0x0A has no confirmed acknowledgement, so 202 means UDP sent.
POST /api/train-voice/trains/{trainId}/pi-commands accepts
{"carId":101,"command":1}. Commands are 0 reset, 1 answer, 2 start
listening, 3 end listening and 10 self-test. The UDP and protocol-header
destination is the selected TCCU, which determines the PI. HTTP 202 means UDP
was sent; TCCU forwarding and PI execution are not confirmed.
POST /api/train-voice/trains/{trainId}/train-information accepts
{"carId":101,"timestamp":"2026-05-15T20:30:40","carIds":[101,102]}.
The caller supplies local time and one to three unique car IDs in formation
order; car IDs must be 1-255. The train ID is supplied in the URL. Database
configuration is used only to locate the destination TCCU. There is no periodic 0x0B.

0x03 PI state, 0x06 driver callback, 0x0C baggage detection and 0x02 reset are sent to Kafka. On reset,
the agent publishes the event and does not automatically delete active-call records.
The SIP client maintains records using the REST API. Kafka errors are
only logged and there is no durable outbox or retry. Do not deploy this code to
control trains until dependencies, failover and integration tests are complete.

0x06 driver request and 0x03 passenger activation are recorded in
train_voice_call_request, deduplicated by train, type and source device until
the record is deleted. GET /api/train-voice/call-requests lists pending
requests; DELETE /api/train-voice/call-requests/{requestId} removes one and
returns 204 even when absent. requestId is a decimal string in JSON to preserve
64-bit precision. Driver cancel and PI state changes do not delete records;
the SIP client owns deletion. A later retransmission after deletion can create a
new record. A database failure is logged and the Kafka event is still attempted;
neither database nor Kafka delivery has durable retry.

The MySQL schema is in database/train_voice_mysql.sql. The SQL registrations are
in database/sqlcode_train_voice.txt and must also be present in TA_BASE's
core/data_access_interface/src/sqlcode.txt; regenerate and rebuild TA_DAI using
the project SQL generator. Radio_Cd Read and Write must reference the database
containing those tables. The schema is a proposal and has not been applied or
validated against the target database. startedAt uses the DB session timezone.
For installations with existing tables, the additional table is provided by
database/train_voice_call_request_mysql.sql; do not run both schema files on the
same database without adapting the deployment.
