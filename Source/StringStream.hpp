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

    void Add(const UNumber length, const char *str) noexcept {
        if (length != 0) {
            Length += length;
            Bits += {length, str, nullptr};
        }
    }

    void AddnC(const UNumber length, char **str) noexcept { // Move
        if (length != 0) {
            Length += length;
            Bits += {length, *str, *str};
            *str = nullptr;
        }
    }

    inline void operator+=(const char *str) noexcept {
        if (str != nullptr) {
            Add(String::Count(str), str);
        }
    }

    inline void operator+=(String &&src) noexcept {
        AddnC(src.Length, &src.Str);
    }

    inline void operator+=(const String &src) noexcept {
        Add(src.Length, src.Str);
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
