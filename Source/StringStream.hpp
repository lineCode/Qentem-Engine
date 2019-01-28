
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

    explicit StringStream() = default;

    void operator+=(String &&) noexcept;      // Move
    void operator+=(const String &) noexcept; // Add +=

    String Eject() noexcept;
};
} // namespace Qentem

#endif
