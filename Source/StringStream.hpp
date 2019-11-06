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
    };

    UNumber          Length = 0;
    Array<StringBit> Bits;
    Array<wchar_t *> _strings;

    void operator+=(String &&src) noexcept {
        if (src.Length != 0) {
            Length += src.Length;
            _strings += src.Str;
            Bits += {src.Length, src.Str};
            src.Str = nullptr;
        }
    }

    void operator+=(String const &src) noexcept {
        if (src.Length != 0) {
            Length += src.Length;
            Bits += {src.Length, src.Str};
        }
    }

    void operator+=(wchar_t const *str) noexcept {
        if (str != nullptr) {
            UNumber len = String::Count(str);
            Length += len;
            Bits += {len, str};
        }
    }

    void Add(wchar_t const *str, UNumber const len) noexcept {
        if (len != 0) {
            Length += len;
            Bits += {len, str};
        }
    }

    String Eject() noexcept {
        String tmp(Length);
        Length = 0;

        UNumber j;
        for (UNumber i = 0; i < Bits.Size; i++) {
            for (j = 0; j < Bits[i].Length; j++) {
                tmp.Str[tmp.Length++] = Bits[i].Str[j];
            }
        }
        Bits.Reset();

        for (UNumber i = 0; i < _strings.Size; i++) {
            Memory::Deallocate<wchar_t>(&_strings[i]);
        }
        _strings.Reset();

        tmp[tmp.Length] = L'\0'; // Null trimmming

        return tmp;
    }
};

} // namespace Qentem

#endif
