#include<string>
#include<vector>
namespace tmms{
    namespace base{
        class StringUtils{
            public:
                static bool StartsWith(const std::string &s,const std::string &sub);
                static bool EndsWith(const std::string &s,const std::string &sub);
                static std::string FilePath(const std::string &path);
                static std::string FileName(const std::string &path);
                static std::string FileNameExt(const std::string &path);
                static std::string Extension(const std::string &path);
                static std::vector<std::string> SplitString(const std::string &s,const std::string &delimter);
        };
    }
}