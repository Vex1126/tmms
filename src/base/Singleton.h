#pragma once
#include <utility>
namespace tmms{
    namespace base{
        template <typename T>
        class Singleton{
            public:
                template<typename... Args>
                static T * Instance(Args&&...args){
                    static T instance(std::forward<Args>(args)...);
                    return & instance;
                }
            protected:
                Singleton()=default;
                ~Singleton()=default;
            private:
                Singleton(const Singleton &)=delete;
                Singleton & operator=(const Singleton &)=delete;
        };
    }
}