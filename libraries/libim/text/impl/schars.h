#ifndef LIBIM_SCHARS_H
#define LIBIM_SCHARS_H

namespace libim::text {
    enum SpecialChars : char
    {
        ChBackSlash   = '\\',
        ChComment     = '#',
        ChComment2    = '/',
        ChDblQuote    = '\"',
        ChDecimalSep  = '.',
        ChEol         = '\n',
        ChEof         = '\0',
        ChIdentifier  = '_',
        ChIdentifier2 = '$',
        ChMinus       = '-',
        ChPlus        = '+',
        ChCr          = '\r',
        ChQuote       = '\'',
        ChSpace       = ' ',
        ChTab         = '\t'
    };
}

#endif // LIBIM_SCHARS_H
