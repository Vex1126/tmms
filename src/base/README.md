# Base 模块说明文档

`base` 文件夹包含了项目的核心基础功能模块，主要分为以下几个部分：

## 📁 模块分类

### 1. 🔧 基础工具类 (Utility Classes)

#### **Singleton.h** - 单例模式模板
- **作用**：提供线程安全的单例模式实现
- **用法**：其他类继承此模板即可成为单例
- **示例**：`TaskMgr`、`FileMgr`、`Logger`、`ConfigMgr` 都使用了单例模式

#### **TTime.h/cpp** - 时间工具类
- **作用**：提供时间相关的工具函数
- **功能**：
  - `NowMs()` - 获取当前时间戳（毫秒）
  - `Now()` - 获取当前时间戳（秒）
  - `Now(year, month, day, ...)` - 获取当前时间的年月日时分秒
  - `ISOTime()` - 获取 ISO 格式的时间字符串（用于日志）

#### **StringUtils.h/cpp** - 字符串工具类
- **作用**：提供字符串处理工具函数
- **功能**：
  - `FilePath()` - 提取文件路径（去掉文件名）
  - `FileName()` - 提取文件名（去掉扩展名）
  - `FileNameExt()` - 提取文件名（包含扩展名）
  - `Extension()` - 提取文件扩展名
  - `SplitString()` - 字符串分割
  - `StartsWith()` / `EndsWith()` - 字符串匹配

---

### 2. 📝 日志系统 (Logging System)

日志系统由 4 个模块组成，采用分层设计：

#### **FileLog.h/cpp** - 文件日志底层实现
- **作用**：直接操作文件，负责日志文件的打开、写入、轮转
- **核心功能**：
  - `Open(filepath)` - 打开/创建日志文件（自动创建目录）
  - `WriteLog(msg)` - 写入日志内容到文件
  - `Rotate(newfile)` - 日志轮转（重命名旧文件）
  - `FileSize()` - 获取文件大小
- **特点**：使用系统调用 `open/write/fsync`，性能高

#### **FileMgr.h/cpp** - 文件管理器（单例）
- **作用**：管理多个日志文件，处理日志轮转
- **核心功能**：
  - `GetFileLog(filename)` - 获取或创建日志文件对象
  - `OnCheck()` - 定期检查并执行日志轮转（按小时/天/分钟）
  - `RotateDays/Hours/Minutes()` - 按不同周期轮转日志
- **特点**：单例模式，统一管理所有日志文件

#### **Logger.h/cpp** - 日志器（单例）
- **作用**：日志系统的核心控制器，管理日志级别
- **核心功能**：
  - `SetLogLevel(level)` - 设置日志级别（TRACE/DEBUG/INFO/WARN/ERROR）
  - `GetLogLevel()` - 获取当前日志级别
  - `Write(msg)` - 写入日志消息到文件
- **特点**：单例模式，全局唯一

#### **LogStream.h/cpp** - 日志流
- **作用**：提供类似 `std::cout` 的流式日志接口
- **核心功能**：
  - 定义了日志宏：`LOG_TRACE`、`LOG_DEBUG`、`LOG_INFO`、`LOG_WARN`、`LOG_ERROR`
  - 自动添加时间戳、线程ID、文件名、行号等信息
  - 支持链式输出：`LOG_INFO << "message" << value;`
- **使用示例**：
  ```cpp
  LOG_INFO << "Application started";
  LOG_DEBUG << "Value: " << 123;
  ```

**日志系统工作流程**：
```
LOG_INFO << "msg" 
  → LogStream 构造（添加时间戳、线程ID等）
  → LogStream 析构时调用 Logger::Write()
  → Logger::Write() 调用 FileLog::WriteLog()
  → FileLog 写入文件并刷新
```

---

### 3. ⚙️ 配置系统 (Configuration System)

#### **Config.h/cpp** - 配置类
- **作用**：解析和管理配置文件（JSON 格式）
- **核心功能**：
  - `LoadConfig(filepath)` - 从 JSON 文件加载配置
  - `ParseInfo()` - 解析日志配置信息
  - `GetLogInFo()` - 获取日志配置（路径、文件名、级别等）
- **配置项**：
  - `name` - 应用名称
  - `cpu_start` - CPU 起始编号
  - `threads` - 线程数
  - `log` - 日志配置（level、name、path、rotate）

#### **ConfigMgr (CnofigMgr)** - 配置管理器（单例）
- **作用**：管理配置对象的生命周期
- **核心功能**：
  - `LoadConfig(filepath)` - 加载配置文件
  - `GetConfigPtr()` - 获取配置对象指针
- **特点**：单例模式，线程安全

---

### 4. ⏰ 任务调度系统 (Task Scheduling System)

#### **Task.h/cpp** - 任务类
- **作用**：封装一个定时任务
- **核心功能**：
  - `Run()` - 执行任务回调函数
  - `Restart()` - 重新设置任务执行时间
  - `When()` - 获取任务应该执行的时间戳
- **特点**：使用回调函数，支持任意任务逻辑

#### **TaskMgr.h/cpp** - 任务管理器（单例）
- **作用**：管理所有定时任务，按时间调度执行
- **核心功能**：
  - `Add(task)` - 添加任务
  - `Del(task)` - 删除任务
  - `DoWork()` - 检查并执行到期的任务
- **工作流程**：
  1. 主循环定期调用 `DoWork()`
  2. `DoWork()` 检查所有任务
  3. 如果任务时间到了（`When() < now`），执行 `Run()`
  4. 任务执行后可以调用 `Restart()` 重新调度
- **特点**：单例模式，线程安全

**使用示例**：
```cpp
auto callback = [](const TaskPtr task) {
    // 执行任务逻辑
    sFileMgr->OnCheck();  // 例如：检查日志轮转
    task->Restart();      // 重新调度
};
TaskPtr task = std::make_shared<Task>(callback, 1000);  // 每1000ms执行一次
sTaskMgr->Add(task);
```

---

## 🔄 模块之间的依赖关系

```
main.cpp
  ├─ ConfigMgr (单例) ──→ Config ──→ JSON 解析
  ├─ FileMgr (单例) ──→ FileLog ──→ 文件操作
  ├─ Logger (单例) ──→ FileLog
  ├─ LogStream ──→ Logger
  ├─ TaskMgr (单例) ──→ Task ──→ TTime
  └─ StringUtils (工具类，被 FileMgr 使用)
```

## 📊 设计模式

1. **单例模式**：`Singleton` 模板，用于 `TaskMgr`、`FileMgr`、`Logger`、`ConfigMgr`
2. **工厂模式**：`FileMgr::GetFileLog()` 创建/获取日志文件对象
3. **观察者模式**：任务回调机制

## 🎯 总结

- **工具类**：提供基础功能（时间、字符串、单例）
- **日志系统**：4层架构，从底层文件操作到上层流式接口
- **配置系统**：JSON 配置解析和管理
- **任务系统**：定时任务调度框架

整个 `base` 模块为上层应用提供了完整的基础设施支持。

