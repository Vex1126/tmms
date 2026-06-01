#pragma once
#include"Singleton.h"
#include"FileLog.h"
#include<string>
namespace tmms{
    namespace base{
        enum LogLevel{
            kTrace,
            kDebug,
            kInfo,
            kWarn,
            kError,
            kMaxNumOfLoglevel
        };

        class Logger:public Singleton<Logger>{
            friend class Singleton<Logger>;
            public:
                void SetLogLevel(const LogLevel & level);
                LogLevel GetLogLevel()const;
                void Write(const std::string &msg);
                Logger(const FileLogPtr &log);
            private:
                ~Logger()=default;
                LogLevel level_{kDebug};
                FileLogPtr log_;
        };
    }
}