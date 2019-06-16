
/**
 * Qentem String
 *
 * @brief     String Stream object for Qentem Engine.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#ifndef QENTEM_STRINGSTREAM_H
#define QENTEM_STRINGSTREAM_H

#include "Array.hpp"
#include "String.hpp"

namespace Qentem {

struct StringStream {
    Array<String> _strings;
    UNumber       Length = 0;

    void operator+=(String &&src) noexcept {
        if (src.Length != 0) {
            Length += src.Length;

            if (_strings.Size == _strings.Capacity) {
                _strings.ExpandTo((_strings.Size + 1) * 4);
            }

            _strings.Storage[_strings.Size] = static_cast<String &&>(src);
            _strings.Size++;
        }
    }

    void operator+=(const String &src) noexcept {
        Length += src.Length;

        if (_strings.Size == _strings.Capacity) {
            _strings.ExpandTo((_strings.Size + 1) * 4);
        }

        _strings.Storage[_strings.Size] = src;
        _strings.Size++;
    }

    String Eject() noexcept {
        if (Length == 0) {
            return L'\0';
        }

        String tmp;
        tmp.SetLength(Length);

        String *sstr;
        UNumber j = 0;

        for (UNumber i = 0; i < _strings.Size; i++) {
            sstr = &(_strings.Storage[i]);
            for (j = 0; j < sstr->Length; j++) {
                tmp.Str[tmp.Length++] = sstr->Str[j];
            }
        }

        tmp.Str[tmp.Length] = L'\0'; // Null trimmed string

        _strings.Reset();
        Length = 0;

        return tmp;
    }
};
} // namespace Qentem

#endif
