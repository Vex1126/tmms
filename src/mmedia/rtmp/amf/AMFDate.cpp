#include"AMFDate.h"
#include"mmedia/base/BytesReader.h"
using namespace tmms::mm;
AMFDate::AMFDate(const std::string & name):AMFAny(name){

}

AMFDate::AMFDate(){

}

AMFDate::~AMFDate(){

}

bool AMFDate::IsDate(){
    return true;
}

double AMFDate::Date(){
    return utc_;
}

void AMFDate::Dump()const{
    RTMP_TRACE <<"Date :"<<utc_<<std::endl;
}

int AMFDate::Decode(const char * data,int size,bool has){
    if(size<10){
        return -1;
    }

    utc_ = BytesReader::ReadUint64T(data);
    data += 8;
    utc_offset_ = BytesReader::ReadUint16T(data);
    return 10;
}