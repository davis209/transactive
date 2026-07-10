# Examples.md

# Development Examples

本文档用于告诉 AI 如何按照本项目已有方式开发新功能。

**所有新增功能都应优先参考已有实现，不要自行设计新的实现方式。**

---

# 示例 1：新开发一个服务端Agent程序

## 开发步骤

codex你很聪明，无需我说明，请参考以下3个主要文件：

code\C830AR\transactive\app\scada\PMSAgent\src\PMSAgentMain.cpp
code\C830AR\transactive\app\scada\PMSAgent\src\PMSAgent.h
code\C830AR\transactive\app\scada\PMSAgent\src\PMSAgent.cpp

所有新开发的XXXAgent需要实现以下类
	class XXXAgent : public virtual TA_Base_Bus::IGenericAgentUser,
					 public virtual TA_Base_Bus::IDataPointAgent,
					 public virtual TA_Base_Bus::IDataNodeAgent
					 {
					 .
					 .
					 .
					 }
					 				 
照着以上PMSAgent的创建方式开发新的Agent。

IGenericAgentUser封装了一些基础功能，比如monitor/control状态切换， 从bus层或core层阅读相关代码了解该功能

# 示例 2：新增一个线程类

多线程编程时，不要用其它线程库，新建类要继承 TA_Base_Core::Thread 

参考以下类的实现方式：
code\C830AR\transactive\app\scada\PMSAgent\src\RTUDpProcessor

涉及代码：code\TA_BASE\transactive\core\threads
