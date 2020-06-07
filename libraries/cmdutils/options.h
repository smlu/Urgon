#ifndef CMDUTILS_OPTIONS_H
#define CMDUTILS_OPTIONS_H
#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_map>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

class CmdArgs
{
protected:
    constexpr static std::string_view ARG_UNSPECIFIED_KEY = "_unspecified_";
    CmdArgs() = default;

public:
    CmdArgs(std::size_t argc, const char *argv[])
    {
        parse(argc, argv, [&](auto tp, auto t, auto a){
            return parseToken(tp, t, a);
        });
    }

    std::size_t count() const
    {
        return m_args.size();
    }

    bool hasArg(const std::string& argKey) const
    {
        return m_args.find(argKey) != m_args.end();
    }

    bool hasArg(std::string_view argKey) const
    {
        return m_args.find(std::string(argKey)) != m_args.end();
    }

    bool hasArg(const char* argKey) const
    {
        return m_args.find(argKey) != m_args.end();
    }

    std::vector<std::string> args(const char* argKey) const
    {
        return args(std::string(argKey));
    }

    std::vector<std::string> args(std::string_view argKey) const
    {
        return args(std::string(argKey));
    }

    std::vector<std::string> args(const std::string& argKey) const
    {
        std::vector<std::string> args;
        auto it = m_args.find(argKey);
        if(it != m_args.end()){
            args = it->second;
        }

        return args;
    }

    std::string arg(const char* argKey) const
    {
        return arg(std::string(argKey));
    }

    std::string arg(std::string_view argKey) const
    {
        return arg(std::string(argKey));
    }

    std::string arg(const std::string& argKey) const
    {
        auto arg = args(argKey);
        if(!arg.empty()) {
            return arg.at(0);
        }
        return "";
    }

    std::vector<std::string> unspecified() const
    {
        return args(ARG_UNSPECIFIED_KEY);
    }

    const std::string& exePath() const
    {
        return m_exePath;
    }

protected:
    template<typename Lambda>
    void parse(std::size_t argc, const char *argv[], Lambda&& pf)
    {
        std::string_view token;
        std::string_view arg;
        for(std::size_t i = 0; i < argc; i++)
        {
            token = argv[i];
            arg = pf(i, token, arg);
        }
    }

    std::string_view parseToken(std::size_t tokenPos, std::string_view token, std::string_view arg)
    {
        if(tokenPos == 0) {
            m_exePath = token;
        }
        else if(token.compare(0, 1, "-") == 0 || token.compare(0, 2, "--") == 0)
        {
            auto s = libim::utils::split(token, "=", 1);
            auto v = s.size() > 1 ? std::vector<std::string>{ std::string(s.at(1)) } : std::vector<std::string>();
            m_args.emplace(s.at(0), std::move(v));
            return s.at(0);
        }
        else if(!arg.empty()) {
            m_args.at(std::string(arg)).emplace_back(token);
        }
        else  {
            m_args[std::string(ARG_UNSPECIFIED_KEY)].emplace_back(token);
        }

        return "";
    }

protected:
    std::string m_exePath;
    std::unordered_map<std::string,
        std::vector<std::string>> m_args;
};

#endif // CMDUTILS_OPTIONS_H
