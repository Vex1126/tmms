#pragma once
#include "AMFAny.h"
namespace tmms{
    namespace mm{
        class AMFBoolean:public AMFAny{
            public:
                AMFBoolean(const std::string & name);
                AMFBoolean();
                ~AMFBoolean();

                bool IsBoolean()override;
                bool Boolean()override;
                void Dump()const override;
                int Decode(const char * data,int size,bool has=false)override;
            private:
                bool b_{false};

        };        
    }

}