#ifndef LIBIM_PARSE_UTILS_H
#define LIBIM_PARSE_UTILS_H
#include "../../cogscript.h"
#include "../../cogvtable.h"
#include "../../../../../utils/utils.h"

#include <algorithm>
#include <cstdio>
#include <locale>
#include <string>
#include <string_view>
#include <variant>

namespace libim::content::asset::impl {
    [[nodiscard]] inline bool is_op_assign(const libim::text::Token& t)
    {
        using namespace std::string_view_literals;
        return t.type() == libim::text::Token::Punctuator && t.value() == "="sv;
    }

    [[nodiscard]] inline bool is_primitive_type(CogSymbol::Type t)
    {
        return t == CogSymbol::Int   ||
               t == CogSymbol::Flex  ||
               t == CogSymbol::Vector;
    }

    // Checks if string value is of format: (%f/%f/%f)
    [[nodiscard]] inline bool is_valid_raw_vector_value(const std::string& value)
    {
        float vec[3];
#ifdef _MSC_VER
        return sscanf_s(value.data(), "(%f/%f/%f)", &vec[0], &vec[1], &vec[2]) == 3;
#else
        return sscanf(value.data(), "(%f/%f/%f)", &vec[0], &vec[1], &vec[2]) == 3;
#endif
    }

    // Checks if string value is integer or float number. If allowFloat is false, only integers are allowed.
    [[nodiscard]] inline bool is_numeric_raw_value(const std::string& value, bool allowFloat = true)
    {
        bool foundFpDot = false;
        for (const auto[idx, c] : utils::enumerate(value))
        {
            const bool p = ::isdigit(c) ||
                (idx == 0 && (c == '+' || c == '-' )) || // +/- sign at the beginning
                (allowFloat && c == '.' && !foundFpDot && (foundFpDot=true) && value.size() > 1); // floating point
            if(!p) {
                return false;
            }
        }
        return true;
    }

    // Checks if string value is integer.
    [[nodiscard]] inline bool is_int_raw_value(const std::string& value)
    {
        return is_numeric_raw_value(value, false);
    }

    [[nodiscard]] static bool is_valid_raw_init_value(CogSymbol::Type stype, const CogSymbolValue& value)
    {
        using namespace std::string_view_literals;
        auto sval = std::get_if<std::string>(&value);
        switch (stype)
        {
            case CogSymbol::Int:
            case CogSymbol::Flex:
                return std::holds_alternative<int32_t>(value) ||
                        std::holds_alternative<float>(value)  ||
                        (sval && is_numeric_raw_value(*sval));
            case CogSymbol::Ai:
                return sval && utils::iends_with(*sval, ".ai"sv);
            case CogSymbol::Cog:
                return sval && is_int_raw_value(*sval);
            case CogSymbol::Keyframe:
                return sval && utils::iends_with(*sval, ".key"sv);
            case CogSymbol::Material:
                return sval && utils::iends_with(*sval, ".mat"sv);
            case CogSymbol::Model:
                return sval && utils::iends_with(*sval, ".3do"sv);
            case CogSymbol::Sector:
                return sval && is_int_raw_value(*sval);
            case CogSymbol::Sound:
                return sval && utils::iends_with(*sval, ".wav"sv);
            case CogSymbol::Surface:
                return sval && is_int_raw_value(*sval);
            case CogSymbol::Vector:
                return sval && is_valid_raw_vector_value(*sval);
            case CogSymbol::Template:
                return sval && !is_int_raw_value(*sval); // must be string chars not all numeric chars.
            case CogSymbol::Thing:
                return sval && is_int_raw_value(*sval);
            default:
                return false;
        }
    }
}
#endif // LIBIM_PARSE_UTILS_H
