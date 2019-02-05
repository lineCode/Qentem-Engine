
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

void Qentem::StringStream::operator+=(String &&src) noexcept {
    this->Length += src.Length;
    this->_strings.Last().Move(src);
    this->_strings.Size++;
}

void Qentem::StringStream::operator+=(const String &src) noexcept {
    this->Length += src.Length;
    this->_strings.Add(src);
}

void Qentem::StringStream::operator+=(StringStream &&src) noexcept {
    this->_strings.Add(src._strings, true);
    this->Length += src.Length;

    src._strings.Clear();
    src.Length = 0;
}

void Qentem::StringStream::operator+=(const StringStream &src) noexcept {
    this->_strings.Add(src._strings);
    this->Length += src.Length;
}

Qentem::String Qentem::StringStream::Eject() noexcept {
    if (this->Length == 0) {
        return L"";
    }

    String str;
    str.SetSize(this->Length);
    String *sstr;

    UNumber offset = 0;
    UNumber j      = 0;

    for (UNumber i = 0; i < this->_strings.Size; i++) {
        sstr = &(this->_strings[i]);
        for (j = 0; j < sstr->Length; j++) {
            str.Str[offset++] = sstr->Str[j];
        }
    }

    this->_strings.Clear();

    return str;
}
