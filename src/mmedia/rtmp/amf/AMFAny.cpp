#include "mmedia/rtmp/amf/AMFAny.h"
#include "mmedia/base/BytesReader.h"
#include "mmedia/base/BytesWriter.h"
#include <cstring>
#include <arpa/inet.h>
using namespace tmms::mm;
namespace{
    std::string str_null;
}
AMFAny::AMFAny(const std::string &name):name_(name){

}

AMFAny::AMFAny(){

}

AMFAny::~AMFAny(){

}

const std::string &AMFAny::String(){
    if(this->IsString()){
        return this->String();
    }
    RTMP_ERROR <<"not a string.";
    return str_null;
}

bool AMFAny::Boolean(){
    if(this->IsString()){
        return this->Boolean();
    }
    RTMP_ERROR <<"not a bool.";
    return false;
}

double AMFAny::Date(){
    if(this->IsString()){
        return this->Date();
    }
    RTMP_ERROR <<"not a data.";
    return 0.0f;
}

AMFObjectPtr AMFAny::Object(){
    if(this->IsObject()){
        return this->Object();
    }
    RTMP_ERROR <<"not a object.";
    return AMFObjectPtr();
}

bool AMFAny::IsString(){
    return false;
}

bool AMFAny::IsBoolean(){
    return false;
}

bool AMFAny::IsNumber(){
    return false;
}

bool AMFAny::IsDate(){
    return false;
}

bool AMFAny::IsObject(){
    return false;
}

const std::string & AMFAny::Name()const{
    return name_;
}

int32_t AMFAny::Count()const{
    return 1;
}   

std::string AMFAny::DeCodeString(const char *data){
    auto len = BytesReader::ReadUint16T(data);
    if(len > 0){
        std::string str(data+2,len);
        return str;
    }
    return std::string();
}   
int AMFAny::WriteNumber(char *buf, double value)
{
    uint64_t res;
    uint64_t in;

    memcpy(&in, &value, sizeof(double));

    res = __bswap_64(in);
    memcpy(buf, &res, 8);

    return 8;
}

int32_t AMFAny::EncodeNumber(char* output,double val){
    char * p = output;
    *p++ = kAMFNumber;
    p += WriteNumber(p,val);
    return p-output;
}

int32_t AMFAny::EncodeString(char* output,const std::string& str){
    char * p = output;
    auto len = str.size();
    *p++ = kAMFString;
    p += BytesWriter::WriteUint16T(p,str.size());
    memcpy(p,str.c_str(),len);
    p+=len;
    return p-output;   
}
                
int32_t AMFAny::EncodeBoolean(char* output,const bool b){
    char * p = output;
    *p++ = kAMFBoolean;
    *p++ = b?0x01:0x00;
    return p-output;
}

int AMFAny::EncodeName(char *buf, const std::string &name)
{
    char *old = buf;
    auto len = name.size();
    unsigned short length = htons(len);
    memcpy(buf, &length, 2);
    buf += 2;

    memcpy(buf, name.c_str(), len);
    buf += len;

    return len + 2;
}

int32_t AMFAny::EncodeNamedNumber(char* output,const std::string& name,double bval){
    char * old = output;
    output+=EncodeName(output,name);
    output+=EncodeNumber(output,bval);
    return output - old;
}

int32_t AMFAny::EncodeNamedString(char* output,const std::string& name,std::string val){
    char * old = output;
    output+=EncodeName(output,name);
    output+=EncodeString(output,val);
    return output - old;   
}

int32_t AMFAny::EncodeNamedBoolean(char* output,const std::string& name,bool val){
    char * old = output;
    output+=EncodeName(output,name);
    output+=EncodeBoolean(output,val);
    return output - old;      
}



