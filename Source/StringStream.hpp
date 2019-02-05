
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

#include "String.hpp"
#include "Array.hpp"

namespace Qentem {

using Qentem::Array;
using Qentem::String;

struct StringStream {
    Array<String> _strings;
    UNumber       Length = 0;

    static void _StringMoveCallback(String *to, String *from, UNumber start, UNumber size) {
        for (UNumber i = 0; i < size; i++) {
            to[start++].Move(from[i]);
        }
    }

    explicit StringStream() noexcept {
        _strings.SetCapacity(10);

        if (Array<String>::Callbacks.MoveCallback == nullptr) {
            Array<String>::Callbacks.MoveCallback = &_StringMoveCallback;
        }
    }

    void Move(String &src) noexcept;

    void operator+=(String &&src) noexcept;
    void operator+=(const String &src) noexcept;

    void operator+=(StringStream &&src) noexcept;
    void operator+=(StringStream const &src) noexcept;

    String Eject() noexcept;
};
} // namespace Qentem

#endif
