#include"LogStream.h"
#include<string.h>
#include<unistd.h>
#include<sys/syscall.h>
#include<iostream>
#include"TTime.h"
using namespace tmms::base;
namespace tmms{
    namespace base{
        Logger *g_logger=nullptr;
    }
}
const char * log_string[]={
    "TARCE",
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR"
};
static thread_local pid_t thread_t=0;

LogStream::LogStream(Logger *logger,const char * file,int line,LogLevel l,const char * fun):logger_(logger){
    const char * file_name=strrchr(file,'/');
    if(file_name){
        file_name=file_name+1;
    }else{
        file_name=file;
    }

    stream_ << TTime::ISOTime() <<" ";
    if(thread_t==0){
        thread_t=static_cast<pid_t>(syscall(SYS_gettid));
    }
    stream_ << thread_t <<" ";
    stream_ << log_string[l];
    stream_ << "[" << file_name << ":" << line << "]" ;
    if(fun){
        stream_ << "[" << fun << "]" ;
    }
}

LogStream::~LogStream(){
    stream_ << "\n";
    if(logger_){
           logger_ ->Write(stream_.str()); 
    }else{
        std::cout<<stream_.str()<<std::endl;
    }

}