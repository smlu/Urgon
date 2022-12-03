#include "parser.h"
#include "parse_utils.h"
#include "../../cogscript.h"
#include "../../cogvtable.h"

#include <libim/io/stremerror.h>
#include <libim/types/typemask.h>
#include <libim/utils/utils.h>
#include <libim/types/flags.h>

#include <filesystem>
#include <string_view>
#include <unordered_map>

using namespace libim;
using namespace libim::content::asset;
using namespace libim::content::asset::impl;
using namespace libim::text;
using namespace libim::utils;
using namespace std::string_view_literals;

// TODO: Define table to store state of global variables: global0-15

static const std::unordered_map<std::string_view, CogSymbol::Type> kSymbolNameMap {
    { "int"sv     , CogSymbol::Int      },
    { "flex"sv    , CogSymbol::Flex     },
    { "float"sv   , CogSymbol::Flex     },
    { "thing"sv   , CogSymbol::Thing    },
    { "template"sv, CogSymbol::Template },
    { "sector"sv  , CogSymbol::Sector   },
    { "surface"sv , CogSymbol::Surface  },
    { "keyframe"sv, CogSymbol::Keyframe },
    { "sound"sv   , CogSymbol::Sound    },
    { "cog"sv     , CogSymbol::Cog      },
    { "material"sv, CogSymbol::Material },
    { "vector"sv  , CogSymbol::Vector   },
    { "model"sv   , CogSymbol::Model    },
    { "ai"sv      , CogSymbol::AI       },
    { "message"sv , CogSymbol::Message  }
};

static const std::unordered_map<CogSymbol::Type, std::string_view> kSymbolTypeMap {
    { CogSymbol::Int     , "int"sv      },
    { CogSymbol::Flex    , "flex"sv     },
    { CogSymbol::Thing   , "thing"sv    },
    { CogSymbol::Template, "template"sv },
    { CogSymbol::Sector  , "sector"sv   },
    { CogSymbol::Surface , "surface"sv  },
    { CogSymbol::Keyframe, "keyframe"sv },
    { CogSymbol::Sound   , "sound"sv    },
    { CogSymbol::Cog     , "cog"sv      },
    { CogSymbol::Material, "material"sv },
    { CogSymbol::Vector  , "vector"sv   },
    { CogSymbol::Model   , "model"sv    },
    { CogSymbol::AI      , "ai"sv       },
    { CogSymbol::Message , "message"sv  }
};


static const std::unordered_map<std::string_view, CogMessageType> kMessageNameMap {
    { "activate"   , CogMessageType::Activate    },
    { "activated"  , CogMessageType::Activated   },
    { "removed"    , CogMessageType::Removed     },
    { "startup"    , CogMessageType::Startup     },
    { "timer"      , CogMessageType::Timer       },
    { "blocked"    , CogMessageType::Blocker     },
    { "entered"    , CogMessageType::Entered     },
    { "exited"     , CogMessageType::Exited      },
    { "crossed"    , CogMessageType::Crossed     },
    { "sighted"    , CogMessageType::Sighted     },
    { "damaged"    , CogMessageType::Damaged     },
    { "arrived"    , CogMessageType::Arrived     },
    { "killed"     , CogMessageType::Killed      },
    { "pulse"      , CogMessageType::Pulse       },
    { "touched"    , CogMessageType::Touched     },
    { "created"    , CogMessageType::Created     },
    { "loading"    , CogMessageType::Loading     },  // Send After the world is loaded up, when the value of each symbol in cog script is being initialized.
    { "selected"   , CogMessageType::Selected    },
    { "deselected" , CogMessageType::Deselected  },
    { "aim"        , CogMessageType::Aim         },
    { "changed"    , CogMessageType::Changed     },
    { "deactivated", CogMessageType::Deactivated },
    { "shutdown"   , CogMessageType::Shutdown    },
    { "respawn"    , CogMessageType::Respawn     },
    { "aievent"    , CogMessageType::AIEvent     },
    { "callback"   , CogMessageType::Callback    },
    { "taken"      , CogMessageType::Taken       },
    { "user0"      , CogMessageType::User0       },
    { "user1"      , CogMessageType::User1       },
    { "user2"      , CogMessageType::User2       },
    { "user3"      , CogMessageType::User3       },
    { "user4"      , CogMessageType::User4       },
    { "user5"      , CogMessageType::User5       },
    { "user6"      , CogMessageType::User6       },
    { "user7"      , CogMessageType::User7       },
    { "newplayer"  , CogMessageType::NewPlayer   },
    { "fire"       , CogMessageType::Fire        },
    { "join"       , CogMessageType::Join        },
    { "leave"      , CogMessageType::Leave       },
    { "splash"     , CogMessageType::Splash      },
    { "trigger"    , CogMessageType::Trigger     },
    { "statechange", CogMessageType::StateChange },
    { "missed"     , CogMessageType::Missed      },
    { "boarded"    , CogMessageType::Boarded     },
    { "unboarded"  , CogMessageType::Unboarded   },
    { "arrivedwpnt", CogMessageType::ArrivedWpnt },
    { "initialized", CogMessageType::Initialized },
    { "updatewpnts", CogMessageType::UpdateWpnts }
};


// TODO: Parse cog script using parse context and write all parsing warning & errors there instead of logging and throwing an exception.



void parseSectionFlags(Tokenizer& tok, CogScript& script)
{
    if(iequal(tok.getNextToken().value(), "flags"sv))
    {
        {
            AT_SCOPE_EXIT([&tok, reol = tok.reportEol()]{
                tok.setReportEol(reol);
            });

            tok.setReportEol(true);

            if(!is_op_assign(tok.getNextToken()))
            {
                auto loc = tok.currentToken().location();
                LOG_ERROR("CogScript %:%:%: Expected assignment operator found '%'", loc.filename, loc.first_line, loc.first_col, tok.currentToken().value());
                throw SyntaxError("Invalid syntax in cog script", loc);
            }

            script.flags = tok.getFlags<decltype(script.flags)>();
        }

        tok.setReportEol(false);
        tok.getNextToken();
    }
}

CogSymbol::Type getSymbolType(Tokenizer& tok)
{
    auto it = kSymbolNameMap.find(tok.currentToken().value());
    if(it == kSymbolNameMap.end())
    {
        auto loc = tok.currentToken().location();
        LOG_ERROR("CogScript %:%:%: Unknown symbol type '%'", loc.filename, loc.first_line, loc.first_col, tok.currentToken().value());
        throw SyntaxError("Unknown symbol type", loc);
    }
    return it->second;
}

void skipAssignment(Tokenizer& tok, Token::Type type)
{
    if(is_op_assign(tok.peekNextToken()))
    {
        tok.skipNextToken();
        if(type == Token::Integer ||
           type == Token::HexInteger ||
           type == Token::OctInteger ||
           type == Token::FloatNumber) {
           tok.skipNextToken();
        }
        else {
            tok.getSpaceDelimitedString(/*throwIfEmpty=*/false);
        }
    }
}

CogSymbolValue parseAssignment(Tokenizer& tok, CogSymbol::Type type)
{
    AT_SCOPE_EXIT([&tok]{
        // Skip ';' at the end of assigned value if present
        if(tok.peekNextToken().value() == ";") {
            tok.skipNextToken();
        }
    });

    const auto& t = tok.getNextToken();
    if(!is_op_assign(t))
    {
        const auto& loc = t.location();
        LOG_ERROR("CogScript %:%:%: Expected assignment operator found '%'", loc.filename, loc.first_line, loc.first_col, t.value());
        throw SyntaxError("Invalid value syntax in cog script", loc);
    }

    // Get arithmetic value
    if(type == CogSymbol::Int ||
       type == CogSymbol::Flex)
    {
        tok.getNextToken();
        if(t.type() == Token::Integer ||
           t.type() == Token::HexInteger ||
           t.type() == Token::FloatNumber)
        {
            if(type == CogSymbol::Int)
            {
                if(t.type() == Token::FloatNumber)
                {
                    auto loc = t.location();
                    LOG_WARNING("CogScript %:%:%: Expected integer value found float '%', truncating to integer", loc.filename, loc.first_line, loc.first_col, t.value());
                }
                return t.getNumber<int32_t>();
            }
            else { // Parse float value
                return t.getNumber<float>();
            }
        }
        else
        {
            auto loc = t.location();
            LOG_ERROR("CogScript %:%:%: Expected numeric value found '%'", loc.filename, loc.first_line, loc.first_col, t.value());
            throw SyntaxError("Invalid value syntax in cog script", loc);
        }
    }

    // Get string value
    auto value = tok.getSpaceDelimitedString(/*throwIfEmpty=*/false);
    to_lower(value);

    if(value.empty()) {
        return std::monostate();
    }
    return value;
}

void parseSymbolInitValue(Tokenizer& tok, CogSymbol& sym)
{
    auto v = parseAssignment(tok, sym.type);
    if(!is_valid_raw_init_value(sym.type, v))
    {
        auto loc = tok.currentToken().location();
        if(std::holds_alternative<std::monostate>(v)) {
            LOG_WARNING("CogScript %:%:%: Found null while parsing initialization value for symbol '%'", loc.filename, loc.first_line, loc.first_col, sym.name);
        }
        else
        {
            LOG_ERROR("CogScript %:%:%: Invalid initialization value '%' for symbol '%' of type '%'", loc.filename, loc.first_line, loc.first_col, tok.currentToken().value(), sym.name, kSymbolTypeMap.at(sym.type));
            throw SyntaxError("Invalid symbol initialization value", loc);
        }
    }

    sym.setDefaultValue(std::move(v));
}

void parseSymbolAttribute(Tokenizer& tok, CogSymbol& sym, bool parse_desc)
{
    const auto& t = tok.currentToken();
    if(iequal(t.value(), "local")){
        sym.isLocal = true;
    }
    // desc
    else if(iequal(t.value(), "desc"))
    {
        if(!is_op_assign(tok.getNextToken()))
        {
            auto loc = t.location();
            LOG_ERROR("CogScript %:%:%: Expected assignment operator after attr 'desc' found '%'", loc.filename, loc.first_line, loc.first_col, t.value());
            throw SyntaxError("Invalid syntax in cog script", loc);
        }

        tok.getDelimitedString([](char c){ return c == '\r' || c == '\n'; });
        if(parse_desc) {
            sym.description = std::move(t).value();
        }
    }
    // nolink
    else if(iequal(t.value(), "nolink"sv)) {
        sym.linkId = -1;
    }
    // linkid
    else if(iequal(t.value(), "linkid"sv))
    {
        if(is_primitive_type(sym.type))
        {
            auto loc = t.location();
            LOG_ERROR("CogScript %:%:%: primitive type can't have attr 'linkId' assigned to.", loc.filename, loc.first_line, loc.first_col);
            throw SyntaxError("Invalid syntax in cog script", loc);
        }

        if(!is_op_assign(tok.peekNextToken()))
        {
            auto loc = t.location();
            LOG_ERROR("CogScript %:%:%: Expected assignment operator after attr 'linkId' found '%'.", loc.filename, loc.first_line, loc.first_col, t.value());
            throw SyntaxError("Invalid syntax in cog script", loc);
        }

        auto v = parseAssignment(tok, CogSymbol::Int);
        if(!std::holds_alternative<int32_t>(v))
        {
            auto loc = t.location();
            LOG_ERROR("CogScript %:%:%: Invalid linkId value '%'.", loc.filename, loc.first_line, loc.first_col, t.value());
            throw SyntaxError("Invalid attr value", loc);
        }

        sym.linkId = std::get<int32_t>(v);
    }
    // mask
    else if(iequal(t.value(), "mask"sv))
    {
        if(!is_primitive_type(sym.type))
        {
            if(is_op_assign(tok.peekNextToken()))
            {
                auto v = parseAssignment(tok, CogSymbol::Int);
                if(!std::holds_alternative<int32_t>(v))
                {
                    auto loc = t.location();
                    LOG_ERROR("CogScript %:%:%: Invalid mask value '%'", loc.filename, loc.first_line, loc.first_col, t.value());
                    throw SyntaxError("Invalid attr value", loc);
                }

                sym.mask = TypeMask<Thing::Type>(std::get<int32_t>(v));
            }
            else
            {
                auto loc = t.location();
                LOG_WARNING("CogScript %:%:%: Expected assignment operator after attr 'mask' found '%'. Attribute ignored.", loc.filename, loc.first_line, loc.first_col, tok.peekNextToken().value());
            }
        }
        else
        {
            auto loc = t.location();
            LOG_WARNING("CogScript %:%:%: Invalid attribute 'mask' for primitive symbol '%'. Attribute ignored.", loc.filename, loc.first_line, loc.first_col, sym.name);
            skipAssignment(tok, Token::HexInteger);
        }
    }
    else
    {
        auto loc = t.location();
        LOG_WARNING("CogScript %:%:%: Unknown symbol attribute '%'", loc.filename, loc.first_line, loc.first_col, t.value());
    }
}

void parseSymbol(Tokenizer& tok, CogScript& script, bool parse_desc)
{
    AT_SCOPE_EXIT([&tok, reol = tok.reportEol()]{
        tok.setReportEol(reol);
    });
    tok.setReportEol(true);

    CogSymbol sym;
    sym.type = getSymbolType(tok);

    const Token& t = tok.getNextToken(); // Note: Do not lowercase symbol name identifier!
    if(t.type() != Token::Identifier)
    {
        auto loc = t.location();
        LOG_ERROR("CogScript %:%:%: Expected symbol name found '%'", loc.filename, loc.first_line, loc.first_col, t.value());
        throw SyntaxError("Invalid symbol name", loc);
    }

    sym.name  = t.value();

    // Message symbol
    if(sym.type == CogSymbol::Message)
    {
        to_lower(sym.name); // Lowercase message symbol name
        auto it = kMessageNameMap.find(sym.name);
        if(it == kMessageNameMap.end())
        {
            auto loc = t.location();
            LOG_ERROR("CogScript %:%:%: Unknown COG message '%'", loc.filename, loc.first_line, loc.first_col, t.value());
            throw SyntaxError("Unknown COG message", loc);
        }

        sym.setDefaultValue(it->second);
        sym.isLocal = true;
    }
    // Variable symbol
    else
    {
        // Get symbol default value
        if(is_op_assign(tok.peekNextToken())) {
            parseSymbolInitValue(tok, sym);
        }

        // Set default mask
        if(!is_primitive_type(sym.type)) {
            sym.mask = TypeMask(Thing::Player) | Thing::Free; // 0x401
        }
        else { // nolink for primitive type
            sym.linkId = -1;
        }

        // Parse symbol attributes
        tok.getNextToken();
        while(t.type() != Token::EndOfLine && t.type() != Token::EndOfFile)
        {
            if(t.type() == Token::Identifier)
            {
                parseSymbolAttribute(tok, sym, parse_desc);
                tok.getNextToken();
            }
            else
            {
                auto loc = t.location();
                LOG_ERROR("CogScript %:%:%: Expected symbol attribute name found '%'", loc.filename, loc.first_line, loc.first_col, t.value());
                throw SyntaxError("Invalid symbol attribute", loc);
            }
        }
    }

    // Try to insert symbol to script's symbol map
    if(!script.symbols.pushBack(sym.name, std::move(sym)).second)
    {
        // Symbol already exists
        auto loc = t.location();
        if(sym.type == CogSymbol::Message) // Skip message symbol
        {
            LOG_WARNING("CogScript %:%:%: Skipping duplicated message symbol '%'", loc.filename, loc.first_line, loc.first_col, sym.name);
            return;
        }

        // Make new symbol name for map key value
        std::string sname = sym.name;
        for(std::size_t i = 0; i < 256; i++)
        {
            auto tmp = sname + "_" + to_string(i);
            if(!script.symbols.contains(tmp))
            {
                sname = std::move(tmp);
                break;
            }
        }

        if(sname == sym.name)
        {
            LOG_ERROR("CogScript %:%:%: Too many duplicated symbols with name '%s'", loc.filename, loc.first_line, loc.first_col, sym.name);
            throw SyntaxError("Cog script syntax error, too many duplicated symbol names", loc);
        }

        LOG_WARNING("CogScript %:%:%: Found duplicated symbol '%', inserting into symbol map as '%'", loc.filename, loc.first_line, loc.first_col, sym.name, sname);
        script.symbols.pushBack(sname, std::move(sym));
    }
}

void parseSectionSymbols(Tokenizer& tok, CogScript& script, bool parse_description)
{
    AT_SCOPE_EXIT([&tok, reol = tok.reportEol()]{
        tok.setReportEol(reol);
    });

    const auto& t = tok.currentToken();
    if(!iequal(t.value(), "symbols"sv))
    {
        auto loc = t.location();
        LOG_ERROR("CogScript %:%:%: Expected section 'symbols' found '%'", loc.filename, loc.first_line, loc.first_col, t.value());
        throw SyntaxError("Invalid cog script section", loc);
    }

    // Parse symbols
    tok.getNextToken(/*lowercased=*/true);
    while(!iequal(t.value(), "end"sv))
    {
        if(t.type() == Token::EndOfLine) {
            continue;
        }
        else if(t.type() == Token::EndOfFile)
        {
            auto loc = tok.currentToken().location();
            LOG_ERROR("CogScript %:%:%: Unexpected end of file in symbols section", loc.filename, loc.first_line, loc.first_col);
            throw SyntaxError("Unexpected end of file in cog script", loc);
        }
        else if(t.type() == Token::Identifier)
        {
            parseSymbol(tok, script, parse_description);
            tok.setReportEol(false);
            tok.getNextToken(/*lowercased=*/true);
        }
        else
        {
            auto loc = tok.currentToken().location();
            LOG_ERROR("CogScript %:%:%: Expected a symbol type found '%'", loc.filename, loc.first_line, loc.first_col, tok.currentToken().value());
            throw SyntaxError("Invalid symbol type", loc);
        }
    }

    tok.getNextToken();
}

void libim::content::asset::impl::parseCogScript(Tokenizer& tok, CogScript& script, bool parseSymDescription)
{
    script.setName(tok.istream().name());
    parseSectionFlags(tok, script);
    parseSectionSymbols(tok, script, parseSymDescription);
}
