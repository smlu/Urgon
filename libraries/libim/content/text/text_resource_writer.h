#ifndef LIBIM_TEXT_RESOURCE_WRITER_H
#define LIBIM_TEXT_RESOURCE_WRITER_H
#include "gradientcolor.h"

#include <libim/content/asset/primitives/box.h>
#include <libim/content/asset/thing/movement/pathinfo.h>
#include <libim/io/binarystream.h>
#include <libim/io/stream.h>
#include <libim/math/abstract_vector.h>
#include <libim/utils/traits.h>
#include <libim/utils/utils.h>
#include <libim/types/flags.h>

#include <array>
#include <cmath>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <vector>

namespace libim::content::text {
    class TextResourceWriter final
    {
    public:

        /**
         * Constructs new TextResourceWriter object.
         * @param os - The reference to OutputStream object to write to.
         */
        TextResourceWriter(OutputStream& os);

        TextResourceWriter(const TextResourceWriter&) = delete;
        TextResourceWriter(TextResourceWriter&&) noexcept = delete;
        TextResourceWriter& operator=(const TextResourceWriter&) = delete;
        TextResourceWriter operator=(TextResourceWriter&&) noexcept = delete;

        /**
         * Returns required indentation width reduced for the number width (i.e. the width of string representation of the number).
         * @param indent - The required indentation width.
         * @param n - Number.
         * @return The Reduced required indentation width.
         */
        template<typename T>
        static inline std::size_t getNumberIndent(std::size_t indent, T n)
        {
            static_assert(std::is_arithmetic_v<T>, "T must be arithmetic type");
            indent = indent - (std::signbit(n) ? 1 : 0);

            std::size_t digits = 0;
            if constexpr (std::is_unsigned_v<T>) {
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

        /** Returns the size of stream. */
        std::size_t size() const
        {
            return ostream_.size();
        }

        /**
         * Returns the current stream position.
        */
        std::size_t tell() const
        {
            return ostream_.tell();
        }

        /**
         * Writes enum of Flag object to the text stream.
         * @tparam base - Numeric base. By default 10.
         * @tparam width - Width of formatted string enum.  i.e. if 4 and base=16, 0x1 will be written as 0x00000001.
         * @param e - Enum object to write.
         * @return Reference to this object.
         */
        template<std::size_t base = 10, std::size_t width = 0, typename T>
        TextResourceWriter& writeEnum(T e)
        {
            return writeNumber<base, width>(utils::to_underlying(e));
        }

        /**
         * Writes enum or Flag object to text string as hex flag.
         * @tparam width - Width of formatted string flags.  i.e. if 4, 0x1 will be written as 0x00000001.
         * @param flags - Flags to write.
         * @return Reference to this object.
         */
        template<std::size_t width = 4, typename T>
        TextResourceWriter& writeFlags(T flags)
        {
            return writeEnum<16, width>(flags);
        }

        /**
         * Writes indentation char line to text stream.
         * @param width - The indent line width.
         * @param ch - The indentation char.
         * @return Reference to this.
         */
        TextResourceWriter& indent(std::size_t width, char indch);

        /**
         * Writes indentChar() line to text stream.
         * @param width - The indent line width.
         * @return Reference to this.
         */
        TextResourceWriter& indent(std::size_t width);

        /**
         * Writes string text to text stream
         * @param text - Text to write.
         * @return Reference to this object.
         */
        TextResourceWriter& write(std::string_view text);

        /**
         * Writes string text padded with indentation character to text stream.
         * @param text - Text to write.
         * @param fieldWidth - Field width.
         * @param minSep - Minimum separation between text and field width.
         * @param indentChar - Indentation character.
         * @return Reference to this.
         */
        TextResourceWriter& write(std::string_view text, std::size_t fieldWidth, std::size_t minSep = 1, char indentChar = ' ');

        /**
         * Writes comment line with EOL to text stream.
         * Format: # <comment...>
         * @param comment -  Comment text format.
         * @param args -  Comment arguments.
         * @return Reference to this.
         */
        template<typename ...Args>
        TextResourceWriter& writeCommentLine(std::string_view comment, Args&&... args)
        {
            if (!comment.empty())
            {
                std::ostringstream ss;
                ss << commentChar() << spaceChar();
                utils::ssprintf(ss, comment, std::forward<Args&&>(args)...);
                ostream_ << ss.view();
                writeEol();
            }
            return *this;
        }

        /**
         * Writes end of line char to text stream.
         * @return Reference to this object.
         */
        TextResourceWriter& writeEol();

        /**
         * @brief Writes GradientColor object to text stream in format: "(r/g/b/a/r/g/b/a/r/g/b/a/r/g/b/a)"
         *
         * @param color - The GradientColor object to write.
         * @return Reference to this object.
         */
        TextResourceWriter& writeGradientColor(const GradientColor& color);

        /**
         * Writes string key-value pair to text stream.
         * @note EOL is put at the end of the value.
         *
         * @param key - The key string.
         * @param value - The value string.
         * @param indent - The indent width between key and value.
         * @return Reference to this object.
         */
        TextResourceWriter& writeKeyValue(std::string_view key, std::string_view value, std::size_t indent = 1);

        /**
         * Writes string key and numeric or enum value to text stream.
         * @note EOL is put at the end of the value.
         *
         * @tparam precision - The precision of float or hex (enum) number.
         * @param key - The key string.
         * @param value - The numeric or enum value. The enum value is written as hex number.
         * @param indent - The indent width between key and value.
         * @return Reference to this object.
         */
        template<std::size_t precision = 0,
            typename T,
            bool isArithmetic = std::is_arithmetic_v<T>,
            typename = std::enable_if_t<isArithmetic || utils::isEnum<T>>
        >
        TextResourceWriter& writeKeyValue(std::string_view key, T value, std::size_t indent = 1)
        {
            constexpr std::size_t p = [&](){
                // Set default precision for float and enum type
                if constexpr(precision == 0 &&
                    (std::is_floating_point_v<T> || utils::isEnum<T>))
                {
                    if constexpr (std::is_floating_point_v<T>) {
                        return std::size_t(6);
                    }
                    else {
                        return std::size_t(4);
                    }
                }
                return precision;
            }();

            if constexpr(isArithmetic) {
                return writeKeyValue(key, utils::to_string<10, p>(value), indent);
            }
            else
            {
                return writeKeyValue(key,
                    utils::to_string<16, p>(utils::to_underlying(value)),
                    indent
                );
            }
        }

        /**
         * Writes string key and numeric std::array to text stream.
         * @note EOL is put at the end of the value.
         *
         * @param key - The key string.
         * @param value - The numeric std::array.
         * @param indent - The indent width between key and value.
         * @return Reference to this object.
         */
        template<typename T, std::size_t S, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
        TextResourceWriter& writeKeyValue(std::string_view key, const std::array<T, S>& a, std::size_t indent = 0)
        {
            write(key);
            this->indent(indent);
            writeNumericArray(a, 1);
            return writeEol();
        }

        /** Writes string key and Box object to stream.
         * @note EOL is put at the end of the value.
         *
         * @param key - The key string.
         * @param box - The Box object to write.
         * @param indent - The indent width between key and value.
         * @return Reference to this object.
         */
        template<typename T, std::size_t N>
        TextResourceWriter& writeKeyValue(std::string_view key, const asset::Box<T, N>& box, std::size_t indent = 0)
        {
            write(key);
            this->indent(indent);
            writeVector(box.min, 1);
            writeVector(box.max, 1);
            return writeEol();
        }

        /**
         * Writes label to stream in format: <name>:<indentChar()><text><eol>
         * @note EOL is put at the end of the value.
         *
         * @param label - The label name.
         * @param text  - The label text.
         * @return Reference to this object.
         */
        TextResourceWriter& writeLabel(std::string_view name, std::string_view text);

        /**
         * Writes line with EOL to text stream.
         * @param line - The text line to write.
         * @return Reference to this object.
         */
        TextResourceWriter& writeLine(std::string_view line);

        /**
         * Write container as list to text stream.
         *
         * @tparam writeListSize - If true, write list size before list items.
         * @tparam lbAfterSize   - If true, write line break after list size.
         * @tparam writeEnd      - writeEnd - If true the "end" keyword is written at the end of the list.
         * @tparam Container     - Container type.
         * @tparam RowWriteFunc  - The type of function to write a single row of the list.
         *
         * @param name - List name.
         * @param list - Container to write.
         * @param writeRow - Lambda function to write a single row of the list.
        */
        template<bool writeListSize,
                 bool lbAfterSize,
                 bool writeEnd,
                 typename Container,
                 typename RowWriteFunc,
                 class = utils::requires_container<Container>>
        TextResourceWriter& writeList([[maybe_unused]] std::string_view name, const Container& list, RowWriteFunc&& writeRow)
        {
            static_assert(std::is_invocable_v<RowWriteFunc, TextResourceWriter&, std::size_t, const typename Container::value_type&>,
                "writeRow function must be invocable with arguments: (TextResourceWriter&, std::size_t rowIdx, const ElementType& element)"
            );

            if constexpr (writeListSize)
            {
                writeKeyValue(name, list.size());
                if constexpr(lbAfterSize) {
                    writeEol();
                }
            }

            for (auto[i, v] : utils::enumerate(list))
            {
                auto pos = tell();
                writeRow(*this, i, v); // TODO: detect via function trait the return type of row writer function.
                                       //       The return type can only be bool or void.
                                       //       Based on te return type write EOL, e.g:
                                       //       if void always write EOL, if bool write EOL only when true is returned.

                if(tell() > pos) { // write eol if pos has changed.
                    writeEol();
                }
            }

            if constexpr (writeEnd && !writeListSize)
            {
                write("end");
                writeEol();
            }

            return *this;
        }

        /**
         * Overload function for writing container list with list size.
         *
         * @tparam lbAfterSize - If true the line brake is written after list size is written. Default true.
         *
         * @param name - Name of the list.
         * @param list - Container to write.
         * @param writeRow - Lambda function to write a single row of the list.
         * @return Reference to this object.
         **/
        template<bool lbAfterSize = true,
                 typename Container,
                 typename Lambda,
                 class = utils::requires_container<Container>>
        TextResourceWriter& writeList(std::string_view name, const Container& list, Lambda&& rowWriter) {
            return writeList</*writeListSize=*/true, lbAfterSize, false>(name, list, std::forward<Lambda>(rowWriter));
        }

        /**
         * Overload method for writing container list without list size.
         *
         * @tparam writeEnd - If true the "end" keyword is written at the end of the list. Default true.
         *
         * @param list - Container to write.
         * @param writeRow - Lambda function to write a single row of the list.
         * @return Reference to this object.
         **/
        template<bool writeEnd = true,
                 typename Container,
                 typename Lambda,
                 class = utils::requires_container<Container>>
        TextResourceWriter& writeList(const Container& list, Lambda&& rowWriter) {
            return writeList</*writeListSize=*/false, false, writeEnd>("", list, std::forward<Lambda>(rowWriter));
        }

         /**
          * Writes number to text string.
          * @tparam base - The number base.
          * @tparam precision - The hex and float number precision.
          *
          * @param n - The number to write.
          * @return Reference to this object.
          */
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

        /**
         * Writes numeric std::array to text stream.
         *
         * @tparam precision - The float number precision.
         * @param array      - The array object to write.
         * @param width      - The string size of each array element. If string representation of element
         *                    is smaller than the width then the element is left padded with indentChar().
         *                    By default 4.
         * @param separator  - The separator between array elements. By default not used.
         * @return Reference to this object.
         */
        template< std::size_t precision = 8, typename T, std::size_t S, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
        TextResourceWriter& writeNumericArray(const std::array<T, S>& array, std::size_t width = 4, char separator = 0)
        {
            for (std::size_t i = 0; i < S; i++)
            {
                if (i > 0 && separator != 0) {
                    write(std::string_view(&separator, 1));
                }
                auto e = array[i];
                if (width > 0 || separator == 0) {
                    this->indent(getNumberIndent(width, e));
                }
                writeNumber<10, precision>(e);
            }
            return *this;
        }

        /**
         * Writes PathFrame object to text stream in format: "(x/y/z:p/y/r)".
         * @param frame - The PathFrame object to write.
         * @return Reference to this object.
         */
        TextResourceWriter& writePathFrame(const asset::PathFrame& frame);

        /**
         * Write row index to text stream.
         * Format: <indent><idx>:
         *
         * @param idx - Row index.
         * @param indent - Left side indentation width.
         * @return Reference to this.
         */
        TextResourceWriter& writeRowIdx(std::size_t idx, std::size_t indent);

        /** Writes resource section start to text stream.
         *  ```Format: SECTION:<indentChar><section>
         *     If overline == True:
         *       ```#######################
         *          SECTION: some_section```
         *       ```
         *
         * @param section - The section name.
         * @param overline - If true, write section overline separator. By default true.
         * @return Reference to this object.
        */
        TextResourceWriter& writeSection(std::string_view section, bool overline = true);

        /**
         * Writes AbstractVector to text stream.
         *
         * @tparam strict - If true, write vector in format: "(x/y/z...)".
         * @tparam T - Vector element type.
         * @tparam S - Vector size.
         *
         * @param vec   - Vector to write.
         * @param width - The string size of each vector element. If string representation of element
         *                is smaller than the width then the element is left padded with indentChar().
         *                Not used if strict is true. By default 4.
         *
         * @return Reference to this object.
         */
        template<bool strict = false, typename T, std::size_t S, typename Tag>
        inline TextResourceWriter& writeVector(const AbstractVector<T, S, Tag>& vec, std::size_t width = 4)
        {
            if constexpr(strict)
            {
                write("(");
                writeNumericArray</*precision=*/6>(vec, /*width=*/0, /*separator=*/ '/');
                write(")");

            }
            else {
                writeNumericArray(vec, width);
            }
            return *this;
        }

        /**
         * Set indentation character.
         * @param ch - The indentation character.
         */
        void setIndentChar(char ch)
        {
            indch_ = ch;
        }

        /**
         * Get indentation character.
         * @return The indentation character.
         */
        char indentChar() const
        {
            return indch_;
        }

    private:
        char commentChar() const;
        char spaceChar() const;

    private:
        OutputStream& ostream_;
        char indch_ = ' ';
    };
}

namespace libim {
    template<typename T, std::size_t S, typename Tag>
    std::string AbstractVector<T,S,Tag>::toString() const
    {
        using namespace libim::content::text;
        std::string vecstr;
        vecstr.reserve(
            S * 10  + /* S * max float num char len */
            (S - 1) + /* S - 1 fwd. slashes or spaces*/
            2         /* 2 parentheses */
        );

        OutputBinaryStream ostream(vecstr);
        TextResourceWriter wr(ostream);
        wr.writeVector</*strict=*/true>(*this);
        return vecstr;
    }
}
#endif // LIBIM_TEXT_RESOURCE_WRITER_H
