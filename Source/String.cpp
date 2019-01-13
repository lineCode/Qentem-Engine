
/**
 * Qentem Engine
 *
 * @brief     String class for Qentem Engine.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "String.hpp"

Qentem::String::String(const String &src) noexcept {
    if (this != &src) {
        _add(src.Str, this, 0, src.Length);
    }
}

Qentem::String::String(String &&src) noexcept {
    if (this != &src) {
        delete[] this->Str;
        // Move
        this->Str    = src.Str;
        this->Length = src.Length;
        this->_index = src._index;

        src.Length = 0;
        src._index = 0;
        src.Str    = nullptr;
    }
}

Qentem::String::String(const wchar_t _str[]) noexcept {
    _add(&_str[0], this, 0, 0);
}

Qentem::String::~String() noexcept {
    delete[] this->Str;
}

Qentem::String &Qentem::String::operator=(const wchar_t _str[]) noexcept {
    _reset(this);
    _add(_str, this, 0, 0);
    return *this;
}

Qentem::String &Qentem::String::operator=(const String &src) noexcept { // Copy
    if (this != &src) {
        _reset(this);
        _add(src.Str, this, 0, src.Length);
    }

    return *this;
}

Qentem::String &Qentem::String::operator=(String &&src) noexcept { // Move
    if (this != &src) {
        delete[] this->Str;
        // Move
        this->Str    = src.Str;
        this->Length = src.Length;
        this->_index = src._index;

        src.Length = 0;
        src._index = 0;
        src.Str    = nullptr;
    }

    return *this;
}

Qentem::String &Qentem::String::operator+=(const wchar_t _str[]) noexcept {
    _add(_str, this, this->_index, 0);
    return *this;
}

Qentem::String &Qentem::String::operator+=(const wchar_t _str) noexcept {
    _add(&_str, this, this->_index, 1);
    return *this;
}

Qentem::String &Qentem::String::operator+=(const String &src) noexcept {
    if (src.Length != 0) {
        _add(src.Str, this, this->_index, src.Length);
    }

    return *this;
}

const Qentem::String Qentem::String::operator+(const wchar_t _str[]) const noexcept {
    String ns = *this;
    _add(_str, &ns, ns._index, 0);
    return ns;
}

const Qentem::String Qentem::String::operator+(const String &src) const noexcept {
    String ns = *this;

    if (src.Length != 0) {
        _add(src.Str, &ns, ns._index, src.Length);
    }

    return ns;
}

const bool Qentem::String::operator==(const String &src) const noexcept {
    if (this->Length != src.Length) {
        return false;
    }

    if (this->Length == 0) {
        return true;
    }

    size_t i = 0;
    while ((this->Str[i] == src.Str[i]) && (++i < this->Length)) {
        // Nothing is needed here.
    }

    if (i == this->Length) {
        return true;
    }

    return false;
}

const bool Qentem::String::operator!=(const String &src) const noexcept {
    return (!(*this == src));
}

void Qentem::String::SetSize(size_t _length) noexcept {
    wchar_t *_tmp = new wchar_t[(_length + 1)];
    size_t   j    = 0;

    for (size_t i = 0; i < this->Length; i++) {
        _tmp[i] = this->Str[j++];
    }
    _tmp[j] = '\0'; // To mark the end of a string.

    delete[] this->Str;
    this->Str    = _tmp;
    this->_index = j;
    this->Length = _length;
}

Qentem::String Qentem::String::ToString(float number) noexcept {
    if (number == 0) {
        return L"0";
    }

    String result;
    String tmp = L"";
    size_t num;

    if (number > 0) {
        num    = (size_t)number;
        result = L"";
    } else {
        number *= -1.0f;
        num    = (size_t)number;
        result = L"-";
    }

    while (num > 0) {
        tmp += wchar_t((num % 10) + 48);
        num /= 10;
    }

    result.SetSize(tmp.Length);

    for (size_t g = tmp.Length; g > 0; --g) {
        result += tmp.Str[g - 1];
    }

    return result;
}

const float Qentem::String::ToNumber(const String &str) noexcept {
    if (str.Length == 0) {
        return 0;
    }

    float   num = 0;
    float   m   = 1;
    wchar_t c   = 0;

    size_t len = str.Length; // It has to have a at least one letter before [
    do {
        c = str.Str[--len];

        if ((c < 47) || (c > 58)) {
            if ((len == 0) && (str.Str[0] == '-')) {
                num *= -1;
            }
            break;
        }

        num += ((c - 48) * m);

        if (len == 0) {
            break;
        }

        m *= 10;
    } while (len != 0);

    return num;
}

/**
 * @brief Get a part of text.
 *
 * @param offset An index to start from.
 * @param limit The number of characters to copy.
 * @return The requested string.
 */
Qentem::String Qentem::String::Part(size_t offset, size_t limit) const noexcept {
    if ((offset + limit) > this->Length) {
        return L"";
    }

    String bit;
    bit.SetSize(limit);

    size_t i = 0;
    while (i < limit) {
        bit.Str[i++] = this->Str[offset++];
    }

    bit.Str[i] = '\0'; // To mark the end of a string.
    bit._index = i;

    return bit;
}

void Qentem::String::_reset(String *str) noexcept {
    delete[] str->Str;
    str->Str = nullptr;

    str->Length = 0;
    str->_index = 0;
}

void Qentem::String::_add(const wchar_t *str_p, String *to, size_t start_at, size_t ln) noexcept {
    if ((ln == 0) && (str_p != nullptr)) {
        while (str_p[ln++] != '\0') {
            // Counting (getting the length).
        };

        --ln;
    }

    size_t j = 0;
    if ((to->Length == 0) || (ln > (to->Length - to->_index))) {
        wchar_t *_tmp = new wchar_t[(ln + to->_index + 1)];

        if (to->Str != nullptr) {
            size_t i = 0;
            while ((_tmp[i++] = to->Str[j++]) != '\0') {
                // fast way of copying a string.
            };
            delete[] to->Str;
        }

        to->Str = _tmp;
    }

    for (j = 0; j < ln;) {
        to->Str[start_at++] = str_p[j++];
    }

    to->Str[start_at] = '\0'; // To mark the end of a string.

    to->Length = ln + to->_index;
    to->_index = to->Length;
}
