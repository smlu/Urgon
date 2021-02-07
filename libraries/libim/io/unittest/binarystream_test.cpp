#include "binarystream_test.h"
#include "../binarystream.h"
#include <libim/math/fmath.h>

#include <assert.h>
#include <cstdint>
#include <string>
#include <string_view>

using namespace libim;

constexpr auto tvBFalse   = bool(false);
constexpr auto tvBTrue    = bool(true);
constexpr auto tvInt8     = int8_t(-128);
constexpr auto tvUInt8    = uint8_t(241);
constexpr auto tvInt16    = int16_t(-32500);
constexpr auto tvUInt16   = uint16_t(35532);
constexpr auto tvInt32    = int32_t(-995575);
constexpr auto tvInt32_2  = int32_t(9856);
constexpr auto tvUInt32   = uint32_t(3000000000);
constexpr auto tvInt64    = int64_t(-42567859);
constexpr auto tvUInt64   = uint64_t(99874154778598);
constexpr auto tvFloat    = float(981587.087565);
constexpr auto tvDouble   = double(3391.00087459);
constexpr auto tvString   = std::string_view("Test test vector");
constexpr auto tvString_2 = std::string_view("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.");


void libim::unit_test::run_binarystream_tests()
{

    ByteArray bytes;

// Test case 1: Write and Read to/from binary stream
    {
        BinaryStream bs(bytes);
        assert(bs.size() == bytes.size());

        // Write to stream
        bs << tvBTrue
           << tvUInt64
           << tvUInt32
           << tvBFalse
           << tvString
           << tvDouble
           << tvInt8
           << tvInt16
           << tvUInt8
           << tvUInt16
           << tvFloat
           << tvInt32
           << tvInt64;

        assert(bs.size() == bytes.size());
        assert(bs.tell() == bytes.size());

        // Read from stream
        bs.seek(0);
        assert(bs.tell() == 0);
        assert(bs.size() == bytes.size());
        assert(bs.read<decltype(tvBTrue)>() == tvBTrue);
        assert(bs.tell() == sizeof(tvBTrue));

        assert(bs.read<decltype(tvUInt64)>() == tvUInt64);
        assert(bs.read<decltype(tvUInt32)>() == tvUInt32);
        assert(bs.read<decltype(tvBFalse)>() == tvBFalse);
        assert(bs.read<std::string>(tvString.size()) == tvString);
        assert(bs.tell() == sizeof(tvBTrue) + sizeof(tvUInt64) + sizeof(tvUInt32) + sizeof(tvBFalse) + tvString.size());

        assert(cmpf(bs.read<decltype(tvDouble)>(), tvDouble));
        assert(bs.read<decltype(tvInt8)>() == tvInt8);
        assert(bs.read<decltype(tvInt16)>() == tvInt16);
        assert(bs.read<decltype(tvUInt8)>() == tvUInt8);
        assert(bs.read<decltype(tvUInt16)>() == tvUInt16);
        assert(cmpf(bs.read<decltype(tvFloat)>(), tvFloat));
        assert(bs.read<decltype(tvInt32)>() == tvInt32);
        assert(bs.read<decltype(tvInt64)>() == tvInt64);

        assert(bs.atEnd());
        assert(bs.tell() == bytes.size());
    }


// Test case 2: Read from binary stream
    {
        InputBinaryStream ibs(bytes);
        assert(ibs.size() == bytes.size());

        assert(ibs.read<decltype(tvBTrue)>() == tvBTrue);
        assert(ibs.tell() == sizeof(tvBTrue));

        assert(ibs.read<decltype(tvUInt64)>() == tvUInt64);
        assert(ibs.read<decltype(tvUInt32)>() == tvUInt32);
        assert(ibs.read<decltype(tvBFalse)>() == tvBFalse);
        assert(ibs.read<std::string>(tvString.size()) == tvString);
        assert(cmpf(ibs.read<decltype(tvDouble)>(), tvDouble));
        assert(ibs.tell() == sizeof(tvBTrue) + sizeof(tvUInt64) + sizeof(tvUInt32) + sizeof(tvBFalse) + tvString.size() + sizeof(tvDouble));

        assert(ibs.read<decltype(tvInt8)>() == tvInt8);
        assert(ibs.read<decltype(tvInt16)>() == tvInt16);
        assert(ibs.read<decltype(tvUInt8)>() == tvUInt8);
        assert(ibs.read<decltype(tvUInt16)>() == tvUInt16);
        assert(cmpf(ibs.read<decltype(tvFloat)>(), tvFloat));
        assert(ibs.read<decltype(tvInt32)>() == tvInt32);
        assert(ibs.read<decltype(tvInt64)>() == tvInt64);

        assert(ibs.atEnd());
        assert(ibs.tell() == bytes.size());
    }

// Test case 3: Read chunk of data from binary stream
    {
        auto beginOffset = sizeof(tvBTrue) + sizeof(tvUInt64);
        auto endOffset   = sizeof(tvInt32) + sizeof(tvInt64);
        auto itBegin = bytes.begin() + beginOffset;
        auto itEnd   = bytes.end()   - endOffset;
        auto ibsSize = bytes.size()  - (beginOffset + endOffset);

        InputBinaryStream ibs(bytes, itBegin, itEnd);
        assert(ibs.size() == ibsSize);

        assert(ibs.read<decltype(tvUInt32)>() == tvUInt32);
        assert(ibs.read<decltype(tvBFalse)>() == tvBFalse);
        assert(ibs.read<std::string>(tvString.size()) == tvString);
        assert(cmpf(ibs.read<decltype(tvDouble)>(), tvDouble));
        assert(ibs.read<decltype(tvInt8)>() == tvInt8);
        assert(ibs.read<decltype(tvInt16)>() == tvInt16);
        assert(ibs.read<decltype(tvUInt8)>() == tvUInt8);
        assert(ibs.read<decltype(tvUInt16)>() == tvUInt16);
        assert(cmpf(ibs.read<decltype(tvFloat)>(), tvFloat));

        assert(ibs.atEnd());
        assert(ibs.tell() == ibsSize);
    }

// Test case 4: Write to chunk binary stream and then read the whole buffer stream
    {
        auto beginOffset = sizeof(tvBTrue) + sizeof(tvUInt64);
        auto endOffset   = sizeof(tvInt32) + sizeof(tvInt64);
        auto itBegin = bytes.begin() + beginOffset;
        auto itEnd   = bytes.end()   - endOffset;
        auto obsSize = bytes.size()  - (beginOffset + endOffset);

        OutputBinaryStream obs(bytes, itBegin, itEnd);
        assert(obs.size() == obsSize);

        obs.write(tvInt32_2);
        obs.seekEnd();
        obs.write(tvString_2);

        assert(obs.atEnd());
        assert(obs.size() == obsSize + tvString_2.size());
        assert(obs.tell() == obs.size());
        assert(bytes.size() == obsSize + beginOffset + tvString_2.size());

        // Read all data in buffer
        InputBinaryStream ibs(bytes);
        assert(ibs.read<decltype(tvBTrue)>() == tvBTrue);
        assert(ibs.tell() == sizeof(tvBTrue));

        assert(ibs.read<decltype(tvUInt64)>() == tvUInt64);
        assert(ibs.read<decltype(tvInt32_2)>() == tvInt32_2);
        assert(ibs.read<decltype(tvBFalse)>() == tvBFalse);
        assert(ibs.read<std::string>(tvString.size()) == tvString);
        assert(ibs.tell() == sizeof(tvBTrue) + sizeof(tvUInt64) + sizeof(tvUInt32) + sizeof(tvBFalse) + tvString.size());

        assert(cmpf(ibs.read<decltype(tvDouble)>(), tvDouble));
        assert(ibs.read<decltype(tvInt8)>() == tvInt8);
        assert(ibs.read<decltype(tvInt16)>() == tvInt16);
        assert(ibs.read<decltype(tvUInt8)>() == tvUInt8);
        assert(ibs.read<decltype(tvUInt16)>() == tvUInt16);
        assert(cmpf(ibs.read<decltype(tvFloat)>(), tvFloat));
        assert(ibs.read<std::string>(tvString_2.size()) == tvString_2);

        assert(ibs.atEnd());
        assert(ibs.size() == obs.size() + beginOffset);
        assert(ibs.tell() == ibs.size());
    }
}
