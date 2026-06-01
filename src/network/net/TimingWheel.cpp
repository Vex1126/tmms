#include"network/net/TimingWheel.h"
#include"network/base/NetWork.h"
using namespace tmms::network;
TimingWheel::TimingWheel():Wheels_(4){
    Wheels_[kTimingTypeSecond].resize(60);
    Wheels_[kTimingTypeMinute].resize(60);
    Wheels_[kTimingTypeHour].resize(24);
    Wheels_[kTimingTypeDay].resize(30);
}

TimingWheel::~TimingWheel(){

}

void TimingWheel::InsertEntry(uint32_t delay,EntryPtr entry){
    if(delay <= 0){
        entry.reset();
    }else if(delay < kTimingMinute){
        InsertSecondEntry(delay,entry);
    }else if(delay < kTimingHour){
        InsertMinuteEntry(delay,entry);
    }else if(delay < kTimingDay){
        InsertHourEntry(delay,entry);
    }else{
        auto day = delay/kTimingDay;
        if(day > 30){
            NET_ERROR << "day is error";
            return;
        }        
        InsertDayEntry(delay,entry);
    }


}

void TimingWheel::InsertSecondEntry(uint32_t delay,EntryPtr entry){
    Wheels_[kTimingTypeSecond][delay-1].emplace(entry);
}

void TimingWheel::InsertMinuteEntry(uint32_t delay,EntryPtr entry){
    uint32_t minute = delay/kTimingMinute;
    uint32_t second = delay%kTimingMinute;
    CallBackEntryPtr entryptr = std::make_shared<CallBackEntry>([this,second,entry](){
        InsertEntry(second,entry);
    });
    Wheels_[kTimingTypeMinute][minute-1].emplace(entry);
}

void TimingWheel::InsertHourEntry(uint32_t delay,EntryPtr entry){
    uint32_t hour = delay/kTimingHour;
    uint32_t second = delay%kTimingHour;
    CallBackEntryPtr entryptr = std::make_shared<CallBackEntry>([this,second,entry](){
        InsertEntry(second,entry);
    });
    Wheels_[kTimingTypeHour][hour-1].emplace(entry);
}

void TimingWheel::InsertDayEntry(uint32_t delay,EntryPtr entry){
    uint32_t day = delay/kTimingDay;
    uint32_t second = delay%kTimingDay;
    CallBackEntryPtr entryptr = std::make_shared<CallBackEntry>([this,second,entry](){
        InsertEntry(second,entry);
    });
    Wheels_[kTimingTypeDay][day-1].emplace(entry);    
}

void TimingWheel::OnTimer(int64_t now){
    if(last_ts == 0){
        last_ts=now;
    }
    if(now - last_ts < 1000){
        return;
    }
    last_ts=now;
    ++tick_;
    PopUp(Wheels_[kTimingTypeSecond]);
    if(tick_%kTimingMinute == 0){
        PopUp(Wheels_[kTimingTypeMinute]);
    }
    if(tick_%kTimingHour == 0){
        PopUp(Wheels_[kTimingTypeHour]);
    }
    if(tick_%kTimingDay == 0){
        PopUp(Wheels_[kTimingTypeDay]);        
    }
}

void TimingWheel::PopUp(Wheel & bp){
    WheelEntry tmp;
    bp.front().swap(tmp);
    bp.pop_front();
    bp.push_back(WheelEntry());
}

void TimingWheel::RunAfter(double delay,const Func & f){
    CallBackEntryPtr cbentryptr = std::make_shared<CallBackEntry>([f](){
        f();
    });
    InsertEntry(delay,cbentryptr);
}

void TimingWheel::RunEvery(double delay,const Func & f){
    CallBackEntryPtr cbentryptr = std::make_shared<CallBackEntry>([this,f,delay](){
        f();
        RunEvery(delay,f);
    });
    InsertEntry(delay,cbentryptr);
}

void TimingWheel::RunAfter(double delay,Func && f){
    CallBackEntryPtr cbentryptr = std::make_shared<CallBackEntry>([f](){
        f();
    });
    InsertEntry(delay,cbentryptr);
}

void TimingWheel::RunEvery(double delay,Func && f){
    CallBackEntryPtr cbentryptr = std::make_shared<CallBackEntry>([this,f,delay](){
        f();
        RunEvery(delay,f);
    });
    InsertEntry(delay,cbentryptr);
}