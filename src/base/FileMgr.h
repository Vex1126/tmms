#pragma once 
#include"Singleton.h"
#include "FileLog.h"
#include<string>
#include<memory>
#include<unordered_map>
#include<mutex>
using namespace tmms::base;
namespace tmms{
    namespace base{
        class FileMgr:public Singleton<FileMgr>{
            friend class Singleton<FileMgr>;
            public:
                void OnCheck();
                FileLogPtr GetFileLog(const std::string & file_name);
                void RemoteFileLog(const FileLogPtr & filelogptr);
                void RotateDays(const FileLogPtr & file);
                void RotateHours(const FileLogPtr & file);
                void RotateMinutes(const FileLogPtr & file);
            private:
                std::unordered_map<std::string,FileLogPtr> log_;
                std::mutex lock_;
                int last_day_{-1};
                int last_hour_{-1};
                int last_month_{-1};
                int last_year_{-1};
                int last_minute_{-1};
                FileMgr()=default;
                ~FileMgr()=default;
        };    
    }
}
#define sFileMgr tmms::base::Singleton<tmms::base::FileMgr>::Instance()