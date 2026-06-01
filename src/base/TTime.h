#pragma once
#include<cstdint>
#include<stdio.h>
#include<string>
#include<sys/time.h>
namespace tmms{
    namespace base{
        class TTime{
            public:
                static int64_t NowMs();
                static int64_t Now();
                static int64_t Now(int &year,int &mouth,int &day,int &hour,int &minute,int &second);
                static std::string ISOTime();            
        };
    }
}