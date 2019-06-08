#ifndef LIBIM_TEXT_RESOURCE_WRITER_H
#define LIBIM_TEXT_RESOURCE_WRITER_H
#include "../../math/abstract_vector.h"
#include "../../math/box.h"
#include "../../io/stream.h"
#include "../../utils/traits.h"
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

            std::size_t digits = 0;
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

        TextResourceWriter& indent(std::size_t width, char indch);
        TextResourceWriter& indent(std::size_t width);
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
                constexpr std::size_t p = std::is_floating_point_v<T> ? 6 : 0;
                return writeKeyValue(key, utils::to_string<10, p>(value), indent);
            }
            else
            {
                return writeKeyValue(key,
                    utils::to_string<16, 4>(utils::to_underlying(value)),
                    indent
                );
            }
        }

        template<typename T, std::size_t S, typename Tag>
        TextResourceWriter& writeKeyValue(std::string_view key, const AbstractVector<T, S, Tag>& v, std::size_t indent = 0)
        {
            write(key);
            this->indent(indent);
            writeVector(v, 1);
            return writeEol();
        }

        template<typename T, std::size_t N>
        TextResourceWriter& writeKeyValue(std::string_view key, const Box<T, N>& box, std::size_t indent = 0)
        {
            write(key);
            this->indent(indent);
            writeVector(box.v0, 1);
            writeVector(box.v0, 1);
            return writeEol();
        }


        TextResourceWriter& writeLabel(std::string_view name, std::string_view text);
        TextResourceWriter& writeLine(std::string_view line);

        template<bool writeListSize = true,
                 typename Container,
                 typename Lambda,
                 class = utils::requires_container<Container>>
        TextResourceWriter& writeList([[maybe_unused]] std::string_view name, const Container& list, Lambda&& writeRow)
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

            if constexpr (writeListSize)
            {
                using ListSizeT = typename Container::size_type;
                writeKeyValue<ListSizeT>(name, list.size());
                writeEol();
            }


            for(auto[i, v] : utils::enumerate(list))
            {
                writeRow(*this, i, v);
                writeEol();
            }

            if constexpr (!writeListSize)
            {
                write("end");
                writeEol();
            }

            return *this;
        }

        template<typename Container,
                 typename Lambda,
                 class = utils::requires_container<Container>>
        TextResourceWriter& writeList(const Container& list, Lambda&& rowWriter) {
            return writeList<false>("", list, std::forward<Lambda>(rowWriter));
        }

        template<std::size_t base = 10,
                 std::size_t precision = 0,
                 typename T,
                 typename DT = std::decay_t<T>>
        TextResourceWriter& writeNumber(T n)
        {
            static_assert (std::is_arithmetic_v<DT>, "T must be an arithmetic type!");
            write(utils::to_string<base, precision, DT>(n));
            return *this;
        }

        TextResourceWriter& writeRowIdx(std::size_t idx, std::size_t indent);
        TextResourceWriter& writeSection(std::string_view section, bool overline = true);

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

        void setIndentCh(char ch)
        {
            indch_ = ch;
        }

        char indentCh() const
        {
            return indch_;
        }

    private:
        OutputStream& ostream_;
        char indch_ = ' ';
    };
}
#endif // LIBIM_TEXT_RESOURCE_WRITER_H
