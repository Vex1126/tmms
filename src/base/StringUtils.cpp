#include"StringUtils.h"
using namespace tmms::base;
bool StringUtils::StartsWith(const std::string &s,const std::string &sub){
    return s.compare(0,sub.size(),sub);
}

bool StringUtils::EndsWith(const std::string &s,const std::string &sub){
    if(s.size()-sub.size()<0){
        return false;
    }
    return s.compare(s.size()-sub.size(),sub.size(),sub);
}

std::string StringUtils::FilePath(const std::string &path){
    size_t pos=path.find_last_of("/\\");
    if(pos!=std::string::npos){
        return path.substr(0,pos);
    }else{
        return "";
    }
}

std::string StringUtils::FileName(const std::string &path){
    
    std::string file_name=FileNameExt(path);
    size_t pos=path.find_last_of(".");
    if(pos!=std::string::npos){
        if(pos!=0){
            return file_name.substr(0,pos);
        }
    }
    return file_name;
}

std::string StringUtils::FileNameExt(const std::string &path){
    size_t pos=path.find_last_of("/\\");
    if(pos!=std::string::npos){
        if(pos+1<path.size()){
            return path.substr(pos+1);
        }
        return ""; 
    }else{
        return path;
    }
}

std::string StringUtils::Extension(const std::string &path){
    
    std::string file_name=FileNameExt(path);
    size_t pos=path.find_last_of(".");
    if(pos!=std::string::npos){
        if(pos!=0&&pos<file_name.size()){
            return file_name.substr(pos+1);
        }
    }
    return std::string();
}

std::vector<std::string> StringUtils::SplitString(const std::string &s,const std::string &delimter){
    size_t next=0;
    size_t last=0;
    std::vector<std::string> result;
    if(delimter.empty()){
        return result;
    }
    while((next=s.find(delimter,last))!=std::string::npos){
        if(next>last){
            result.emplace_back(s.substr(last,next-last));
        }
        last=delimter.size()+next;
    }
    if(last<s.size()){
        result.emplace_back(s.substr(last));
    }
    return result;
}