#ifndef LIBIM_PARRSER_H
#define LIBIM_PARRSER_H
#include "../../cogscript.h"
#include "../../../../../text/tokenizer.h"

namespace libim::content::asset::impl {
    void parseCogScript(text::Tokenizer& tok, CogScript& script, bool parseSymDescription);
}
#endif // LIBIM_PARRSER_H
