#pragma once
#include<string>
#include<memory>
namespace tmms{
    namespace base{
        class FileLog;
        using FileLogPtr = std::shared_ptr<FileLog>;
        enum RotateType{
            kRotateNone,
            kRotateHour,
            kRotateDay,
            kRotateMinute
        };
        class FileLog{
            public:
                FileLog() = default;
                ~FileLog() = default;

                bool Open(const std::string &filepath);
                size_t WriteLog(const std::string & msg);
                void Rotate(const std::string &file);
                void SetRotate(RotateType type);
                RotateType GetRotateType() const;
                int64_t FileSize() const;
                std::string FilePath();
            private:
                int fd_{-1};
                std::string file_path;
                RotateType rotate_type_{kRotateNone};
        };
    }
}