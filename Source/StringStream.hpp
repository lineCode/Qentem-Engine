/**
 * Qentem String Stream
 *
 * @brief     String Stream object for Qentem Engine.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Array.hpp"
#include "String.hpp"

#ifndef QENTEM_STRINGSTREAM_H
#define QENTEM_STRINGSTREAM_H

namespace Qentem {

struct StringStream {
    struct StringBit {
        UNumber     Length;
        const char *Str;
        char *      Collect;
    };

    Array<StringBit> Bits;
    UNumber          Length = 0;

    void Add(const char *str, const UNumber length) noexcept {
        Length += length;
        Bits += {length, str, nullptr};
    }

    void Add(char *str, const UNumber length) noexcept {
        Length += length;
        Bits += {length, str, str};
    }

    void operator+=(const char *str) noexcept {
        if (str != nullptr) {
            UNumber length = String::Count(str);
            Length += length;
            Bits += {length, str, nullptr};
        }
    }

    void operator+=(String &&src) noexcept {
        if (src.Length != 0) {
            Length += src.Length;
            Bits += {src.Length, src.Str, src.Str};
            src.Str = nullptr;
        }
    }

    void operator+=(const String &src) noexcept {
        if (src.Length != 0) {
            Length += src.Length;
            Bits += {src.Length, src.Str, nullptr};
        }
    }

    String ToString() noexcept {
        String tmp(Length);
        Length = 0;

        StringBit *bit;
        UNumber    j;

        for (UNumber i = 0; i < Bits.Size; i++) {
            bit = &(Bits[i]);

            for (j = 0; j < bit->Length; j++) {
                tmp[tmp.Length++] = bit->Str[j];
            }

            if (bit->Collect != nullptr) {
                Memory::Deallocate<char>(&(bit->Collect));
            }
        }

        tmp[tmp.Length] = '\0';

        Bits.Reset();

        return tmp;
    }
};

} // namespace Qentem

#endif
