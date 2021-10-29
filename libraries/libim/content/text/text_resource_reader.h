#ifndef LIBIM_RESOURCE_READER_H
#define LIBIM_RESOURCE_READER_H
#include <libim/log/log.h>
#include <libim/math/abstract_vector.h>
#include <libim/content/asset/primitives/box.h>
#include <libim/text/tokenizer.h>
#include <libim/utils/traits.h>
#include <libim/utils/utils.h>
#include <libim/types/flags.h>

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

        void assertLabel(std::string_view label);

        void assertKey(std::string_view key);

        template<typename T>
        void assertKeyValue(std::string_view key, T v)
        {
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
                using namespace std::string_view_literals;
                LOG_DEBUG("assertKey: expected value '%', found '%'", v, cachedTkn_.value());
                throw TokenizerError("invalid value"sv, cachedTkn_.location());
            }
        }

        void assertSection(std::string_view section);

        template<typename T, typename DT = typename std::decay_t<T>>
        DT readFlags()
        {
            static_assert(utils::isEnum<DT>,
                "T must be either enum, Flags or TypeMask type"
            );
            return getFlags<DT>();
        }

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
                    return std::move(cachedTkn_).value();
                }
            }
        }

        void readKey(std::string_view key, Token& t);


        template<typename Container,
                 bool hasRowIdxs = true,
                 bool hasListSize = true,
                 typename Lambda,
                 typename = utils::requires_container<Container>>
        Container readList(std::string_view expectedName, Lambda&& constructor)
        {
            static_assert(utils::has_mf_push_back<Container> ||
                          utils::has_no_pos_mf_insert<Container>, "Container doesn't support any valid insertion function!");

            /*TODO: Uncomment when static reflection is available and decltype is avaliable for generic lambdas.

            using LambdaTriats = typename utils::function_traits<Lambda>;
            static_assert(LambdaTriats::arity == 2, "constructor func must have at least 2 arguments");
            static_assert(std::is_same_v<typename LambdaTriats::template arg_t<0>,
                TextResourceReader&>, "first arg in constructor must be of a type TextResourceReader&"
            );
            // Note: Constructor can be of arguments: (TextResourceReader&, std::size_t rowIdx, T& type) or
            //                                        (TextResourceReader&, T& type)
            // TODO: check if constructor func has 2 or 3 arguments
            static_assert(std::is_same_v<typename LambdaTriats::template arg_t<1> or typename LambdaTriats::template arg_t<2>,
                T&>, "second arg in constructor must be of a type T&"
            );
            */

            auto reserve = [](auto& c, typename Container::size_type r ){
                if constexpr(utils::has_mf_reserve<Container>) {
                    c.reserve(r);
                }
            };

            auto append = [](auto& c, auto&& v){
                if constexpr(utils::has_mf_push_back<Container>) {
                    c.push_back(std::move(v));
                }
                else {
                    c.insert(std::move(v));
                }
            };

            Container result;
            [[maybe_unused]] std::size_t rowIdx = 0;
            std::function<bool()> isAtEnd;

            if constexpr(hasListSize)
            {
                auto len = readKey<std::size_t>(expectedName);
                reserve(result, len);
                isAtEnd = [&, len]() { return rowIdx >= len; };
            }
            else
            {
                reserve(result, 256);
                isAtEnd = [&]() {
                    if (peekNextToken().value() == std::string_view("end"))
                    {
                        getNextToken();
                        return true;
                    }
                    return false;
                };
            }


            while(!isAtEnd())
            //for(std::size_t i = 0; i < len; i++)
            {
                if constexpr (hasRowIdxs)
                {
                    [[maybe_unused]] const auto rrowIdx = readRowIdx();
                    assert(rowIdx == rrowIdx && "rowIdx == rrowIdx!");
                }

                if constexpr(!hasListSize && utils::has_mf_capacity<Container>)
                {
                    if (result.capacity() < 10) {
                        reserve(result, 256);
                    }
                }

                typename Container::value_type item;
                constructor(*this, rowIdx, item);
                append(result, std::move(item));
                rowIdx++;
            }

            return result;
        }

        template<typename Container,
                 bool hasRowIdxs = true,
                 typename Lambda,
                 class = utils::requires_container<Container>>
        Container readList(Lambda&& rowReader) {
            return readList<Container, hasRowIdxs, false>("", std::forward<Lambda>(rowReader));
        }

        template<typename T, std::size_t N, typename DT = typename std::decay_t<T>>
        std::array<T, N> readNumericArray()
        {
            static_assert(std::is_arithmetic_v<DT>, "T must be an arithmetic type!");
            std::array<DT, N> result;
            for (std::size_t i = 0; i < result.size(); i++) {
                result[i] = getNumber<DT>();
            }
            return result;
        }

        template<typename T, typename DT = typename std::decay_t<T>>
        DT readVector()
        {
            using namespace utils;
            static_assert(isVector<DT> && std::is_default_constructible_v<DT>,
                "T must be derivative of type AbstractVector and default constructable!"
            );

            return static_cast<DT &&>(
                readNumericArray<typename DT::value_type, DT::size()>()
            );
        }

        template<typename B, typename DB = typename std::decay_t<B>>
        DB readBox()
        {
            static_assert(asset::isBox<DB>, "B must be of type Box");

            DB result;
            result.min = readVector<decltype(result.min)>();
            result.max = readVector<decltype(result.max)>();
            return result;
        }

        std::string readSection();
        std::size_t readRowIdx();

    private:
        void readSection(Token& t);
    };
}
#endif // LIBIM_RESOURCE_READER_H
