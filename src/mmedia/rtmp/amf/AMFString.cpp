#include"AMFString.h"
#include"mmedia/base/BytesReader.h"
using namespace tmms::mm;
AMFString::AMFString(const std::string & name):AMFAny(name){

}

AMFString::AMFString(){

}

AMFString::~AMFString(){

}

bool AMFString::IsString(){
    return true;
}

const std::string & AMFString::String(){
    return string_;
}

void AMFString::Dump()const{
    RTMP_TRACE <<"String :"<<string_<<std::endl;
}

int AMFString::Decode(const char * data,int size,bool has){
    if(size<2){
        return -1;
    }
    auto len = BytesReader::ReadUint16T(data);
    if(len<0||size<len+2){
        return -1;
    }
    string_ = DeCodeString(data);
    return len+2;
}