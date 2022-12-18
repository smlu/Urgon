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

        const std::filesystem::path& ndyFile() const
        {
            return ndyfile_;
        }

    private:
        void parseToken(std::size_t tokenPos, std::string_view token)
        {
            if(tokenPos == 1)
            {
                cmd_ = token;
                return;
            }
            else if (tokenPos == 2 &&
                    !(token.compare(0, 1, "-") == 0 || token.compare(0, 2, "--") == 0))
            {
                scmd_ = token;
                return;
            }
            else if(positionalArgs().empty()) // try parsing cnd or ndy file paths
            {
                if(libim::fileExtMatch(token, ".cnd"))
                {
                    if (!cndfile_.empty()) {
                        throw std::invalid_argument("Multiple CND file paths");
                    }
                    cndfile_ = token;
                    return;
                }
                else if(libim::fileExtMatch(token, ".ndy"))
                {
                    if (!ndyfile_.empty()) {
                        throw std::invalid_argument("Multiple NDY file paths");
                    }
                    ndyfile_ = token;
                    return;
                }
            }

            // No match call default parse func
            CmdArgs::parseToken(tokenPos, token);
        }

    private:
        std::string cmd_;
        std::string scmd_;
        std::filesystem::path cndfile_;
        std::filesystem::path ndyfile_;
    };
}
#endif // CNDTOOL_CNDTOOLARGS_H