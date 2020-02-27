#ifndef CNDTOOLARGS_H
#define CNDTOOLARGS_H
#include <stdexcept>
#include <libim/common.h>
#include <libim/utils/utils.h>
#include <cmdutils/options.h>

class CndToolArgs : public CmdArgs
{
public:
    CndToolArgs(std::size_t argc, const char *argv[])
    {
        parse(argc, argv, [&](auto tp, auto t, auto a){
            return parseToken(tp, t, a);
        });
    }

    const std::string& cmd() const
    {
        return cmd_;
    }

    const std::string& subcmd() const
    {
        return scmd_;
    }

    const std::string& cndFile() const
    {
        return cndfile_;
    }

private:
    std::string_view parseToken(std::size_t tokenPos, std::string_view token, std::string_view arg)
    {
        if(libim::FileExtMatch(token, ".cnd"))
        {
            if(!cndfile_.empty()) {
                throw std::invalid_argument("CND file path already set");
            }
            cndfile_ = token;
        }
        else if(tokenPos == 1)
        {
            cmd_ = token;
        }
        else if (tokenPos == 2 &&
                !(token.compare(0, 1, "-") == 0 || token.compare(0, 2, "--") == 0))
        {
            scmd_ = token;
        }
        else {
            return CmdArgs::parseToken(tokenPos, token, arg);
        }

        return "";
    }

private:
    std::string cmd_;
    std::string scmd_;
    std::string cndfile_;
};

#endif // CNDTOOLARGS_H
