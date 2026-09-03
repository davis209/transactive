# PABridgeAgent REST API 对接说明

本文档提供给 Web 版 PA 客户端及第三方开发团队，用于对接 `PABridgeAgent` 暴露的 REST API。

`PABridgeAgent` 负责把 Web 客户端的 REST 调用桥接到既有 `PAAgent` CORBA 接口，同时订阅 `PAAgentComms` 消息并转发到 Kafka。

---

## 1. 基本约定

### 1.1 Base URL

默认监听端口由 `PABridgeAgent` 启动参数 `--RestPort` 指定，默认值为 `8088`。

示例：

```text
http://127.0.0.1:8088
```

### 1.2 数据格式

请求体和响应体均使用 JSON。

建议请求头：

```http
Content-Type: application/json
Accept: application/json
```

### 1.3 Session 处理

第三方 Web PA 客户端不需要、也不允许生成或传递 `sessionId`。

`PABridgeAgent` 启动后会根据启动参数中的操作员、profile、location、console、password 等信息，向 Authentication 服务申请真实 session id。所有调用 `PAAgent` 的接口都会由 `PABridgeAgent` 自动携带该 session id。

当 `PABridgeAgent` 尚未成功获取 session id 时：

- `GET /health` 返回 `status=starting`
- 其他 PA REST API 返回 HTTP `503`

---

## 2. 通用响应

### 2.1 成功响应

成功响应使用 HTTP `200`。不同接口返回的数据结构不同。

示例：

```json
{
  "status": "ok"
}
```

### 2.2 错误响应

错误响应统一为 JSON：

```json
{
  "error": "PABridgeAgent has not acquired an Authentication session yet",
  "status": 503
}
```

常见状态码：

| HTTP 状态码 | 含义 |
| --- | --- |
| 200 | 调用成功 |
| 404 | 未知接口 |
| 500 | PABridgeAgent 内部处理异常 |
| 502 | 调用 PAAgent CORBA 接口失败 |
| 503 | PABridgeAgent 尚未获取 Authentication session |

---

## 3. 健康检查

### GET `/health`

用于检查 `PABridgeAgent` 是否启动。

#### 响应示例：session 已就绪

```json
{
  "status": "ok"
}
```

#### 响应示例：session 未就绪

```json
{
  "status": "starting"
}
```

第三方客户端应仅在 `/health` 返回 `status=ok` 后开放 PA 操作入口。

---

## 4. 广播查询接口

### GET `/api/pa/broadcasts`

获取当前广播列表。

#### 响应字段

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| broadcastId | string | 广播 ID |
| broadcastType | string | 广播类型 |

#### 响应示例

```json
[
  {
    "broadcastId": "StationDva_001",
    "broadcastType": "StationDva"
  }
]
```

### GET `/api/pa/broadcasts/{broadcastId}/config`

获取指定广播的配置。

#### 路径参数

| 参数 | 类型 | 说明 |
| --- | --- | --- |
| broadcastId | string | 广播 ID |

#### 响应示例

```json
{
  "broadcastId": "StationDva_001",
  "broadcastType": "StationDva",
  "isEventTriggered": false
}
```

### GET `/api/pa/broadcasts/{broadcastId}/progress`

获取指定广播的执行进度。

#### 响应示例

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

## 5. 广播控制接口

### POST `/api/pa/broadcasts/{broadcastId}/terminate`

终止指定广播。

#### 请求体

```json
{}
```

#### 响应示例

```json
{
  "broadcastId": "StationDva_001",
  "terminated": true
}
```

### POST `/api/pa/broadcasts/{broadcastId}/remove`

删除指定广播。

#### 请求体

```json
{}
```

#### 响应示例

```json
{
  "broadcastId": "StationDva_001",
  "removed": true
}
```

### POST `/api/pa/broadcasts/{broadcastId}/change-id`

修改广播 ID。

#### 请求字段

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| toBroadcastId | string | 是 | 新广播 ID |

#### 请求示例

```json
{
  "toBroadcastId": "StationDva_002"
}
```

#### 响应示例

```json
{
  "fromBroadcastId": "StationDva_001",
  "toBroadcastId": "StationDva_002"
}
```

### POST `/api/pa/broadcasts/{broadcastId}/retry-station`

重试站台广播。

#### 请求字段

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| zones | number[] | 是 | PA zone key 列表 |

#### 请求示例

```json
{
  "zones": [1001, 1002]
}
```

#### 响应示例

```json
{
  "broadcastId": "StationDva_001",
  "accepted": true
}
```

### POST `/api/pa/broadcasts/{broadcastId}/retry-train`

重试列车广播。

#### 请求字段

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| trains | number[] | 是 | Train ID 列表 |
| hasOverrideOption | boolean | 否 | 是否覆盖选项，默认 `false` |

#### 请求示例

```json
{
  "trains": [1, 2],
  "hasOverrideOption": false
}
```

---

## 6. Station PA 接口

### POST `/api/pa/station/music`

播放或切换站台背景音乐。

#### 请求字段

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| musicType | string | 是 | `LocalMusic`、`OccMusic`、`NoOccMusic`、`NoLocalMusic` |

#### 请求示例

```json
{
  "musicType": "OccMusic"
}
```

#### 响应示例

```json
{
  "accepted": true
}
```

### GET `/api/pa/station/music-status`

获取当前音乐状态。

#### 响应示例

```json
{
  "musicType": "OccMusic"
}
```

### POST `/api/pa/station/dva`

发起站台 DVA 广播。

#### 请求字段

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| messages | number[] | 是 | DVA message key 列表 |
| zones | number[] | 是 | PA zone key 列表 |
| dwellInSecs | number | 否 | 停留秒数，默认 `0` |
| hasChime | boolean | 否 | 是否播放 chime，默认 `false` |
| startTime | number | 否 | 开始时间，Unix time，默认 `0` |
| stopTime | number | 否 | 结束时间，Unix time，默认 `0` |
| periodInSecs | number | 否 | 周期秒数，默认 `0` |
| isSynchronisedWithTis | boolean | 否 | 是否与 TIS 同步，默认 `false` |
| groupName | string | 否 | 组名 |

#### 请求示例

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

#### 响应示例

```json
{
  "broadcastId": "StationDva_001"
}
```

### POST `/api/pa/station/live`

发起站台 live 广播。

#### 请求字段

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| sourceId | number | 是 | 音源 ID |
| zones | number[] | 是 | PA zone key 列表 |
| groupName | string | 否 | 组名 |

#### 请求示例

```json
{
  "sourceId": 1,
  "zones": [2001, 2002],
  "groupName": ""
}
```

### POST `/api/pa/station/record-adhoc`

录制 adhoc 消息。

#### 请求字段

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| sourceId | number | 是 | 音源 ID |
| messageKey | number | 是 | Adhoc message key |

#### 请求示例

```json
{
  "sourceId": 1,
  "messageKey": 1001
}
```

### POST `/api/pa/station/adhoc-type`

设置 adhoc 消息类型。

#### 请求字段

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| messageKey | number | 是 | Adhoc message key |
| messageType | string | 是 | 消息类型字符，例如 `N` |

#### 响应示例

```json
{
  "updated": true
}
```

### POST `/api/pa/station/adhoc-label`

设置 adhoc 消息标签。

#### 请求字段

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| messageKey | number | 是 | Adhoc message key |
| label | string | 是 | 标签 |

#### 请求示例

```json
{
  "messageKey": 1001,
  "label": "Test message"
}
```

---

## 7. Train PA 接口

### POST `/api/pa/train/dva`

发起列车 DVA 广播。

#### 请求字段

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| trains | number[] | 是 | Train ID 列表 |
| messageKey | number | 是 | DVA message key |
| libraryVersion | number | 是 | DVA library version |
| announcementPeriod | number | 否 | 播放周期 |
| announcementDuration | number | 否 | 播放持续时间 |
| hasOverrideOption | boolean | 否 | 是否覆盖选项，默认 `false` |

#### 请求示例

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

#### 响应示例

```json
{
  "broadcastId": "TrainDva_001"
}
```

### POST `/api/pa/train/live`

发起列车 live 广播。

#### 请求字段

| 字段 | 类型 | 必填 | 说明 |
| --- | --- | --- | --- |
| trains | number[] | 是 | Train ID 列表 |
| hasOverrideOption | boolean | 否 | 是否覆盖选项，默认 `false` |
| staticGroup | string | 否 | 静态组名 |

#### 请求示例

```json
{
  "trains": [1, 2],
  "hasOverrideOption": false,
  "staticGroup": ""
}
```

### POST `/api/pa/train-live/{broadcastId}/begin`

开始列车 live 广播。

#### 请求体

```json
{}
```

### POST `/api/pa/train-live/{broadcastId}/continue`

继续列车 live 广播。

#### 请求体

```json
{}
```

### POST `/api/pa/train-live/{broadcastId}/end`

结束列车 live 广播。

#### 请求体

```json
{}
```

---

## 8. 配置查询接口

### GET `/api/pa/priority-scheme`

获取 PA 优先级方案。

响应为三维数组：

```json
[
  [
    [1, 2, 3]
  ]
]
```

### GET `/api/pa/config/station-dva-messages`

获取站台 DVA 消息配置。

#### 响应字段

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| pKey | number | 主键 |
| locationKey | number | 位置 key |
| id | number | 消息 ID |
| label | string | 消息标签 |
| type | string | 消息类型 |
| tisMessageTag | number | TIS message tag |
| tisLibrarySection | number | TIS library section |
| tisLibraryVersion | number | TIS library version |

### GET `/api/pa/config/train-dva-messages`

获取列车 DVA 消息配置。

#### 响应字段

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| pKeyOrId | number | 主键或 ID |
| label | string | 消息标签 |

### GET `/api/pa/config/dva-public-versions`

获取 DVA public version 配置。

### GET `/api/pa/config/dva-private-versions`

获取 DVA private version 配置。

### GET `/api/pa/config/zones`

获取 PA zone 配置。

#### 响应字段

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| pKey | number | PA zone key |
| locationKey | number | 位置 key |
| locationName | string | 位置名称 |
| id | number | zone ID |
| label | string | zone 标签 |
| area | string | 区域 |

---

## 9. 枚举值

### broadcastType

可能值：

- `StationMusic`
- `StationLive`
- `StationDva`
- `StationRecording`
- `TrainLive`
- `TrainDva`
- `Unknown`

### broadcastState

可能值：

- `Creation`
- `Execution`
- `Runtime`
- `Termination`
- `PendingRemoval`
- `Unknown`

### musicType

请求和响应中使用以下值：

- `LocalMusic`
- `OccMusic`
- `NoOccMusic`
- `NoLocalMusic`

---

## 10. 对接建议

1. Web 客户端启动后先轮询 `GET /health`。
2. 只有当 `/health` 返回 `status=ok` 时，才允许用户发起 PA 操作。
3. Web 客户端不要在任何请求体、URL 或 header 中传递 `sessionId`。
4. 对 HTTP `503` 做可恢复处理，提示“PA bridge 正在等待认证 session”。
5. 对 HTTP `502` 做 PAAgent 依赖异常处理，提示用户稍后重试或联系后台运维。
6. 对需要 `broadcastId` 的接口，先通过创建广播接口或 `GET /api/pa/broadcasts` 获取有效 ID。
