# TrainVoiceAgent REST 客户端模拟器

C# WinForms 客户端，兼容 Visual Studio 2017 和 .NET Framework 4.6.1，不需要安装 NuGet 包。它发送 HTTP 请求用于联调 TrainVoiceAgent，不实现 SIP 注册、媒体通话或 Kafka 消费。

## 编译运行

在 VS2017 中打开 `TrainVoiceRestClientSimulator.sln`，生成后运行 `bin/Debug/TrainVoiceRestClientSimulator.exe`。服务地址默认为 `http://127.0.0.1:8094`，可改为目标 Agent 或网关的 HTTP(S) 地址。

1. 选择接口模板，填写列车 ID、callId 或 requestId，点击“载入模板”。修改这些参数后，需要重新载入模板；重新载入会覆盖当前路径和 JSON。
2. 根据数据库实际配置编辑 `carId`、`deviceKey`、SIP 号码等示例字段。选择请求方法并编辑相对路径和 JSON，然后点击“发送请求”。
3. 底部显示发送的 URL、JSON、HTTP 状态码、响应头、响应正文和耗时；204 无响应正文。请求超时设置为 15 秒，音频请求可能需要等待列车回复。
4. 创建通话时可点击“新建 UUID”产生小写 callId，然后载入创建模板。删除时填写相同的 callId 后载入删除模板；requestId 以字符串输入，避免大整数精度损失。

提供健康检查、列车查询、当前通话查询/创建/删除、通话请求查询/删除、音频请求、自由文本、列车控制、时间和编组、PI 命令模板。`train-information` 的时间在载入模板时取本机时间。自由文本按 UTF-8 发送，服务端限制 255 字节。

`202` 只表示 Agent 已发送 UDP，不能证明列车或 PI 已执行；`504` 表示音频响应超时，结果未知。使用真实 Agent 时，需要其数据库、列车 UDP 配置及运行状态满足相应接口要求；列车协议可配合独立的 TrainVoiceSimulator 测试。调用创建、删除和控制接口会改变服务状态或发送列车命令，请使用测试环境。

当前客户端不处理 SIP 或 Kafka，不提供自定义认证头；需要认证的部署应使用测试网关或在客户端扩展认证。服务端本身的实际安全配置以 TrainVoiceAgent 为准。
