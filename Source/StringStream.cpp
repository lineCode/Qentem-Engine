
/**
 * Qentem String
 *
 * @brief     String Stream object for Qentem Engine.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "StringStream.hpp"

void Qentem::StringStream::operator+=(String &&src) noexcept { // Move
    this->Length += src.Length;
    this->_strings.Add(src);
}

void Qentem::StringStream::operator+=(const String &src) noexcept { // Copy
    this->Length += src.Length;
    this->_strings.Add(src);
}

Qentem::String Qentem::StringStream::Eject() noexcept {
    if (this->Length == 0) {
        return L"";
    }

    String str;
    String::SetSize(&str, this->Length);

    for (UNumber i = 0; i < this->_strings.Size; i++) {
        str += this->_strings[i];
    }

    this->_strings.Clear();

    return str;
}
