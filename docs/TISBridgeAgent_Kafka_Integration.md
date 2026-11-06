# TISBridgeAgent Kafka 对接说明

`TISBridgeAgent` 订阅本 Location 的现有 `TISComms` 消息，把支持的 CORBA Payload 转换成 JSON 后发布到 Kafka。Kafka 只负责异步事件通知，不替代 REST API；命令提交和状态快照仍通过 REST 获取。

浏览器不应直接连接 Kafka。推荐由第三方 Web 后端消费 Kafka，再通过 WebSocket 或 SSE 推送到浏览器。

## 1. Topic 和消息格式

7 种事件统一发布到 `<prefix>.events`，默认 Topic 为 `tis.events`。可通过 `--kafka-topic-prefix` 配置前缀。每个 Bridge 只订阅实体所属 Location；需要区分来源时，可使用不同前缀，例如 `tis.occ.events` 和 `tis.dbg.events`。

每条 Kafka Record 的 Key 均为空字符串。UTF-8 JSON Value 包含 `eventType` 和原有 `payload`：

```json
{"eventType":"TrainDisplayResult","payload":{"trainId":1}}
```

消费端订阅统一 Topic，并按 `eventType` 分发。原有 Topic 和 Value 格式的消费者需要适配。

## 2. Kafka Key 和顺序

所有事件的 Kafka Key 均为空字符串，不再使用列车或消息 ID；这些 ID 仍保留在 payload 中。分区选择取决于分区器配置，顺序仅在同一分区内保证，跨分区不保证顺序。

## 3. TTIS 事件

以下示例仅展示 Kafka Value 中的 `payload`，各小节标题为 `eventType`。

### `TrainDisplayResult`

列车对预定义消息、自由文本或清除命令的最终响应。

```json
{
  "trainId": 1,
  "timestamp": 1790820001,
  "originalCommand": "TisPredefinedMessageCommand",
  "success": true,
  "errorDetails": ""
}
```

`originalCommand`：

- `TisPredefinedMessageCommand`
- `TisFreeTextMessageCommand`
- `TisClearCommand`

Web 后端使用 REST 返回的 `timestamp`，并结合 `originalCommand` 和 `trainId` 关联原请求。Bridge 的 Authentication Session 不会写入 Payload。

### `TisTrainDownloadUpdate`

```json
{
  "trainNumber": 1,
  "type": "LibraryDownloadFinish",
  "success": true,
  "errorDetails": ""
}
```

`type` 可为：

- `LibraryDownloadStart`
- `LibraryDownloadFinish`
- `LibraryUpgrade`
- `ScheduleDownloadStart`
- `ScheduleDownloadFinish`
- `ScheduleUpgrade`

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

`changeType` 为 `Added`、`Deleted` 或 `Modified`。

## 4. STIS/RATIS 事件

### `IncomingRATISMessage`

```json
{
  "messageId": 1801,
  "sessionRef": 42,
  "requiresVetting": true,
  "type": "RATIS_IN_NEW"
}
```

该事件只包含通知字段。Consumer 收到后调用 `GET /api/tis/stis/ratis/{messageId}` 获取完整正文、目的地、时间和优先级。

`sessionRef` 是 RATIS Call Banner 的数字路由标识，不是 Authentication `sessionId`。

### `RATISStatusUpdate`

```json
{"messageId":1801,"sessionRef":42,"status":"APPROVED"}
```

`status` 为：

- `APPROVED`
- `NOT_APPROVED`
- `REJECTED`
- `APPROVE_FAILED`

### `RATISVetting`

```json
{"enabled":true}
```

表示 OCC RATIS 人工审批模式发生变化。

## 5. 初始快照与事件恢复

Kafka 事件描述变化，不保证 Consumer 启动时一定能获得完整当前状态。Web 后端启动或重新连接时应先调用 REST：

- `/api/tis/ttis/downloads`
- `/api/tis/ttis/versions`
- `/api/tis/ttis/version-alarms`
- `/api/tis/stis/library-versions`
- `/api/tis/stis/ratis`
- `/api/tis/stis/ratis-vetting`

然后再持续消费 Kafka。Consumer 应自行保存 Offset，并按业务主键进行幂等更新。

## 6. File Spool 测试模式

未连接 Kafka 时，可通过 `--kafka-spool-file=<path>` 将记录逐行写入文件：

```json
{
  "topic":"tis.events",
  "key":"",
  "payload":{"eventType":"TrainDisplayResult","payload":{"trainId":1,"timestamp":1790820001,"originalCommand":"TisPredefinedMessageCommand","success":true,"errorDetails":""}}
}
```

该模式用于接口联调，不提供 Kafka 的 Partition、Offset、确认或重试语义。

## 7. 当前 Kafka 配置边界

当前实现与 `PABridgeAgent` 一致，仅直接配置 `bootstrap.servers`。TLS、SASL、Broker ACL、Topic ACL、重试和生产级监控需由部署环境补充，或后续扩展 Bridge 的 Kafka 参数。
