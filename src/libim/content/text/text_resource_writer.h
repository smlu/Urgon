#ifndef LIBIM_TEXT_RESOURCE_WRITER_H
#define LIBIM_TEXT_RESOURCE_WRITER_H
#include "../../math/abstract_vector.h"
#include "../../io/stream.h"
#include "../../utils/utils.h"

#include <cmath>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <vector>

namespace libim::content::text {
    class TextResourceWriter final
    {
    public:
        TextResourceWriter(OutputStream& os);
        TextResourceWriter(const TextResourceWriter&) = delete;
        TextResourceWriter(TextResourceWriter&&) noexcept = delete;
        TextResourceWriter& operator=(const TextResourceWriter&) = delete;
        TextResourceWriter operator=(TextResourceWriter&&) noexcept = delete;



        template<typename T>
        static inline std::size_t getNumberIndent(std::size_t indent, T n)
        {
            static_assert(std::is_arithmetic_v<T>, "T must be arithmetic type");
            indent = indent - (std::signbit(n) ? 1 : 0);

            std::size_t digits = 0ULL;
            if constexpr(std::is_unsigned_v<T>) {
                digits = utils::numdigits(static_cast<uint64_t>(n));
            }
            else {
                digits = utils::numdigits(static_cast<uint64_t>(std::abs(n)));
            }

            if(digits >= indent) {
                return 1;
            }

            return indent - digits;
        }

        std::size_t size() const
        {
            return ostream_.size();
        }

        std::size_t tell() const
        {
            return ostream_.tell();
        }

        template<std::size_t base = 10, std::size_t width = 0, typename T>
        TextResourceWriter& writeEnum(T n)
        {
            return writeNumber<base, width>(utils::to_underlying(n));
        }

        template<std::size_t width = 4, typename T>
        TextResourceWriter& writeFlags(T n)
        {
            return writeEnum<16, width>(n);
        }

        TextResourceWriter& indent(std::size_t width, char indch = ' ');
        TextResourceWriter& write(std::string_view text);
        TextResourceWriter& writeCommentLine(std::string_view comment);
        TextResourceWriter& writeEol();

        TextResourceWriter& writeKeyValue(std::string_view key, std::string_view value, std::size_t indent = 1);

        template<typename T,
            bool isArithmetic = std::is_arithmetic_v<T>,
            typename = std::enable_if_t<isArithmetic || std::is_enum_v<T>>
        >
        TextResourceWriter& writeKeyValue(std::string_view key, T value, std::size_t indent = 1)
        {
            if constexpr(isArithmetic) {
                return writeKeyValue(key, utils::to_string(value), indent);
            }
            else
            {
                return writeKeyValue(key,
                    utils::to_string<16, 4>(utils::to_underlying(value)),
                    indent
                );
            }
        }
        }

        TextResourceWriter& writeLabel(std::string_view name, std::string_view text);
        TextResourceWriter& writeLine(std::string_view line);

        template<typename T, typename Lambda>
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

            const auto list_size = list.size();
            for(std::size_t i = 0; i < list_size; i++)
            {
                writeRow(*this, i, list.at(i));
                writeEol();
            }

            return *this;
        }

        template<std::size_t base = 10, std::size_t precision = 0, typename T>
        TextResourceWriter& writeNumber(T n)
        {
            write(utils::to_string<base, precision, T>(n));
            return *this;
        }

        TextResourceWriter& writeRowIdx(std::size_t idx, std::size_t indent);
        TextResourceWriter& writeSection(std::string_view section);

        template<typename T, std::size_t S, typename Tag>
        TextResourceWriter& writeVector(const AbstractVector<T, S, Tag>& v, std::size_t indent = 4)
        {
            for(const auto e : v)
            {
                this->indent(getNumberIndent(indent, e));
                writeNumber<10, 8>(e);
            }
            return *this;
        }


    private:
        OutputStream& ostream_;
    };
}
#endif // LIBIM_TEXT_RESOURCE_WRITER_H
