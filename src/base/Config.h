#pragma once
#include<memory>
#include<cstdint>
#include<iostream>
#include<string>
#include<memory>
#include<mutex>
#include"Singleton.h"
#include"../../third_party/jsoncpp/include/json/json.h"
#include"LogStream.h"
#include"Logger.h"
namespace tmms{
    namespace base{
        struct LogInfo
        {
            std::string name;
            LogLevel level;
            std::string path;
            RotateType rotate_type_{kRotateNone};
        };
        using LogInfoPtr = std::shared_ptr<LogInfo>;
        class Config{
            public:
                Config()=default;
                ~Config()=default;
                bool LoadConfig(const std::string & file);
                bool ParseInfo(const Json::Value & root);
                LogInfoPtr& GetLogInFo();
            private:
                LogInfoPtr log_info_;
                std::string name;
                int32_t cup_start_{0};
                int32_t thread_num_{1};
        };

        using ConfigPtr = std::shared_ptr<Config>;
        class CnofigMgr:public Singleton<CnofigMgr>{
            public:
                CnofigMgr()=default;
                ~CnofigMgr()=default;
                bool LoadConfig(const std::string &file);
                ConfigPtr GetConfigPtr();
            private:
                ConfigPtr config_;
                std::mutex lock_;
        };
        #define sConfigMgr tmms::base::Singleton<tmms::base::CnofigMgr>::Instance()
    }
}

