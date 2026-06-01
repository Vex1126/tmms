#include"AMFObject.h"
#include"mmedia/base/BytesReader.h"
#include"AMFString.h"
#include"AMFNumber.h"
#include"AMFLongString.h"
#include"AMFDate.h"
#include"AMFBoolean.h"
#include"AMFAny.h"
using namespace tmms::mm;
namespace{
    AMFAnyPtr AnyPtr_Null;
}
AMFObject::AMFObject(const std::string & name):AMFAny(name){

}

AMFObject::AMFObject(){

}

AMFObject::~AMFObject(){

}

bool AMFObject::IsObject(){
    return true;
}

AMFObjectPtr AMFObject::Object(){
    return std::dynamic_pointer_cast<AMFObject>(shared_from_this());
}

void AMFObject::Dump()const{
    RTMP_TRACE <<"Object Start" << std::endl;
    for(auto &p:properties_){
        p->Dump();
    }
    RTMP_TRACE <<"Object End" << std::endl;
}

int AMFObject::Decode(const char * data,int size,bool has){
    std::string nname;
    int32_t parse=0;
    RTMP_TRACE << "Decode Object" << std::endl;
    while((parse+3)<=size){
        if(BytesReader::ReadUint24T(data)==0x000009){
            parse+=3;
            return parse;
        }
        if(has){
            nname = DeCodeString(data);
            parse+=(nname.size()+2);
            data+=(nname.size()+2);
        }
        char type = *data++;
        parse++;
        switch(type){
            case kAMFNumber:{
                std::shared_ptr<AMFNumber> p = std::make_shared<AMFNumber>(nname);
                auto len = p->Decode(data,size-parse);
                if(len<0){
                    return -1;
                }
                parse+=len;
                data+=len;
                RTMP_TRACE <<"Number value:"<<p->Number()<<std::endl;
                properties_.emplace_back(std::move(p));
                break;
            }
            case kAMFBoolean:{
                std::shared_ptr<AMFBoolean> p = std::make_shared<AMFBoolean>(nname);
                auto len = p->Decode(data,size-parse);
                if(len<0){
                    return -1;
                }
                parse+=len;
                data+=len;
                RTMP_TRACE <<"Boolean value:"<<p->Boolean()<<std::endl;
                properties_.emplace_back(std::move(p));
                break;                    
            }
            case kAMFString:{
                std::shared_ptr<AMFString> p = std::make_shared<AMFString>(nname);
                auto len = p->Decode(data,size-parse);
                if(len<0){
                    return -1;
                }
                parse+=len;
                data+=len;
                RTMP_TRACE <<"String value:"<<p->String()<<std::endl;
                properties_.emplace_back(std::move(p));
                break; 
            }
            case kAMFObject:{
                std::shared_ptr<AMFObject> p = std::make_shared<AMFObject>(nname);
                auto len = p->Decode(data,size-parse,true);
                if(len<0){
                    return -1;
                }
                parse+=len;
                data+=len;
                RTMP_TRACE <<"Object:"<<std::endl;
                p->Dump();
                properties_.emplace_back(std::move(p));
                break;                     
            }
            case kAMFNull:{
                RTMP_TRACE <<"Null."<<std::endl;
                break;
            }
            case kAMFEcmaArray:{
                int count = BytesReader::ReadUint32T(data);
                parse+=4;
                data+=4;
                std::shared_ptr<AMFObject> p = std::make_shared<AMFObject>(nname);
                auto len = p->Decode(data,size-parse,true);
                if(len<0){
                    return -1;
                }
                parse+=len;
                data+=len;
                RTMP_TRACE <<"EcmaArray"<<std::endl;
                p->Dump();
                properties_.emplace_back(std::move(p));
                break; 
            }
            case kAMFObjectEnd:{
                return parse;
            }
            case kAMFStrictArray:{
                int count = BytesReader::ReadUint32T(data);
                parse+=4;
                data+=4;   
                std::shared_ptr<AMFObject> p = std::make_shared<AMFObject>(nname);                                     
                while(count>0){
                    auto len = p->DecodeOnce(data,size-parse,true);
                    if(len<0){
                        return -1;
                    }
                    parse+=len;
                    data+=len;
                    count--;
                }
                RTMP_TRACE <<"StrictArray"<<std::endl;
                p->Dump();
                properties_.emplace_back(std::move(p));
                break; 
            }
            case kAMFDate:{
                std::shared_ptr<AMFDate> p = std::make_shared<AMFDate>(nname);
                auto len = p->Decode(data,size-parse,true);
                if(len<0){
                    return -1;
                }
                parse+=len;
                data+=len;
                RTMP_TRACE <<"Date:"<<p->Date()<<std::endl;
                p->Dump();
                properties_.emplace_back(std::move(p));
                break;                      
            }
            case kAMFLongString:{
                std::shared_ptr<AMFLongString> p = std::make_shared<AMFLongString>(nname);
                auto len = p->Decode(data,size-parse);
                if(len<0){
                    return -1;
                }
                parse+=len;
                data+=len;
                RTMP_TRACE <<"LoogString value:"<<p->String()<<std::endl;
                properties_.emplace_back(std::move(p));
                break;                     
            }
            case kAMFMovieClip:
            case kAMFUndefined:
            case kAMFReference:
            case kAMFUnsupported:
            case kAMFRecordset:
            case kAMFXMLDoc:
            case kAMFTypedObject:
            case kAMFAvmplus:{
                RTMP_TRACE <<"no support"<<std::endl;
                break;                    
            }
        }
    }
    return parse;
}

int AMFObject::DecodeOnce(const char * data,int size,bool has){
        std::string nname;
        int32_t parse=0;

        if(has){
            nname = DeCodeString(data);
                parse+=(nname.size()+2);
                data+=(nname.size()+2);
        }
        char type = *data++;
            parse++;
            switch(type){
                case kAMFNumber:{
                    std::shared_ptr<AMFNumber> p = std::make_shared<AMFNumber>(nname);
                    auto len = p->Decode(data,size-parse);
                    if(len<0){
                        return -1;
                    }
                    parse+=len;
                    data+=len;
                    RTMP_TRACE <<"Number value:"<<p->Number()<<std::endl;
                    properties_.emplace_back(std::move(p));
                    break;
                }
                case kAMFBoolean:{
                    std::shared_ptr<AMFBoolean> p = std::make_shared<AMFBoolean>(nname);
                    auto len = p->Decode(data,size-parse);
                    if(len<0){
                        return -1;
                    }
                    parse+=len;
                    data+=len;
                    RTMP_TRACE <<"Boolean value:"<<p->Boolean()<<std::endl          ;
                    properties_.emplace_back(std::move(p));
                    break;                    
                }
                case kAMFString:{
                    std::shared_ptr<AMFString> p = std::make_shared<AMFString>(nname);
                    auto len = p->Decode(data,size-parse);
                    if(len<0){
                        return -1;
                    }
                    parse+=len;
                    data+=len;
                    RTMP_TRACE <<"String value:"<<p->String()<<std::endl;
                    properties_.emplace_back(std::move(p));
                    break; 
                }
                case kAMFObject:{
                    std::shared_ptr<AMFObject> p = std::make_shared<AMFObject>(nname);
                    auto len = p->Decode(data,size-parse,true);
                    if(len<0){
                        return -1;
                    }
                    parse+=len;
                    data+=len;
                    RTMP_TRACE <<"Object:"<<std::endl;
                    p->Dump();
                    properties_.emplace_back(std::move(p));
                    break;                     
                }
                case kAMFNull:{
                    RTMP_TRACE <<"Null."<<std::endl;
                    break;
                }
                case kAMFEcmaArray:{
                    int count = BytesReader::ReadUint32T(data);
                    parse+=4;
                    data+=4;
                    std::shared_ptr<AMFObject> p = std::make_shared<AMFObject>(nname);
                    auto len = p->Decode(data,size-parse,true);
                    if(len<0){
                        return -1;
                    }
                    parse+=len;
                    data+=len;
                    RTMP_TRACE <<"EcmaArray"<<std::endl;
                    p->Dump();
                    properties_.emplace_back(std::move(p));
                    break; 
                }
                case kAMFObjectEnd:{
                    return parse;
                }
                case kAMFStrictArray:{
                    int count = BytesReader::ReadUint32T(data);
                    parse+=4;
                    data+=4;   
                    std::shared_ptr<AMFObject> p = std::make_shared<AMFObject>(nname);                                     
                    while(count>0){
                        auto len = p->DecodeOnce(data,size-parse,true);
                        if(len<0){
                            return -1;
                        }
                        parse+=len;
                        data+=len;
                        count--;
                    }
                    RTMP_TRACE <<"StrictArray"<<std::endl;
                    p->Dump();
                    properties_.emplace_back(std::move(p));
                    break; 
                }
                case kAMFDate:{
                    std::shared_ptr<AMFDate> p = std::make_shared<AMFDate>(nname);
                    auto len = p->Decode(data,size-parse,true);
                    if(len<0){
                        return -1;
                    }
                    parse+=len;
                    data+=len;
                    RTMP_TRACE <<"Date:"<<p->Date()<<std::endl;
                    p->Dump();
                    properties_.emplace_back(std::move(p));
                    break;                      
                }
                case kAMFLongString:{
                    std::shared_ptr<AMFLongString> p = std::make_shared<AMFLongString>(nname);
                    auto len = p->Decode(data,size-parse);
                    if(len<0){
                        return -1;
                    }
                    parse+=len;
                    data+=len;
                    RTMP_TRACE <<"LoogString value:"<<p->String()<<std::endl;
                    properties_.emplace_back(std::move(p));
                    break;                     
                }
                case kAMFMovieClip:
                case kAMFUndefined:
                case kAMFReference:
                case kAMFUnsupported:
                case kAMFRecordset:
                case kAMFXMLDoc:
                case kAMFTypedObject:
                case kAMFAvmplus:{
                    RTMP_TRACE <<"no support"<<std::endl;
                    break;                    
                }
            }
    return parse;
}   

const AMFAnyPtr & AMFObject::Property(const std::string &name)const{
    for(const auto &p :properties_){
        if(p->Name()==name){
            return p;
        }else if(p->IsObject()){
            AMFObjectPtr obj = p->Object();
            const AMFAnyPtr& p2 = obj->Property(name);
            if(p2){
                return p2;
            }
        }
    }
    return AnyPtr_Null;
} 

const AMFAnyPtr & AMFObject::Property(int index){
    if(index<0 || index>=properties_.size()){
        return AnyPtr_Null;
    }
    return properties_[index];
} 