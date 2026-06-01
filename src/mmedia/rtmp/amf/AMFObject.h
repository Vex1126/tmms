#pragma once
#include "AMFAny.h"
#include <vector>
namespace tmms{
    namespace mm{
        using AMFAnyPtr = std::shared_ptr<AMFAny>;
        class AMFObject:public AMFAny{
            public:
                AMFObject(const std::string & name);
                AMFObject();
                ~AMFObject();

                bool IsObject()override;
                AMFObjectPtr Object()override;
                void Dump()const override;
                int Decode(const char * data,int size,bool has=false)override;
                int DecodeOnce(const char * data,int size,bool has=false);     
                const AMFAnyPtr & Property(const std::string &name)const;     
                const AMFAnyPtr & Property(int index); 
            private:
                std::vector<AMFAnyPtr> properties_;

        };        
    }

}