#include"FileLog.h"
#include"StringUtils.h"
#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<errno.h>
#include<iostream>
#include<string>
using namespace tmms::base;

// 递归创建目录
static bool CreateDirectoryRecursive(const std::string& dir_path) {
    if(dir_path.empty()) {
        return true;
    }
    
    // 检查目录是否已存在
    struct stat st;
    if(stat(dir_path.c_str(), &st) == 0) {
        if(S_ISDIR(st.st_mode)) {
            return true; // 目录已存在
        }
        return false; // 路径存在但不是目录
    }
    
    // 创建父目录
    size_t pos = dir_path.find_last_of("/\\");
    if(pos != std::string::npos && pos > 0) {
        std::string parent_dir = dir_path.substr(0, pos);
        if(!CreateDirectoryRecursive(parent_dir)) {
            return false;
        }
    }
    
    // 创建当前目录
    if(mkdir(dir_path.c_str(), 0755) != 0) {
        // 如果目录已存在（可能被其他进程创建），也算成功
        if(errno != EEXIST) {
            return false;
        }
    }
    return true;
}

bool FileLog::Open(const std::string &filepath){
    file_path = filepath;
    
    // 提取目录路径并创建目录
    std::string dir_path = StringUtils::FilePath(file_path);
    if(!dir_path.empty() && !CreateDirectoryRecursive(dir_path)) {
        std::cout << "create directory failed: " << dir_path << std::endl;
        return false;
    }
    
    int fd = open(file_path.c_str(), O_CREAT | O_APPEND | O_WRONLY ,0644);
    if(fd<0){
        std::cout << "open file error!!!" << std::endl;
        return false;
    }
    fd_=fd;
    return true;
}

size_t FileLog::WriteLog(const std::string & msg){
    int fd = fd_ == -1?1:fd_;
    size_t written = write(fd,msg.data(),msg.size());
    // 确保数据写入磁盘（对于日志文件很重要）
    if(fd != 1 && fd != 2) {  // 不是标准输出/错误
        fsync(fd);
    }
    return written;
}

void FileLog::Rotate(const std::string &file){
    if(file.empty()){
        return ;
    }
    if(fd_ != -1){
        ::close(fd_);
        fd_ = -1;
    }
    int ret=::rename(file_path.c_str(),file.c_str());
    if(ret!=0){
        std::cout<<"rename failed: " << file_path << " -> " << file << std::endl;
        return;
    }
    int fd = open(file_path.c_str(), O_CREAT | O_APPEND | O_WRONLY ,0644);
    if(fd<0){ 
        std::cout << "open file log path failed: " << file_path << std::endl;
        return ;
    } 
    ::dup2(fd,fd_);
    ::close(fd);
    return ;
}

void FileLog::SetRotate(RotateType type){
    rotate_type_=type;
}

RotateType FileLog::GetRotateType() const{
    return rotate_type_;
}

int64_t FileLog::FileSize() const{
    return ::lseek(fd_,0,SEEK_END);
}

std::string FileLog::FilePath(){
    return file_path;
}