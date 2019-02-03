
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

    explicit StringStream() = default;

    void operator+=(String &&src) noexcept;
    void operator+=(const String &src) noexcept;

    String Eject() noexcept;
};
} // namespace Qentem

#endif
