#ifndef CNDTOOL_CNDTOOLARGS_H
#define CNDTOOL_CNDTOOLARGS_H
#include <filesystem>
#include <stdexcept>

#include <libim/common.h>
#include <libim/utils/utils.h>
#include <cmdutils/options.h>

namespace cndtool {
    class CndToolArgs : public cmdutils::CmdArgs
    {
    public:
        CndToolArgs(std::size_t argc, const char *argv[])
        {
            parse(argc, argv, [&](auto tp, auto t){
                parseToken(tp, t);
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

        const std::filesystem::path& cndFile() const
        {
            return cndfile_;
        }

    private:
        void parseToken(std::size_t tokenPos, std::string_view token)
        {
            if(libim::fileExtMatch(token, ".cnd"))
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
                CmdArgs::parseToken(tokenPos, token);
            }
        }

    private:
        std::string cmd_;
        std::string scmd_;
        std::filesystem::path cndfile_;
    };
}
#endif // CNDTOOL_CNDTOOLARGS_H