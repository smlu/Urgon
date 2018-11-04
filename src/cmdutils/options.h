#ifndef OPTIONS_H
#define OPTIONS_H
#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_map>
#include <string>
#include <utility>
#include <vector>


class Options
{
    constexpr static const char* OPT_UNSPECIFIED_KEY = "_unspecified_";
public:
    Options(uint32_t argc, const char *argv[])
    {
        std::string token;
        std::string option;
        for(uint32_t i = 0; i < argc; i++)
        {
            token = argv[i];
            if(i == 0) {
                m_exePath = std::move(token);
            }
            else if(strncmp(token.c_str(), "-", 1) == 0 || strncmp(token.c_str(), "--", 2) == 0 )
            {
                m_options.emplace(token, std::vector<std::string>());
                option = token;
            }
            else if(!option.empty()) {
                m_options.at(option).emplace_back(std::move(token));
            }
            else
            {
                m_options[OPT_UNSPECIFIED_KEY].emplace_back(std::move(token));
                option.erase();
                token.erase();
            }
        }
    }

    std::size_t count() const
    {
        return m_options.size();
    }

    bool hasOpt(const std::string& arg) const
    {
        return m_options.find(arg) != m_options.end();
    }

    bool hasOpt(const char* arg) const
    {
        return m_options.find(arg) != m_options.end();
    }

    std::vector<std::string> args(const std::string& opt) const
    {
        std::vector<std::string> args;
        auto it = m_options.find(opt);
        if(it != m_options.end()){
            args = it->second;
        }

        return args;
    }

    std::string arg(const std::string& opt) const
    {
        auto arg = args(opt);
        if(!arg.empty()) {
            return arg.at(0);
        }

        return "";
    }

    std::vector<std::string> unspecified() const
    {
        return args(OPT_UNSPECIFIED_KEY);
    }

    const std::string& exePath() const
    {
        return m_exePath;
    }

private:
    std::string m_exePath;
    std::unordered_map<std::string,
        std::vector<std::string>> m_options;
};

#endif // OPTIONS_H
