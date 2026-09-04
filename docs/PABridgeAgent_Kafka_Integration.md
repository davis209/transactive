# PABridgeAgent Kafka 对接说明

本文档面向需要订阅 PA 状态变化的第三方系统和 Web PA 客户端开发团队。

`PABridgeAgent` 订阅本地 PAAgent 的 `PAAgentComms` 消息，将支持的消息转换为 JSON 后发布到 Kafka。Kafka 是事件通知通道，不替代 REST API；创建、控制、终止和查询广播仍应调用 PABridgeAgent REST API。

---

## 1. Topic 命名和消息封装

Topic 命名规则：

```text
pa.<MessageName>
```

Kafka 的 record 内容如下：

| 部分 | 内容 |
| --- | --- |
| Topic | 按上述规则生成。 |
| Key | 广播相关消息使用 broadcast ID；配置类消息使用空字符串。 |
| Value | UTF-8 JSON 文本。 |

PABridgeAgent 不在 JSON Value 中再包裹统一的 `eventType`、时间戳、location 或消息 ID。Consumer 应从 Topic 名称确定事件类型；若一个 Consumer 同时订阅多个 location，应通过不同的 Topic 前缀、独立部署或外部元数据区分来源 location。

---

## 2. 全部 PA 事件和对应 Topic

PABridgeAgent 当前订阅并转发以下全部 PAAgentComms 事件。DVA 版本事件会根据实际 Payload 类型输出到三个 Topic 之一，因此共有 7 个输出 Topic。

| MessageName | Topic | Key | 来源 PAAgentComms |
| --- | --- | --- | --- |
| `BroadcastProgressUpdate` | `pa.BroadcastProgressUpdate` | `broadcastId` | `BroadcastProgressUpdate` |
| `CurrentBroadcastsUpdate` | `pa.CurrentBroadcastsUpdate` | `toBroadcastId` | `CurrentBroadcastsUpdate` |
| `DvaMessagesUpdate` | `pa.DvaMessagesUpdate` | 空字符串 | `DvaMessagesUpdate` |
| `DvaVersionsUpdate` | `pa.DvaVersionsUpdate` | 空字符串 | `DvaVersionsUpdate`，完整版本记录 |
| `DvaPublicVersionsUpdate` | `pa.DvaPublicVersionsUpdate` | 空字符串 | `DvaVersionsUpdate`，公共版本记录 |
| `DvaPrivateVersionsUpdate` | `pa.DvaPrivateVersionsUpdate` | 空字符串 | `DvaVersionsUpdate`，私有版本记录 |
| `PrioritySchemeUpdate` | `pa.PrioritySchemeUpdate` | 空字符串 | `PrioritySchemeUpdate` |

同一个 `DvaVersionsUpdate` PAAgentComms 消息，根据 CORBA payload 的实际类型，会发布为上述三个 DVA Version Topic 之一。

---

## 3. Payload Schema

### 4.1 BroadcastProgressUpdate

在广播状态、全局状态或 zone/train 的本地执行状态变化时发布。

```json
{
  "broadcastId": "StationDva_001",
  "broadcastState": "Execution",
  "globalState": 0,
  "localProgresses": [
    {
      "zoneKeyOrTrainId": 1001,
      "localState": 0,
      "overridingSourcePriorityForZonesOnly": 0,
      "errorOccurred": false
    }
  ]
}
```

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| broadcastId | string | 广播 ID。 |
| broadcastState | string | 广播生命周期状态。 |
| globalState | number | 广播整体结果或故障状态的 PAAgent 数值枚举。 |
| localProgresses | array | 每个 PA zone 或列车的执行状态。 |
| zoneKeyOrTrainId | number | PA zone key 或 Train ID。 |
| localState | number | 本地执行状态的 PAAgent 数值枚举。 |
| overridingSourcePriorityForZonesOnly | number | 仅 zone 广播使用的覆盖源优先级。 |
| errorOccurred | boolean | 是否发生错误。 |

#### localState 枚举值

| 值 | 枚举名 | 含义 |
| --- | --- | --- |
| 0 | `LOCAL_STATE_IN_PROGRESS` | 广播进行中。 |
| 1 | `LOCAL_STATE_IN_PROGRESS_DWELL_TIME` | 广播进行中，处于停留/间隔时间。 |
| 2 | `LOCAL_STATE_CONNECTING` | 正在连接列车。 |
| 3 | `LOCAL_STATE_TRAIN_READY` | 列车已就绪。 |
| 4 | `LOCAL_STATE_CONTINUE_LIVE_ANNOUNCEMENT_REQUIRED` | 需要继续实时广播。 |
| 5 | `LOCAL_STATE_COMPLETED` | 已完成。 |
| 6 | `LOCAL_STATE_ABORTED` | 已中止。 |
| 7 | `LOCAL_STATE_FAILED_CYCLE` | 广播循环失败。 |
| 8 | `LOCAL_STATE_FAILED_ZONE` | Zone 故障。 |
| 9 | `LOCAL_STATE_FAILED_OVERRIDDEN` | 被更高优先级来源覆盖。 |
| 10 | `LOCAL_STATE_FAILED_TRAIN` | 列车侧失败。 |
| 11 | `LOCAL_STATE_FAILED_CONNECTION_RESET` | 连接被重置。 |
| 12 | `LOCAL_STATE_FAILED_INTERRUPTED` | 被中断。 |
| 13 | `LOCAL_STATE_FAILED_VERSION_CONFLICT` | 版本冲突。 |
| 14 | `LOCAL_STATE_FAILED_INVALID_REQUEST` | 请求无效。 |
| 15 | `LOCAL_STATE_FAILED_BUSY` | 目标忙。 |
| 16 | `LOCAL_STATE_FAILED_BUSY_WITH_OVERRIDE_OPTION` | 目标忙，但可选择覆盖。 |
| 17 | `LOCAL_STATE_FAILED_CONTINUE_LIVE_ANNOUNCEMENT_TIMEOUT` | 未收到 TIMS 的继续广播请求，超时。 |
| 18 | `LOCAL_STATE_FAILED_NO_PERMISSION` | 当前地点对此列车无 duty/权限。 |
| 19 | `LOCAL_STATE_TRAIN_FIRST_CYCLE_CANCELLED` | 列车首个循环已取消。 |
| 20 | `LOCAL_STATE_FAILED_BAD_COMMAND` | 列车返回无效/错误命令。 |
| 21 | `LOCAL_STATE_FAILED_DIFFERENT_ORIGIN` | 来源不匹配。 |
| 22 | `LOCAL_STATE_FAILED_DIFFERENT_VERSION` | 版本不匹配。 |
| 23 | `LOCAL_STATE_FAILED_MPU_CHANGEOVER` | MPU 切换失败。 |

### 4.2 CurrentBroadcastsUpdate

在广播创建、删除或广播 ID 变化时发布。

```json
{
  "fromBroadcastId": "StationDva_001",
  "toBroadcastId": "StationDva_002",
  "broadcastType": "StationDva",
  "sessionId": "..."
}
```

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| fromBroadcastId | string | 原广播 ID。创建时通常为空字符串。 |
| toBroadcastId | string | 新广播 ID。删除时可能为空字符串。 |
| broadcastType | string | 广播类型。 |
| sessionId | string | PAAgent 原始消息中的会话标识。Consumer 不得使用它调用 REST 或 CORBA 接口，也不应记录或显示。 |

#### broadcastType 枚举值

| 值 | 含义 |
| --- | --- |
| `StationMusic` | 车站音乐广播。 |
| `StationLive` | 车站实时广播。 |
| `StationDva` | 车站 DVA（数字语音公告）广播。 |
| `StationRecording` | 车站临时/adhoc 语音录制。 |
| `TrainLive` | 列车实时广播。 |
| `TrainDva` | 列车 DVA（数字语音公告）广播。 |
| `Unknown` | 未识别的 PAAgent 广播类型；不是正常定义的广播类型。 |

注意：当前实现会转发 `sessionId`。由于它属于敏感会话信息，生产环境应限制该 Topic 的访问权限；后续版本可能删除该字段。

### 4.3 DvaMessagesUpdate

站台 DVA 消息配置发生变化时发布。Payload 是记录数组。

```json
[
  {
    "pKey": 1001,
    "locationKey": 12345,
    "id": 12,
    "label": "Next train service",
    "type": "N",
    "tisMessageTag": 101,
    "tisLibrarySection": 1,
    "tisLibraryVersion": 3
  }
]
```

### 4.4 DVA Version Updates

`DvaVersionsUpdate`、`DvaPublicVersionsUpdate` 和 `DvaPrivateVersionsUpdate` 的 Payload 均为数组。

每条记录包含 `pKey`、`locationKey`，以及相应的版本字段：

| Topic | 版本字段 |
| --- | --- |
| `DvaVersionsUpdate` | `privateAdhoc1` 到 `privateAdhoc6`、`privatePreRecorded`、`publicAdhoc1` 到 `publicAdhoc6`、`publicPreRecorded` |
| `DvaPublicVersionsUpdate` | `publicAdhoc1` 到 `publicAdhoc6`、`publicPreRecorded` |
| `DvaPrivateVersionsUpdate` | `privateAdhoc1` 到 `privateAdhoc6`、`privatePreRecorded` |

### 4.5 PrioritySchemeUpdate

PA 优先级方案变化时发布。Payload 是三维数值数组：

```json
[
  [
    [1, 2, 3]
  ]
]
```

维度依次为：station -> zone -> mode priority。

---

## 4. 消费语义和顺序

- Producer 使用 librdkafka 的异步 `rd_kafka_producev` 发布，并在每次发布后调用非阻塞 `rd_kafka_poll(0)`。
- 当前实现没有为 Producer 指定分区；Kafka 根据 Topic 和 Key 决定分区。
- 同一个带 Key 的广播消息通常可按 broadcast ID 获得 Kafka 分区内顺序，但跨 Topic 和跨分区没有全局顺序保证。
- 当前实现没有应用层重试、持久化 outbox、去重 ID 或 exactly-once 保证。
- Consumer 必须能够处理重复消息、乱序消息、漏消息和服务重启后的状态重建。
- 建议 Consumer 收到事件后以 REST 查询作为最终状态校验，尤其是启动、重连或检测到事件缺失时。

---

## 5. 安全与部署限制

当前 PABridgeAgent 仅配置 `bootstrap.servers`。代码未提供以下 Kafka 客户端配置：

- TLS/SSL；
- SASL 用户名、密码或令牌认证；
- ACL 配置；
- Schema Registry；
- 自定义分区策略；
- Delivery callback 和失败重试策略。

因此，在投入生产前，应在网络隔离、Broker ACL、Topic ACL 和 TLS 终止策略中补足安全控制，或扩展 PABridgeAgent 的 Kafka 配置能力。

---

## 6. Consumer 接入建议

1. 使用独立 Consumer Group 消费所需 Topic。
2. 按业务需要订阅 `BroadcastProgressUpdate` 和 `CurrentBroadcastsUpdate`；配置缓存需求再订阅 DVA 和 Priority Topic。
3. 用 `broadcastId` 作为广播状态聚合键。
4. 将 Topic、分区、offset 与消费时间记录在 Consumer 自己的日志中，便于诊断。
5. 不记录、显示或使用 `CurrentBroadcastsUpdate` 中的 `sessionId`。
6. 初次启动或恢复后，调用 REST 查询接口重建广播与配置基线，再消费 Kafka 增量事件。
