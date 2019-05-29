#ifndef LIBIM_RESOURCE_READER_H
#define LIBIM_RESOURCE_READER_H
#include "../../math/abstract_vector.h"
#include "../../log/log.h"
#include "../../text/tokenizer.h"
#include "../../utils/utils.h"

#include <cstdint>
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

        template<typename T, typename DT = std::decay_t<T>>
        void assertKeyValue(std::string_view key, DT v)
        {
            if constexpr (std::is_enum_v<DT>){
                return assertKeyValue(key, utils::to_underlying(v));
            }

            bool bValid = false;
            auto rv = readKey<DT>();
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
            static_assert(std::is_enum_v<DT>, "T must be an enum type");

            using U = utils::underlying_type_t<DT>;
            return static_cast<DT>(getNumber<U>());
        }

        template<typename T, typename DT = std::decay_t<T>>
        DT readKey(std::string_view key)
        {
            using U = utils::underlying_type_t<DT>;
            if constexpr(isVector<DT>)
            {
                assertKey(key);
                return readVector<DT>();
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

        template<typename T, bool hasRowIdxs = true, typename Lambda, typename DT = std::decay_t<T>>
        std::vector<DT> readList(std::string_view expectedName, Lambda&& constructor)
        {
            /*TODO: Uncomment when static reflection is available and decltype is avaliable for generic lambdas.

            using LambdaTriats = typename utils::function_traits<Lambda>;
            static_assert(LambdaTriats::arity == 2, "constructor func must have 2 arguments");
            static_assert(std::is_same_v<typename LambdaTriats::template arg_t<0>,
                TextResourceReader&>, "first arg in constructor must be of a type TextResourceReader&"
            );

            static_assert(std::is_same_v<typename LambdaTriats::template arg_t<1>,
                T&>, "second arg in constructor must be of a type T&"
            );
            */

            auto len = readKey<std::size_t>(expectedName);
            std::vector<DT> result;
            result.reserve(len);


            for(/*[[maybe_unused]] MSVC 17 doesn't like this */std::size_t i = 0; i < len; i++)
            {
                if constexpr (hasRowIdxs)
                {
                    [[maybe_unused]]  const auto rowIdx = readRowIdx();
                    assert(i == rowIdx && "reading list row failed!");
                }

                T item;
                constructor(*this, item);
                result.push_back(std::move(item));
            }

            return result;
        }

        template<typename T, typename DT = typename std::decay_t<T>>
        DT readVector()
        {
            static_assert (isVector<DT> &&
                           std::is_default_constructible_v<DT>, "T must be derivative of type AbstractVector and default constructable");

            DT result;
            for(typename DT::size_type i = 0; i < result.size(); i++) {
                result[i] = getNumber<typename DT::value_type>();
            }
            return result;
        }


        std::string readSection();
        void readSection(Token& t);

    private:
        std::size_t readRowIdx();
    };
}
#endif // LIBIM_RESOURCE_READER_H
