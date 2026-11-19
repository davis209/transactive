# TrainVoiceAgent REST API 接口说明

本文面向 Web SIP 客户端和集成开发人员，描述当前代码已实现的接口。当前实现尚未完成真实数据库、Kafka、列车及主备环境联调，不可视为已验证的生产接口。

## 1 接入与约定

默认地址为 http://127.0.0.1:8089；可使用 --rest-bind-address 和 --rest-port 修改。请求和响应使用 JSON；POST 请求头使用 Content-Type: application/json。请求体上限为 1 MiB。服务没有内置调用者认证或 TLS，应部署在受信任的网关之后。浏览器跨域访问需要在网关配置 CORS。当前使用精确路径匹配，勿附加查询字符串。

接口仅在 Control 模式且 UDP 已绑定时发送列车命令。列车 ID 范围为 1 至 254。每个 POST 必须提供正整数 carId；数据库中该列车车辆必须恰有一个启用的 TCCU，且配置有效 IP、端口和协议设备 ID（例如 0x0201 表示 TCCU 编号 1）。

| 方法 | 路径 | 用途 |
| --- | --- | --- |
| GET | /health | 查询 UDP 就绪状态 |
| GET | /api/train-voice/trains | 查询列车、车辆、设备和 SIP 号码 |
| GET | /api/train-voice/active-calls | 查询当前通话 |
| GET | /api/train-voice/call-requests | 查询待处理司机及乘客请求 |
| DELETE | /api/train-voice/call-requests/{requestId} | 删除待处理请求 |
| POST | /api/train-voice/active-calls | 创建当前通话记录 |
| DELETE | /api/train-voice/active-calls/{callId} | 删除当前通话记录 |
| POST | /api/train-voice/trains/{trainId}/audio-requests | 发送 0x01，等待 0x02 |
| POST | /api/train-voice/trains/{trainId}/pi-commands | 向 TCCU 发送 0x04 PI 命令 |
| POST | /api/train-voice/trains/{trainId}/live-text | 发送 0x25 自由文本 |
| POST | /api/train-voice/trains/{trainId}/md-commands | 发送 0x0A 原始字节 |
| POST | /api/train-voice/trains/{trainId}/train-information | 按请求发送一次 0x0B |

## 2 GET /health

Control 模式且 UDP Socket 已绑定时返回 HTTP 200 和 {"status":"ok"}；否则返回 HTTP 503 和 {"status":"starting","reason":"train communication not ready"}。该检查不验证数据库或 Kafka。

## 3 GET /api/train-voice/trains

返回启用列车、车辆、设备和 SIP 号码。未配置车辆的列车返回空 cars；未配置设备的车辆返回空 devices。IP 地址和端口不对外返回。

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

protocolDeviceId 是数据库保存的整数；示例仅说明格式。数据库失败返回 503。

## 4 GET /api/train-voice/active-calls

返回当前记录；startedAt 是数据库会话时区的时间字符串，不附带时区。

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

### 4.1 创建当前通话

SIP 客户端确认通话建立后调用 POST /api/train-voice/active-calls。客户端生成并传入小写、带连字符的 UUID callId，并在重试时保持不变。

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

全部字段必填。trainId 为 1–254；carId 和 deviceKey 为 1–4294967295；audioFunction 为 2、4、6、10 或 11。字符串不得为空或包含反斜杠、控制字符；sipExtension、sipCallId、clientId 的 UTF-8 字节长度分别不超过 64、255、128。数据库外键要求列车、车辆和设备的关联有效。callId 和 sipCallId 分别是业务记录 ID 与 SIP Call-ID。

HTTP 200 返回 {"callId":"550e8400-e29b-41d4-a716-446655440000","status":"active"}。同一 callId 的相同内容重试返回成功，不同内容返回 409。数据库错误返回 503。

### 4.2 删除当前通话

SIP 客户端通话结束时调用 DELETE /api/train-voice/active-calls/{callId}。HTTP 204 不带响应体；记录不存在时也返回 204。无效 callId 返回 400。客户端应对同一通话串行执行创建和删除，并自行处理重试与重启恢复，避免延迟的创建覆盖删除。


### 4.3 查询和删除待处理通话请求

GET /api/train-voice/call-requests 返回司机及乘客待处理请求。requestId 是十进制字符串，避免 JavaScript 丢失 64 位整数精度；receivedAt 为数据库会话时区时间。sourceDeviceId 是协议设备 ID（deviceType 左移 8 位，加 deviceIndex）；例如 770 为 PI 0x0302，可与列车设备列表中的 protocolDeviceId 匹配以查找 SIP 号码。

```json
{
  "requests": [
    {"requestId":"1","trainId":1,"requestType":"PASSENGER","sourceDeviceId":770,"deviceType":3,"deviceIndex":2,"receivedAt":"2026-11-19 10:30:00"}
  ]
}
```

DELETE /api/train-voice/call-requests/{requestId} 删除指定记录，成功或记录不存在均返回 204；非法或超出无符号 64 位范围的 ID 返回 400。司机取消及 PI 状态变化不会自动删除记录，由 SIP 客户端决定并调用删除接口。

Agent 收到司机 0x06 请求值 1 或 PI 0x03 状态 1 时按列车、请求类型及源设备保存；未删除期间重复上报不会增加记录。删除后再次收到重发可能生成新记录。数据库保存失败时仍尝试发送 Kafka，但目前没有持久化重试。

## 5 POST /api/train-voice/trains/{trainId}/audio-requests

发送 0x01，按列车 ID、序号、功能和来源设备匹配 0x02；等待时间 5 秒。请求示例：

```json
{
  "carId": 101,
  "function": 10,
  "start": true,
  "additional": 1
}
```

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| carId | 正整数 | 目标车辆 |
| function | 整数 | 2 实时广播；4 预录广播；6 司机通话；10 乘客对讲；11 监听 |
| start | 布尔 | true 启动；false 停止 |
| additional | 整数 | 可省略，默认 0；范围 0–255。功能 4 为消息 ID；10、11 为 PI ID；功能 2、6 必须为 0 |

HTTP 200 表示已收到并解析列车回复，result 为 PROCEED 或 REJECTED。calleeDeviceId 和 calleeDeviceType 均为整数；设备字段均非 0、且唯一匹配配置的 SIP 号码时，返回可选的 sipExtension。

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

PROCEED 不表示 SIP 通话已建立。HTTP 504 表示等待超时、列车实际处理结果未知；勿盲目重试。0x02 Reset 不作为此接口的同步回复，而通过 Kafka 事件处理。

## 6 POST /api/train-voice/trains/{trainId}/live-text

发送 UTF-8 文字，编码后最多 255 字节；空字符串表示停止显示。只在收到 REST 请求时发送。

```json
{
  "carId": 101,
  "text": "Hello"
}
```

HTTP 202 响应为 {"status":"SENT","confirmed":false}，只表示 UDP 已提交发送，不表示列车已接收或显示。

## 7 POST /api/train-voice/trains/{trainId}/md-commands

按原始字节发送 0x0A；两个字段均为 0–255 的整数。位含义仍待协议方确认；仅在 REST 请求时发送，不周期重发。

```json
{
  "carId": 101,
  "do0": 0,
  "do1": 0
}
```

成功时 HTTP 202 返回 {"status":"SENT","confirmed":false}；没有确认列车执行。

## 8 POST /api/train-voice/trains/{trainId}/train-information

发送一次 0x0B，不周期发送。请求体提供本地时间和编组；trainId 从 URL 获取，carId 只用于查询目标 TCCU 地址。时间不得带时区，按提供的数值直接编码，不进行时区转换。carIds 按编组顺序提供 1 至 3 个互不重复的车辆 ID，每项为 1–255；不足三项由 Agent 补零。

```json
{
  "carId": 101,
  "timestamp": "2026-05-15T20:30:40",
  "carIds": [101, 102]
}
```

成功时 HTTP 202 返回 {"status":"SENT","confirmed":false}；没有确认列车执行。timestamp 格式固定为 YYYY-MM-DDTHH:MM:SS，年份 2000–2099，必须是有效日期。无效字段返回 400。

## 9 POST /api/train-voice/trains/{trainId}/pi-commands

发送 0x04 给目标车辆的 TCCU，由 TCCU 自行确定并转发给 PI；UDP 目的地址和协议头目标设备均为 TCCU。请求中不指定 PI ID。

```json
{
  "carId": 101,
  "command": 1
}
```

command 为整数：0 复位、1 应答、2 开始监听、3 结束监听、10 开始自检。数据段为 command 和一个 0 值保留字节。不支持的值返回 400。HTTP 202 返回 {"status":"SENT","confirmed":false}，只表示 UDP 已发送，不代表 TCCU 已转发或 PI 已执行。

## 10 状态码与限制

| HTTP | 含义 |
| --- | --- |
| 200 | 查询成功或收到音频允许/拒绝回复 |
| 202 | UDP 已提交发送，列车结果未确认 |
| 400 | 请求路径参数、JSON 或字段不合法 |
| 404 | POST 子路径未知，或目标 TCCU 配置不可用 |
| 409 | 目标车辆有多个 TCCU，或 callId 对应不同的通话内容 |
| 502 | 列车音频回复无法解析 |
| 503 | 数据库、UDP 或业务服务不可用；其他未实现路径也可能返回 503 |
| 504 | 等待列车音频回复超时，实际结果未知 |

当前通话由 SIP 客户端通过 REST 创建和删除；Agent 收到 Reset 后不自动删除，Reset 事件通过 Kafka 发送且尚无持久化重试。切勿将本说明中的模拟测试结果当作真实列车联调结果。
