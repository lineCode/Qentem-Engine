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
        UNumber        Length;
        wchar_t const *Str;
        wchar_t *      Collect;
    };

    Array<StringBit> Bits;
    UNumber          Length = 0;

    void operator+=(wchar_t const *str) noexcept {
        if (str != nullptr) {
            UNumber len = String::Count(str);
            Length += len;
            Bits += {len, str, nullptr};
        }
    }

    void Add(wchar_t const *str, UNumber const len) noexcept {
        Length += len;
        Bits += {len, str, nullptr};
    }

    void operator+=(String &&src) noexcept {
        if (src.Length != 0) {
            Length += src.Length;
            Bits += {src.Length, src.Str, src.Str};
            src.Str = nullptr;
        }
    }

    void operator+=(String const &src) noexcept {
        if (src.Length != 0) {
            Length += src.Length;
            Bits += {src.Length, src.Str, nullptr};
        }
    }

    String Eject() noexcept {
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
                Memory::Deallocate<wchar_t>(&(bit->Collect));
            }
        }
        tmp[tmp.Length] = L'\0';

        Bits.Reset();

        return tmp;
    }
};

} // namespace Qentem

#endif
