#pragma once
#include<cstdint>
#include<deque>
#include<unordered_set>
#include<vector>
#include<memory>
#include<functional>
namespace tmms{
    namespace network{
        class CallBackEntry;
        using EntryPtr = std::shared_ptr<void>;
        using WheelEntry = std::unordered_set<EntryPtr>;
        using Wheel = std::deque<WheelEntry>;
        using Wheels = std::vector<Wheel>;
        using Func = std::function<void()>;

        enum TimingType{
            kTimingTypeSecond=0,
            kTimingTypeMinute=1,
            kTimingTypeHour=2,
            kTimingTypeDay=3
        };

        const int kTimingMinute = 60; 
        const int kTimingHour = 60*60; 
        const int kTimingDay = 60*60*24; 
        class CallBackEntry{
            public: 
                CallBackEntry(const Func & cb):cb_(cb){};
                ~CallBackEntry(){
                    if(cb_){
                        cb_();
                    }
                };
            private:
                Func cb_;
        };
        using CallBackEntryPtr = std::shared_ptr<CallBackEntry>;
        class TimingWheel{
            public:
                TimingWheel();
                ~TimingWheel();
                void InsertEntry(uint32_t delag,EntryPtr entry);
                void OnTimer(int64_t now);
                void PopUp(Wheel & bp);
                void RunAfter(double delay,const Func & f);
                void RunEvery(double delay,const Func & f);
                void RunAfter(double delay,Func && f);
                void RunEvery(double delay,Func && f);                
            private:
                void InsertSecondEntry(uint32_t delag,EntryPtr entry);
                void InsertMinuteEntry(uint32_t delag,EntryPtr entry);
                void InsertHourEntry(uint32_t delag,EntryPtr entry);
                void InsertDayEntry(uint32_t delag,EntryPtr entry);
                Wheels Wheels_;
                int64_t last_ts{0};
                uint64_t tick_{0};
        };
    }
}