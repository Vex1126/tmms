# TMMS

TMMS 是一个用 C++ 编写的流媒体服务器学习项目，当前代码包含基础设施模块、网络模块和 RTMP 多媒体模块。项目使用 CMake 构建，并通过 jsoncpp 读取 JSON 配置。

## 功能概览

- 基础模块：配置读取、日志系统、定时任务、文件管理和字符串工具。
- 网络模块：事件循环、TCP/UDP 服务端与客户端、连接管理、DNS 服务和线程池。
- 多媒体模块：RTMP 握手、RTMP 上下文、AMF 编解码和基础包结构。
- 主程序：加载配置、初始化日志，并周期性执行日志轮转检查。

## 目录结构

```text
.
├── CMakeLists.txt              # 顶层 CMake 构建入口
├── bin/
│   ├── Config/config.json      # 默认运行配置
│   ├── log/                    # 默认日志输出目录
│   └── sbin/                   # 安装后的可执行文件目录
├── src/
│   ├── base/                   # 配置、日志、任务、文件和通用工具
│   ├── main/                   # 主程序入口
│   ├── mmedia/                 # RTMP/AMF 等多媒体协议代码
│   └── network/                # 网络事件循环、TCP/UDP 封装
└── third_party/jsoncpp/        # jsoncpp 第三方依赖
```

## 构建

推荐在项目根目录外或 `build` 目录中构建：

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

如果需要安装到项目内的 `bin` 目录：

```bash
cmake --install build
```

默认安装前缀在顶层 CMake 中设置为 `../bin`，因此从 `build` 目录安装时，可执行文件会进入项目的 `bin` 相关目录。

## 运行

主程序会从相对路径 `../Config/config.json` 加载配置。因此建议进入安装后的可执行文件目录运行：

```bash
cd bin/sbin
./tmms
```

如果直接运行构建目录中的可执行文件，需要确保当前工作目录下的 `../Config/config.json` 能被正确找到，或者调整配置加载路径。

## 配置文件

默认配置位于：

```text
bin/Config/config.json
```

示例：

```json
{
  "name": "tmms_server",
  "cpu_start": 0,
  "threads": 4,
  "log": {
    "level": "DEBUG",
    "name": "tmms.log",
    "path": "../log/"
  }
}
```

配置项说明：

- `name`：服务名称。
- `cpu_start`：CPU 起始编号，预留给线程绑定等功能使用。
- `threads`：线程数配置。
- `log.level`：日志级别，例如 `DEBUG`。
- `log.name`：日志文件名。
- `log.path`：日志输出目录。

## 开发说明

- 代码使用 C++11。
- 构建时会编译 `third_party/jsoncpp` 作为静态依赖。
- `src/base/README.md` 中有基础模块的更详细说明。
- 当前工作区如果存在未提交代码，请提交时只暂存相关文件，避免把临时构建产物或无关修改混入提交。

## 常用命令

```bash
# 查看工作区状态
git status --short

# 重新构建
cmake --build build --parallel

# 运行主程序
cd bin/sbin && ./tmms
```
