#include<iostream>
#include"base/Config.h"
#include"base/TaskMgr.h"
#include"base/FileMgr.h"
#include"base/LogStream.h"
#include<thread>
int main(int argc, char const *argv[])
{
    std::cout<<"hello world"<<std::endl;
    if(!sConfigMgr->LoadConfig("../Config/config.json")){
        std::cerr << "load config file failed" << std::endl;
        return -1;
    }
    LogInfoPtr log_info_=sConfigMgr->GetConfigPtr()->GetLogInFo();
    std::cout << "leave : " << log_info_->level << " "
              << "name : "  << log_info_->name << " "
              << "path : "  << log_info_->path << " "
              << "name : "  << log_info_->rotate_type_ << std::endl;
    FileLogPtr log = sFileMgr->GetFileLog(log_info_->path+log_info_->name);
    if(!log){
        std::cerr << "log create failed" << std::endl;
        return -1;
    }
    log->SetRotate(log_info_->rotate_type_);

    tmms::base::g_logger = tmms::base::Logger::Instance(log);
    g_logger->SetLogLevel(log_info_->level);
    
    
        auto callback4=[](const TaskPtr task){
        sFileMgr->OnCheck();
        task->Restart();
    };
    TaskPtr task4=std::make_shared<Task>(callback4,1000);
        sTaskMgr->Add(task4);
    while(true){
        sTaskMgr->DoWork();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    return 0;
}
