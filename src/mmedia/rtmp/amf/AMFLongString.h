#pragma once
#include "AMFAny.h"
namespace tmms{
    namespace mm{
        class AMFLongString:public AMFAny{
            public:
                AMFLongString(const std::string & name);
                AMFLongString();
                ~AMFLongString();

                bool IsString()override;
                const std::string & String()override;
                void Dump()const override;
                int Decode(const char * data,int size,bool has=false)override;
            private:
                std::string string_;
        };        
    }

}