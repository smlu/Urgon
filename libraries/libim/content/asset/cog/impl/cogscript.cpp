#include "../cogscript.h"
#include "../../../../text/tokenizer.h"
#include "../../../../io/stream.h"
#include "grammer/parser.h"

using namespace libim;
using namespace libim::content::asset;
using namespace libim::content::asset::impl;
using namespace libim::text;


CogScript::CogScript(const InputStream& istream, bool parseSymDescription)
{
    Tokenizer tok(istream);
    parseCogScript(tok, *this, parseSymDescription);
}
