#include "../cogscript.h"
#include "../../../../text/tokenizer.h"
#include "../../../../io/stream.h"
#include "grammer/parser.h"

using namespace libim;
using namespace libim::content::asset;
using namespace libim::text;

namespace asset = libim::content::asset;

asset::CogScript asset::loadCogScript(const InputStream& istream, bool parseSymDescription)
{
    Tokenizer tok(istream);
    return cogParseScript(tok, parseSymDescription);
}
