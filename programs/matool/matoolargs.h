#ifndef MATOOL_MATOOLARGS_H
#define MATOOL_MATOOLARGS_H
#include <stdexcept>
#include <filesystem>
#include <vector>

#include <cmdutils/options.h>
#include <libim/common.h>
#include <libim/utils/utils.h>



namespace matool {
    namespace fs = std::filesystem;
    class MatoolArgs : public cmdutils::CmdArgs
    {
    public:
        MatoolArgs(std::size_t argc, const char *argv[])
        {
            parse(argc, argv, [&](auto tp, auto t){
                parseToken(tp, t);
            });
        }

        const std::string& cmd() const
        {
            return cmd_;
        }

        /**
         * Returns sub-command argument which is
         * equal to positionalArgs().at(0).
         */
        std::string subcmd() const
        {
            return positionalArgs().empty() ? "" : positionalArgs().at(0);
        }

        const fs::path& matFile() const
        {
            return matfile_;
        }

        const std::vector<fs::path>& imageFiles() const
        {
            return imgfiles_;
        }

    private:
        void parseToken(std::size_t tokenPos, std::string_view token)
        {
            if(libim::fileExtMatch(token, ".mat"))
            {
                if(!matfile_.empty()) {
                    throw std::invalid_argument("Only one MAT file path should be set");
                }
                matfile_ = token;
            }
            if(libim::fileExtMatch(token, ".png") || libim::fileExtMatch(token, ".bmp"))
            {
                auto file = std::string(token);
                imgfiles_.push_back(file);
            }
            else if(tokenPos == 1)
            {
                cmd_ = token;
            }
            else {
                CmdArgs::parseToken(tokenPos, token);
            }
        }

    private:
        fs::path matfile_;
        std::vector<fs::path> imgfiles_;
        std::string cmd_;
    };
}

#endif // MATOOL_MATOOLARGS_H