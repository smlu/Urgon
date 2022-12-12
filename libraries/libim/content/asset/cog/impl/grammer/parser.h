#ifndef LIBIM_PARRSER_H
#define LIBIM_PARRSER_H
#include "../../cogscript.h"
#include <libim/text/tokenizer.h>

namespace libim::content::asset::impl {
    CogScript parseCogScript(text::Tokenizer& tok, bool parseSymDescription);
}
#endif // LIBIM_PARRSER_H
