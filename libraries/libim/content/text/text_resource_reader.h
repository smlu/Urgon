#ifndef LIBIM_RESOURCE_READER_H
#define LIBIM_RESOURCE_READER_H
#include "gradientcolor.h"

#include <libim/content/asset/primitives/box.h>
#include <libim/content/asset/thing/movement/pathinfo.h>
#include <libim/log/log.h>
#include <libim/io/binarystream.h>
#include <libim/math/abstract_vector.h>
#include <libim/text/tokenizer.h>
#include <libim/types/flags.h>
#include <libim/utils/traits.h>
#include <libim/utils/utils.h>

#include <assert.h>
#include <cstdint>
#include <string_view>
#include <vector>
#include <type_traits>

namespace libim::content::text {
    using namespace libim::text;

    class TextResourceReader final : public Tokenizer
    {
    public:
        using Tokenizer::Tokenizer;

        /**
         * Asserts that the next token is expected label.
         * Label in stream must be in format: "label:".
         *
         * @param label - The expected label.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If label doesn't match.
        */
        void assertLabel(std::string_view label);

        /**
         * Asserts that the next token is expected key.
         * @param key - The expected key.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If key doesn't match.
        */
        void assertKey(std::string_view key);

        /**
         * Asserts that the next tokens are expected key and value.
         * @tparam T - the type of value.
         *
         * @param key - The expected key.
         * @param v   - The expected value.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If either key or value don't match.
        */
        template<typename T>
        void assertKeyValue(std::string_view key, T v)
        {
            using namespace std::string_view_literals;
            using DT =  typename std::decay_t<T>;
            if constexpr (utils::isEnum<DT>){
                return assertKeyValue(key, utils::to_underlying(v));
            }

            bool bValid = false;
            auto rv = readKey<DT>(key);
            if constexpr(isVector<DT> || std::is_arithmetic_v<DT>) {
                bValid = (v = rv);
            }
            else {
                bValid = utils::iequal(rv, v);
            }

            if(!bValid)
            {
                LOG_DEBUG("assertKey: expected value '%', found '%'", v, cachedTkn_.value());
                throw SyntaxError("invalid value"sv, cachedTkn_.location());
            }
        }

        /**
         * Asserts that the next line is expected section.
         * Line in stream must be in format: "SECTION section".
         *
         * @param section - The expected section.
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If section doesn't match.
        */
        void assertSection(std::string_view section);

        /**
         * Reads std::array from text stream
         * @tparam T - the type of array elements.
         * @tparam S - The number of elements in array.
         * @param parseFunc - function that parses single element of the array.
         * @return std::array object.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw If parseFunc throws an exception.
         */
        template<typename T, size_t S, typename ElemParserF>
        std::array<T, S> readArray(ElemParserF&& parseFunc)
        {
            std::array<T, S> array;
            for (std::size_t i = 0; i < array.size(); i++) {
                array[i] = parseFunc(i, *this);
            }
            return array;
        }

        /**
         * Reads flag number from text stream.
         * @tparam T - Type of the flag number.
         * @return Flag number.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If the token is not a flag number.
         */

        template<typename T, typename DT = typename std::decay_t<T>>
        DT readFlags()
        {
            static_assert(utils::isEnum<DT>,
                "T must be either enum, Flags or TypeMask type"
            );
            return getFlags<DT>();
        }

        /**
         * Reads GradientColor object from text stream in format: "(r/g/b/a/r/g/b/a/r/g/b/a/r/g/b/a)"
         * @return GradientColor object.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If string GradientColor object in stream is not in valid format..
         */
        GradientColor readGradientColor();

        /**
         * Reads a value of the expected key from the stream.
         *
         * @tparam T - Type of the value to read.
         * @param key - Expected key.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If the token is not a number.
        */
        template<typename T, typename DT = std::decay_t<T>>
        DT readKey(std::string_view key)
        {
            using namespace utils;
            using U = underlying_type_t<DT>;
            if constexpr (isVector<DT>)
            {
                assertKey(key);
                return readVector<DT>();
            }
            else if constexpr (isNumericStdArray<DT>)
            {
                assertKey(key);
                return readNumericArray<typename DT::value_type, arraySize<DT>>();
            }
            else
            {
                readKey(key, cachedTkn_);
                if constexpr(std::is_arithmetic_v<U>) {
                    return static_cast<DT>(cachedTkn_.getNumber<U>());
                } else {
                    return cachedTkn_.value();
                }
            }
        }

        /**
         * Reads the value of the expected key from the stream.
         *
         * @param key - Expected key.
         * @param t   - Token object where the value will be stored.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If the token is not a number.
        */
        void readKey(std::string_view key, Token& t);

        /**
         * Reads key-value from stream in format: key|delim|value.
         * Key-value must be on the same line.
         *
         * @param kt     - Read key token is stored here.
         * @param vt     - Read value token is stored here.
         * @param delim  - Delimiter between key and value.
         * @param strict - If true strict parsing mode is enabled causing an exception to be thrown
         *                 if delimiter is not found or value is either empty string or invalid. By default True.
         * @return True if key-value pair was read, false otherwise.
         *         False is returned also in case when end of line or end of stream is reached.
         *
         * @throw SyntaxError if strict parsing was enabled and an error has occurred.
         * @throw StreamError if IO stream error occurs.
         */
        bool readKeyValue(Token& kt, Token& vt, std::string_view delim = "=", bool strict = true)
        {
            using namespace std::string_view_literals;
            const bool bReportEol = reportEol();
            setReportEol(true);
            AT_SCOPE_EXIT([&] { setReportEol(bReportEol); });

            getNextToken(kt);
            if (!kt.isValid()) {
                return false; // end of stream or end of line was reached
            }

            auto dt = getNextToken();
            if (dt.value() != delim)
            {
                if (strict) throw SyntaxError("Invalid key-value delimiter"sv, dt.location());
                return false;
            }

            peekNextToken(vt);
            if (vt.isNumber()) {
                skipNextToken(); // advance stream; TODO: Find better way to do this.
            }
            else
            {
                if (vt.isValid())
                {
                    getSpaceDelimitedString(vt, /*throwIfEmpty=*/ false);
                    if (strict && vt.value().empty()) {
                        vt.setType(Token::Invalid);
                    }
                }
            }

            if (strict && !vt.isValid()) {
                throw SyntaxError("Invalid value"sv, dt.location());
            }
            return vt.isValid();
        }

        /**
         * Reads string line from current stream position to the end of line.
         * @return String line.
         *
         * @throw StreamError - If an error occurs while reading from stream.
        */
        std::string_view readLine();

        /**
         * Reads a string list of values from the stream.
         *
         * 2 Formats:
         *   1. List with size:
         *      SomeListName 3
         *       value1
         *       value2
         *       value3
         *        ...
         *
         *   2. List without size:
         *      value1
         *      value2
         *      value3
         *       ...
         *      end
         *
         * @tparam Container   - Type of the list to store the values in.
         * @tparam hasRowIdxs  - If true, the serialized text list in stream is expected to have row indices.
         * @tparam hasListSize - If true, the serialized text list in stream is expected to have a size.
         *                       If false, the end of list must be marked with keyword "end".
         * @tparam ReadRowFunc - Function type which parses list line and constructs list element: (TextResourceReader&, std::size_t rowIdx, ElementType& element) -> void
         * @tparam InsertFunc  - Function which inserts parsed element into container: (Container&, std::size_t rowIdx, ElementType&& element) -> void
         *
         * @param expectedName - Expected name of the list. Not required when hasListSize is false.
         * @param readRow      - Function which parses each list row to construct each list element.
         * @param insert       - Function which inserts parsed element into container. Default is DefaultListInserter<Container>.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If text list is not in correct format.
         *                          i.e. if hasListSize is true and the expected name mismatch or the size of the list is not specified.
         * @throw Other exceptions that the constructor function may throw.
         */
        template<typename Container,
                 bool hasRowIdxs  = true,
                 bool hasListSize = true,
                 typename ReadRowFunc,
                 typename InsertFunc = decltype(DefaultListInserter<Container>),
                 typename = utils::requires_container<Container>>
        Container readList(std::string_view expectedName, ReadRowFunc&& readRow, InsertFunc&& insert  = DefaultListInserter<Container>)
        {
            // static_assert(utils::has_mf_push_back<Container> ||
            //               utils::has_no_pos_mf_insert<Container>, "Container doesn't support any valid insertion function!");

            static_assert(std::is_default_constructible_v<Container>, "Container must be default constructible!");

            constexpr bool parseWithNoContainer = std::is_invocable_v<ReadRowFunc, TextResourceReader&, std::size_t, typename Container::value_type&>;
            constexpr bool parseWithContainer   = std::is_invocable_v<ReadRowFunc, TextResourceReader&, const Container&, std::size_t, typename Container::value_type&>;
            static_assert( parseWithNoContainer || parseWithContainer,
                "readRow function must be invocable with arguments: (TextResourceReader&, std::size_t rowIdx, ElementType& element)"
            );

            static_assert(std::is_invocable_v<InsertFunc, Container&, typename Container::value_type&&>,
                "insert function must be invocable with arguments: (Container&, ElementType)"
            );

            using namespace std::string_view_literals;
            auto reserve = [](auto& c, typename Container::size_type r ){
                if constexpr(utils::has_mf_reserve<Container>) {
                    c.reserve(r);
                }
            };

            // auto append = [](auto& c, auto&& v){
            //     if constexpr(utils::has_mf_push_back<Container>) {
            //         c.push_back(std::move(v));
            //     }
            //     else {
            //         insert(c, std::move(v));
            //     }
            // };

            Container container;
            [[maybe_unused]] std::size_t rowIdx = 0;
            std::function<bool()> isAtEnd;

            if constexpr(hasListSize)
            {
                auto len = readKey<std::size_t>(expectedName);
                reserve(container, len);
                isAtEnd = [&, len]() { return rowIdx >= len; };
            }
            else
            {
                reserve(container, 256);
                isAtEnd = [&]() {
                    if (peekNextToken().value() == "end"sv)
                    {
                        getNextToken();
                        return true;
                    }
                    return false;
                };
            }

            while(!isAtEnd())
            {
                if constexpr (hasRowIdxs)
                {
                    [[maybe_unused]] const auto rrowIdx = readRowIdx();
                    assert(rowIdx == rrowIdx && "rowIdx == rrowIdx!");
                }

                if constexpr(!hasListSize && utils::has_mf_capacity<Container>)
                {
                    if (container.capacity() < 10) {
                        reserve(container, 256);
                    }
                }

                typename Container::value_type element{};
                if constexpr(parseWithNoContainer) {
                    readRow(*this, rowIdx, element);
                }
                else {
                    readRow(*this, container, rowIdx, element);
                }

                insert(container, std::move(element));
                rowIdx++;
            }

            return container;
        }

        /**
         * Reads a size-less string list of values from the stream.
         * The end of list must be marked with keyword "end".
         *
         * @tparam Container   - Type of the list to store the values in.
         * @tparam hasRowIdxs  - If true, the serialized text list in stream is expected to have row indices.
         * @tparam ReadRowFunc - Function type which parses list line and constructs list element: (TextResourceReader&, std::size_t rowIdx, ElementType& element) -> void
         * @tparam InsertFunc  - Function which inserts parsed element into container: (Container&, std::size_t rowIdx, ElementType&& element) -> void
         *
         * @param readRow      - Function which parses each list row to construct each list element.
         * @param insert       - Function which inserts parsed element into container. Default is DefaultListInserter<Container>.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If text list is not in correct format.
         *                          i.e. if hasListSize is true and the expected name mismatch or the size of the list is not specified.
         * @throw Other exceptions that the constructor function may throw.
         */
        template<typename Container,
                 bool hasRowIdxs = true,
                 typename ReadRowFunc,
                 typename InsertFunc = decltype(DefaultListInserter<Container>),
                 class = utils::requires_container<Container>>
        Container readList(ReadRowFunc&& readRow, InsertFunc&& insert = DefaultListInserter<Container>) {
            return readList<Container, hasRowIdxs, /*hasListSize*/false>(
                "", std::forward<ReadRowFunc>(readRow), std::forward<InsertFunc>(insert)
            );
        }

        /**
         * Reads numeric std::array from text stream.
         * @tparam T - Type of array elements.
         * @tparam S - Size of array.
         * @return std::array object.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If any of the array elements is not in a number.
         */
        template<typename T, std::size_t S, typename DT = typename std::decay_t<T>>
        std::array<T, S> readNumericArray()
        {
            static_assert(std::is_arithmetic_v<DT>, "T must be an arithmetic type!");
            return readArray<DT, S>([](auto, auto& rr) {
                return rr.getNumber<DT>();
            });
        }

        /**
         * Reads math vector object from text stream.
         *
         * @tparam VecT Type of vector to read.
         * @param strict - If true, the serialized string vector in the stream must be in the format of "(x/y/z/...)".
         *                 If false, the string can be in any format, e.g.: "x y z ...", "x,y, z,..." etc...
         *                 By default false.
         *
         * @throw StreamError - If IO error occurs.
         * @throw SyntaxError - If vector is not in valid format.
         */
        template<typename VecT, typename DVecT = typename std::decay_t<VecT>>
        DVecT readVector(bool strict = false)
        {
            using namespace utils;
            static_assert(isVector<DVecT> && std::is_default_constructible_v<DVecT>,
                "T must be derivative of type AbstractVector and default constructable!"
            );

            if (strict) assertPunctuator("(");
            using et = typename DVecT::value_type;
            auto vec = readArray<et, DVecT::size()>([strict](std::size_t i, TextResourceReader& rr) {
                if (!strict){ // Since we're not in strict mode, skip any unwanted characters.
                    while (rr.skipNextTokenIf(TypeMask(Token::Identifier) | Token::String | Token::Punctuator)) {}
                }
                else if(i > 0) rr.assertPunctuator("/");
                return rr.getNumber<et>();
            });
            if (strict) assertPunctuator(")");
            return static_cast<DVecT&&>(std::move(vec));
        }

        /**
         * Reads Box object from text stream in format: "minx miny minz... maxx maxy maxz".
         * @tparam BoxT - Type of box to read.
         * @return Box object.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If box is not in correct format.
         */
        template<typename BoxT, typename DBoxT = typename std::decay_t<BoxT>>
        DBoxT readBox()
        {
            static_assert(asset::isBox<DBoxT>, "B must be of type Box");

            DBoxT box;
            box.min = readVector<decltype(box.min)>();
            box.max = readVector<decltype(box.max)>();
            return box;
        }

        /**
         * Reads PathFrame object from text stream in format: "(x/y/z:p/y/r)".
         * @return PathFrame object.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If string PathFrame in stream is not in correct format.
         */
        asset::PathFrame readPathFrame();

        /**
         * Reads list row index from text stream in format: "<index>:".
         * @return Row index.
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If string row index in stream is not in correct format.
        */
        std::size_t readRowIdx();

        /**
         * Reads section line from text stream in format: "SECTION section_name".
         * @return Const reference to string section name (section_name).
         *
         * @throw StreamError - If an error occurs while reading from stream.
         * @throw SyntaxError - If section line in stream is not in correct format.
        */
        std::string_view readSection();

    private:
        template <typename Container>
        static void DefaultListInserter(Container& c, typename Container::value_type&& v)
        {
            if constexpr(utils::has_mf_push_back<Container>) {
                c.push_back(std::move(v));
            }
            else {
                insert(c, std::move(v));
            }
        }
    };
}

// Definition of helper functions & constructors for other types that can be constructed from a string.
namespace libim {
    template<typename T, std::size_t S, typename Tag>
    AbstractVector<T,S,Tag>::AbstractVector(std::string_view vecstr, bool strict)
    {
        using namespace libim::content::text;
        InputBinaryStream istream(vecstr);
        TextResourceReader rr(istream);
        *this = rr.readVector<decltype(*this)>(strict);
    }

    namespace content::asset {
        inline PathFrame::PathFrame(std::string_view framestr)
        {
            using namespace libim::content::text;
            InputBinaryStream istream(framestr);
            TextResourceReader rr(istream);
            *this = rr.readPathFrame();
        }
    }
}
#endif // LIBIM_RESOURCE_READER_H
