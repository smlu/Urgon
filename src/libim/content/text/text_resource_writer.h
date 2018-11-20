#ifndef LIBIM_TEXT_RESOURCE_WRITER_H
#define LIBIM_TEXT_RESOURCE_WRITER_H
#include "../../io/stream.h"
#include "../../utils/utils.h"

#include <iomanip>
#include <sstream>
#include <string_view>
#include <vector>
#include <type_traits>

namespace libim::content::text {
    class TextResourceWriter final
    {
    public:
        TextResourceWriter(OutputStream& os);
        TextResourceWriter(const TextResourceWriter&) = delete;
        TextResourceWriter(TextResourceWriter&&) noexcept = delete;
        TextResourceWriter& operator=(const TextResourceWriter&) = delete;
        TextResourceWriter operator=(TextResourceWriter&&) noexcept = delete;

        std::size_t size() const
        {
            return ostream_.size();
        }

        std::size_t tell() const
        {
            return ostream_.tell();
        }

        template<std::size_t width = 4, typename T>
        TextResourceWriter& writeFlag(T n)
        {
            return writeNumber<16, width>(utils::to_underlying(n));
        }

        TextResourceWriter& indent(std::size_t width, char indch = ' ');
        TextResourceWriter& write(std::string_view text);
        TextResourceWriter& writeCommentLine(std::string_view comment);
        TextResourceWriter& writeEol();

        template<typename T,
            bool isArithmetic = std::is_arithmetic_v<T>,
            bool isEnum       = std::is_enum_v<T>,
            typename Value = std::conditional_t<isArithmetic || isEnum, T, std::string_view>
        >
        TextResourceWriter& writeKeyValue(std::string_view key, Value value)
        {
            if constexpr(isArithmetic) {
                return writeKey(key, convertToString(value));
            } else if constexpr(isEnum)
            {
                return writeKey(key,
                    convertToString<16, 4>(to_underlying(value))
                );
            }
            else {
                return writeKey(key, value);
            }
        }

        TextResourceWriter& writeLabel(std::string_view name, std::string_view text);
        TextResourceWriter& writeLine(std::string_view line);

        template<bool writeRowIdxs = true, typename T, typename Lambda>
        TextResourceWriter& writeList(std::string_view name, const std::vector<T>& list, Lambda&& writeRow)
        {
            /*TODO: Uncomment when static reflection is available and decltype is avaliable for generic lambdas.

            using LambdaTriats = typename utils::function_traits<Lambda>;
            static_assert(LambdaTriats::arity == 3, "constructor func must have 3 arguments");
            static_assert(std::is_same_v<typename LambdaTriats::template arg_t<0>,
                TextResourceReader&>, "first arg in writeRow must be of a type TextResourceWriter&"
            );

            static_assert(std::is_same_v<typename LambdaTriats::template arg_t<1>,
                std::size_t>, "second arg in writeRow must be of a type std::size_t"
            );

            static_assert(std::is_same_v<typename LambdaTriats::template arg_t<2>,
                T&>, "third arg in writeRow must be of a type T&"
            );
            */

            using ListSizeT = typename std::decay<decltype(list)>::type::size_type;
            writeKeyValue<ListSizeT>(name, list.size());
            writeEol();

            for(std::size_t i = 0; i < list.size(); i++)
            {
                startNewRow(i, writeRowIdxs);
                writeRow(*this, i, list.at(i));
                writeEol();
            }

            return *this;
        }

        template<std::size_t base = 10, std::size_t width = 0, typename T>
        TextResourceWriter& writeNumber(T n)
        {
            write(convertToString<base, width, T>(n));
            return *this;
        }

        TextResourceWriter& writeSection(std::string_view section);

    private:
        template<std::size_t base = 10, int width = 0, typename T>
        static std::string convertToString(T n)
        {
            static_assert(base == 8 || base == 10 || base == 16, "invalid encoding base");
            static_assert(std::is_arithmetic_v<T>, "T is not a arithmetic type");
            static_assert(!std::is_floating_point_v<T> || base == 10,
                "floating point can be only represented in base 10"
            );

            std::stringstream ss;
            ss.exceptions(std::ios::failbit);


            if constexpr(base == 8) {
                ss << std::oct << std::showbase;
            }
            else if constexpr (base == 10) {
                ss << std::dec;
            }
            else
            {
                ss << "0x"
                   << std::uppercase
                   << std::hex;
            }

            if constexpr(width != 0 || std::is_floating_point_v<T>)
            {
                const auto w = width != 0 ? width : 4; // default floating point width = 4
                ss << std::setw(w)
                   << std::setfill('0')
                   << std::fixed
                   << std::setprecision(w);
            }

            ss << n;
            return ss.str();
        }

        TextResourceWriter& startNewRow(std::size_t idx, bool writeRowIdx);
        TextResourceWriter& writeKey(std::string_view key, std::string_view value, std::size_t indent = 1);
        TextResourceWriter& writeRowIdx(std::size_t idx);

    private:
        OutputStream& ostream_;
    };
}
#endif // LIBIM_TEXT_RESOURCE_WRITER_H
