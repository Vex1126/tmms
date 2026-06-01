#pragma once
#include "AMFAny.h"
namespace tmms{
    namespace mm{
        class AMFNumber:public AMFAny{
            public:
                AMFNumber(const std::string & name);
                AMFNumber();
                ~AMFNumber();

                bool IsNumber()override;
                double Number()override;
                void Dump()const override;
                int Decode(const char * data,int size,bool has=false)override;
            private:
                double number_{0.0f};
        };        
    }

}