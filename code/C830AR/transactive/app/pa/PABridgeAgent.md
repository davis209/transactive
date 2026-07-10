# PABridgeAgent.md

# PABridgeAgent software requirement

---

# 开发需求

1. 不修改paagent代码，尽量复用pamanager的代码

2. pamanager从paagent订阅到的消息通过kafka发送给第三方应用

3. PABridgeAgent 需要把paagent的corba接口转换成restful api接口以供第三方应用调用

4. 选择合适的kafka开发库和restful api第三方开发库， 并从网上下载到 codex_tra\code\C830AR\cots\ 文件夹中， 第三方库头文件的引用需要保证正确

5. 选择restful api开发库时， 需要满足不依赖任何web容器（如tomcat）能独立运行

6. 重新阅读[docs/Examples.md]， 参看PMSAgent代码，新建一个PaBridgeAgent类

6. PABridgeAgent运行在linux系统， 参考paagent的makefile文件，生成正确的makefile文件，确保编译通过

6. PABridgeAgent的代码保存到 codex_tra\code\C830AR\transactive\app\pa\ 文件夹下
