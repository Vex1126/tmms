#include<Config.h>
#include<json/json.h>
#include<iostream>
#include<fstream>
using namespace tmms::base;
bool Config::LoadConfig(const std::string & file){
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::ifstream ifs(file);
    std::string errs;
    auto ok=Json::parseFromStream(builder,ifs,&root,&errs);
    if(!ok){
        LOG_ERROR << "config file:" << file <<"parse fila";
        return false;
    }
    Json::Value nameobj=root["name"];
    if(!nameobj.isNull()){
        name = nameobj.asString();        
    }

    Json::Value cupstartoobj=root["cpu_start"];
    if(!cupstartoobj.isNull()){
        cup_start_ = cupstartoobj.asInt();        
    }

    Json::Value threadnumobj=root["threads"];
    if(!threadnumobj.isNull()){
        thread_num_ = threadnumobj.asInt();        
    }

    Json::Value log=root["log"];
    if(!log.isNull()){
        ParseInfo(log);
    }
    return true;
}

bool Config::ParseInfo(const Json::Value & root){
    if(!log_info_){
        log_info_ = std::make_shared<LogInfo>();
    }
    Json::Value levelObj=root["level"];
    if(!levelObj.isNull()){
        std::string level=levelObj.asString(); 
        if(level == "TRACE"){
            log_info_->level = kTrace;
        }else if(level == "DEBUG"){
            log_info_->level = kDebug;
        }else if(level == "INFO"){
            log_info_->level = kInfo;
        }else if(level == "ERROR"){
            log_info_->level = kError;
        }else if(level == "WARN"){
            log_info_->level = kWarn;       
        }
    }

    Json::Value rotateObj=root["rotate"];
    if(!rotateObj.isNull()){
        std::string rotate=rotateObj.asString(); 
        if(rotate == "HOUR"){
            log_info_->rotate_type_ = kRotateHour;
        }else if(rotate == "DAY"){
            log_info_->rotate_type_ = kRotateDay;
        }
    }

    Json::Value namelObj=root["name"];
    if(!namelObj.isNull()){
        log_info_->name = namelObj.asString();
    }

    Json::Value pathlObj=root["path"];
    if(!pathlObj.isNull()){
        log_info_->path = pathlObj.asString();
    }

    return true;
}

LogInfoPtr& Config::GetLogInFo(){
    return log_info_;
}

bool CnofigMgr::LoadConfig(const std::string &file){
    ConfigPtr config=std::make_shared<Config>();
    if(config->LoadConfig(file)){
        std::lock_guard<std::mutex> lock(lock_);
        config_ = config;
        return true;
    }
    return false;
}

ConfigPtr CnofigMgr::GetConfigPtr(){
    std::lock_guard<std::mutex> lock(lock_);
    return config_;
}