#pragma once
#include <string>
#include <cstdint>
#include <memory>
#include "mmedia/base/MMediaLog.h"
namespace tmms{
    namespace mm{
        enum AMFDataType
        {
            kAMFNumber = 0,
            kAMFBoolean,
            kAMFString,
            kAMFObject,
            kAMFMovieClip,
            kAMFNull,
            kAMFUndefined,
            kAMFReference,
            kAMFEcmaArray,
            kAMFObjectEnd,
            kAMFStrictArray,
            kAMFDate,
            kAMFLongString,
            kAMFUnsupported,
            kAMFRecordset,
            kAMFXMLDoc,
            kAMFTypedObject,
            kAMFAvmplus,
            kAMFInvalid = 0xff,
        };
        class AMFObject;
        using AMFObjectPtr = std::shared_ptr<AMFObject>;
        class AMFAny:public std::enable_shared_from_this<AMFAny>{
            public:
                AMFAny(const std::string &name);
                AMFAny();
                virtual~AMFAny();

                virtual int Decode(const char * data,int size,bool has=false)=0;
                virtual const std::string &String();
                virtual double Number(){
                    if(this->IsNumber()){
                        return this->Number();
                    }
                    RTMP_ERROR <<"not a number.";
                    return 0.0f;    
                }
                virtual bool Boolean();
                virtual double Date();
                virtual AMFObjectPtr Object();

                virtual bool IsString();
                virtual bool IsBoolean();
                virtual bool IsNumber();
                virtual bool IsDate();
                virtual bool IsObject();

                virtual void Dump()const=0;
                virtual const std::string & Name()const;
                virtual int32_t Count()const;
                static int32_t EncodeNumber(char* output,double val);
                static int32_t EncodeString(char* output,const std::string& str);
                static int32_t EncodeBoolean(char* output,const bool b);
                static int32_t EncodeNamedNumber(char* output,const std::string& name,double bval);
                static int32_t EncodeNamedString(char* output,const std::string& name,std::string val);
                static int32_t EncodeNamedBoolean(char* output,const std::string& name,bool val);
            protected:
                static int EncodeName(char *buf, const std::string &name);
                static int WriteNumber(char* output,double val);
                static std::string DeCodeString(const char *data);
                
            private:
                std::string name_;

        };
    }
}