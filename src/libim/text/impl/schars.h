#ifndef LIBIM_SCHARS_H
#define LIBIM_SCHARS_H

namespace libim::text {
    enum SpecialChars : char
    {
        ChBackSlash   = '\\',
        ChComment     = '#',
        ChDblQuote    = '\"',
        ChDecimalSep  = '.',
        ChEol         = '\n',
        ChEof         = '\0',
        ChIdentifier  = '_',
        ChMinus       = '-',
        ChPlus        = '+',
        ChQuote       = '\'',
        ChSpace       = ' ',
        ChTab         = '\t'
    };
}

#endif // LIBIM_SCHARS_H
