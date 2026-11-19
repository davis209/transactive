# TrainVoiceAgent Kafka 接口说明

本文面向 Web 后端和事件消费者，描述 TrainVoiceAgent 当前代码的事件输出。接口尚未完成真实 Kafka、数据库和列车联调。

## 1 Topic 与消息格式

所有事件使用一个 Topic：<prefix>.events。默认前缀是 trainvoice，默认 Topic 为 trainvoice.events；通过 --kafka-topic-prefix 修改前缀，通过 --kafka-servers 配置 bootstrap.servers。所有 record 的 Key 均为空字符串。

Value 为 UTF-8 JSON，由 eventType 和 payload 组成。payload 包含 trainId、sequenceNumber、sourceDeviceBytes；后者是报文源设备 ID 的两个原始字节，不是 SIP 号码。没有统一的事件 ID、时间戳或 location 字段。

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

## 2 事件类型

| eventType | 列车消息 | 含义 |
| --- | --- | --- |
| AudioFunctionReset | 0x02，结果为 2 | 音频功能被复位或中断 |
| DriverCallRequested | 0x06，值为 1 | 司机请求 CCW 回呼 |
| DriverCallCancelled | 0x06，值为 0 | 司机取消回呼 |
| PassengerIntercomStateChanged | 0x03 | PI 状态变化 |
| BaggageDetectionChanged | 0x0C | 行李检测状态变化 |

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

audioFunction 是协议数字值；deviceId 和 deviceType 是 0x02 的附加字段。Reset 不完成等待中的 REST 请求；Agent 发布事件，不自动删除当前通话记录。SIP 客户端通过 REST 维护通话记录。

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

state：0 空闲、1 激活、2 CCW 通话、3 CCW 监听、4 自检。selfTestStatus：0 非自检、1 进行中、2 成功、3 失败。

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

detected 为 true 表示检测到，false 表示清除或未检测到。

司机请求和乘客 PI 激活事件也会尝试写入待处理请求表；数据库写入失败时仍会尝试发布 Kafka。重复事件在记录存在期间按列车、类型和源设备去重，删除后重发可能再次创建记录。

## 3 消费及可靠性

消费者按 eventType 分发，按 trainId 识别列车。事件可能重复或丢失；sequenceNumber 是 0–255 循环值，不是全局唯一事件 ID。Kafka 顺序只在分区内成立；所有 Key 均为空字符串，具体分区行为取决于分区器。建议消费者支持幂等处理并在重连时重建状态。

当前生产者异步入队，publish 成功仅表示本地队列接受，未确认 Broker 已接收。当前没有持久化 outbox、应用层失败重试或交付确认处理；Kafka 出错仅记录日志，可能导致客户端收不到通知。生产部署前需完成真实环境联调。
