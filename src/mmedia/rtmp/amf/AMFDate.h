#pragma once
#include "AMFAny.h"
namespace tmms{
    namespace mm{
        class AMFDate:public AMFAny{
            public:
                AMFDate(const std::string & name);
                AMFDate();
                ~AMFDate();

                bool IsDate()override;
                double Date()override;
                void Dump()const override;
                int Decode(const char * data,int size,bool has=false)override;
            private:
                double utc_{0.0f};
                int16_t utc_offset_{0};
        };        
    }

}