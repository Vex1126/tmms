#pragma once
#include "AMFAny.h"
namespace tmms{
    namespace mm{
        class AMFString:public AMFAny{
            public:
                AMFString(const std::string & name);
                AMFString();
                ~AMFString();

                bool IsString()override;
                const std::string & String()override;
                void Dump()const override;
                int Decode(const char * data,int size,bool has=false)override;
            private:
                std::string string_;
        };        
    }

}