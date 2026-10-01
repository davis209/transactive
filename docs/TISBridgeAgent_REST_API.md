# TISBridgeAgent REST API 对接说明

本文档供 Web 版 STIS Manager、TTIS Manager 及第三方后端对接使用。`TISBridgeAgent` 将 JSON REST 请求转换为现有 TISAgent CORBA 调用。异步结果由 Kafka 接口提供。

## 1. 基本约定

- 默认端口：`8089`，可通过 `--rest-port` 修改。
- Content-Type：`application/json`。
- Web 客户端不传递 Transactive `sessionId`；Bridge 使用启动参数自行申请并维护 Session。
- `GET /health` 不要求 Session。Session 尚未取得时返回 `{"status":"starting"}`，业务接口返回 HTTP `503`。
- TTIS 显示命令为异步命令。REST 成功只表示 TISAgent 已接受请求，最终结果通过 Kafka `TrainDisplayResult` 返回。

通用错误：

```json
{"error":"TISAgent CORBA call failed","status":502}
```

| HTTP 状态 | 含义 |
|---|---|
| 200 | 调用成功或已接受 |
| 404 | 接口不存在 |
| 500 | JSON、参数转换或 Bridge 内部错误 |
| 502 | TISAgent CORBA 调用失败 |
| 503 | Bridge 尚未取得 Authentication Session |

## 2. 公共接口

### `GET /health`

```json
{"status":"ok"}
```

## 3. STIS 查询接口

| 方法 | 路径 | 说明 |
|---|---|---|
| GET | `/api/tis/stis/library-versions` | 获取 STIS/TTIS 库版本、同步状态及当前时刻表版本 |
| GET | `/api/tis/stis/ratis` | 获取全部待处理 RATIS 消息 |
| GET | `/api/tis/stis/ratis/{messageId}` | 获取指定 RATIS 消息详情 |
| GET | `/api/tis/stis/ratis-vetting` | 查询是否启用 RATIS 人工审批 |
| POST | `/api/tis/stis/current-display/query` | 查询一个 PID 当前显示内容 |

当前显示查询请求：

```json
{
  "destination": {
    "station": "DBG",
    "levels": ["Platform"],
    "pids": ["PDP01"]
  }
}
```

返回：

```json
{
  "messageContent": "Train service is delayed",
  "startTime": "20261001100000",
  "endTime": "20261001103000",
  "priority": 3
}
```

## 4. STIS 控制接口

| 方法 | 路径 | 说明 |
|---|---|---|
| POST | `/api/tis/stis/display/predefined` | 发送预定义车站显示消息 |
| POST | `/api/tis/stis/display/free-text` | 发送自由文本车站显示消息 |
| POST | `/api/tis/stis/display/clear` | 清除 PID 消息 |
| POST | `/api/tis/stis/pid/control` | 打开或关闭 PID |
| POST | `/api/tis/stis/pid/lock` | 锁定或解锁 PID |
| POST | `/api/tis/stis/library/station/upgrade` | 升级车站消息库 |
| POST | `/api/tis/stis/library/train/upgrade` | 升级列车消息库 |

目的地数组统一格式：

```json
"destinations": [
  {"station":"DBG","levels":["Platform"],"pids":["PDP01","PDP02"]}
]
```

预定义消息请求：

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

自由文本请求除公共字段外支持：`messageContent`、`displayMode`、`scrollSpeed`、`repeatInterval`、`displayTime`、`justification`、`plasmaFontType`、`plasmaFontSize`、`plasmaFontColour`、`plasmaBackgroundColour`、`ledFontSize`、`ledIntensity`、`ledFontColour`、`ledBackgroundColour`。

清除请求：

```json
{
  "destinations": [{"station":"DBG","levels":["Platform"],"pids":["PDP01"]}],
  "upperPriority": 8,
  "lowerPriority": 1
}
```

PID 控制与锁定：

```json
{"destination":"DBG.TIS.Platform.PDP01","command":"TURN_ON"}
```

```json
{"destination":"DBG.TIS.Platform.PDP01","locked":true}
```

库升级请求为 `{"version":13}`。

## 5. RATIS 接口

| 方法 | 路径 | 说明 |
|---|---|---|
| POST | `/api/tis/stis/ratis` | 提交 RATIS 消息 |
| POST | `/api/tis/stis/ratis/{messageId}/vetting-response` | 审批或拒绝 RATIS 消息 |
| POST | `/api/tis/stis/ratis-vetting` | 开启或关闭 RATIS 人工审批 |

提交 RATIS：

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

审批请求：

```json
{"approved":true,"priority":3,"content":"Approved message text"}
```

审批模式：`{"enabled":true}`。

## 6. TTIS 查询接口

| 方法 | 路径 | 说明 |
|---|---|---|
| GET | `/api/tis/ttis/downloads` | 当前列车下载状态快照 |
| GET | `/api/tis/ttis/versions` | 各列车消息库和时刻表版本 |
| GET | `/api/tis/ttis/version-alarms` | 各列车版本不一致告警状态 |

## 7. TTIS 控制接口

| 方法 | 路径 | 说明 |
|---|---|---|
| POST | `/api/tis/ttis/display/predefined` | 发送预定义列车显示消息 |
| POST | `/api/tis/ttis/display/free-text` | 发送自由文本列车显示消息 |
| POST | `/api/tis/ttis/display/clear` | 清除列车 PID 消息 |
| POST | `/api/tis/ttis/message-library/download` | 下载下一版本消息库 |
| POST | `/api/tis/ttis/message-library/upgrade` | 切换到已下载消息库 |
| POST | `/api/tis/ttis/time-schedule/download` | 下载当前时刻表 |
| POST | `/api/tis/ttis/time-schedule/upgrade` | 切换到已下载时刻表 |
| POST | `/api/tis/ttis/time-schedule/change` | 通知其他客户端时刻表已变化 |

TTIS 预定义消息：

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

`pids` 使用 `EPIDSelection` 数值：`0` 表示整列车，`1..3` 表示各车厢全部 PID，`4..15` 表示具体车厢 PID。

自由文本请求使用：`trains`、`pids`、`message`、`fontSize`、`justification`、`intensity`、`displayMode`、`priority`、`startTime`、`endTime`、`repeatInterval`、`timestamp`。

清除请求：

```json
{"trains":[1,2],"pids":[0],"clearType":0,"timestamp":1790820002}
```

下载和升级请求统一使用：

```json
{"trains":[1,2]}
```

时刻表变化通知：

```json
{"timeSchedulePkey":12345,"changeType":"Modified"}
```

显示类接口返回 `accepted` 和实际使用的 `timestamp`。Web 后端应使用该时间戳、命令类型和列车号匹配 Kafka `TrainDisplayResult`。
