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
    Array<StringBit> Bits;

    UNumber       Length = 0;
    Array<String> _strings;

    void operator+=(String &&src) noexcept {
        if (src.Length != 0) {
            Length += src.Length;
            _strings += static_cast<String &&>(src);
            String *last = &(_strings[_strings.Size - 1]);
            Bits += {last->Length, last->Str};
        }
    }

    void operator+=(String const *src) noexcept {
        if (src->Length != 0) {
            Length += src->Length;
            Bits += {src->Length, src->Str};
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
        String tmp;
        tmp.SetLength(Length);

        if (Length != 0) {
            UNumber j;
            for (UNumber i = 0; i < Bits.Size; i++) {
                for (j = 0; j < Bits[i].Length; j++) {
                    tmp.Str[tmp.Length++] = Bits[i].Str[j];
                }
            }

            Length = 0;
            _strings.Reset();
            Bits.Reset();
        }

        tmp[tmp.Length] = L'\0'; // Null trimmming

        return tmp;
    }
};

} // namespace Qentem

#endif
