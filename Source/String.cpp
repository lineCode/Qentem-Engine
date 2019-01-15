
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
    size_t   i    = 0;

    if (_length > this->Length)
        for (; i < this->Length; i++) {
            _tmp[i] = this->Str[i];
        }
    else {
        for (; i < _length; i++) {
            _tmp[i] = this->Str[i];
        }
    }
    _tmp[i] = '\0'; // To mark the end of string.

    delete[] this->Str;
    this->Str    = _tmp;
    this->_index = i;
    this->Length = _length;
}

Qentem::String Qentem::String::Trim(const String &str) noexcept {
    size_t start = 0;
    size_t len   = str.Length;

    while (str.Str[start++] == L' ') {
    }
    start -= 1;

    while (str.Str[--len] == L' ') {
    }
    len += 1;

    String tmp = str.Part(start, len - start);
    return tmp;
}

Qentem::String Qentem::String::Revers(const String &str) noexcept {
    String tmp = L"";
    tmp.SetSize(str.Length);

    for (size_t g = str.Length; g > 0; --g) {
        tmp += str.Str[g - 1];
    }

    return tmp;
}

Qentem::String Qentem::String::ToString(float number, size_t min, size_t max) noexcept {
    String sign = L"";
    if (number < 0.0f) {
        sign = L"-";
        number *= -1.0f;
    }

    size_t num;
    String tmp_g = L"";
    if (max > 0) {
        float nuw = 1;
        for (size_t i = 0; i <= max; i++) {
            nuw *= 10;
        }

        num = (size_t)(number * nuw);

        size_t di = (num % 10);
        num /= 10;

        if (di >= 5) {
            num += 1;
        }

        for (size_t g = 0; g < max; g++) {
            di = (num % 10);
            tmp_g += wchar_t(di + 48);
            num /= 10;
        }

        tmp_g = String::Revers(tmp_g);
    } else {
        num = (size_t)(number);
    }

    String tmp_l = L"";
    while (num > 0.0f) {
        tmp_l += wchar_t(((num % 10) + 48));
        num /= 10;
    }

    tmp_l = String::Revers(tmp_l);

    String min_str = L"";
    for (size_t i = tmp_l.Length; i < min; i++) {
        min_str += L"0";
    }
    tmp_l = min_str + tmp_l;

    if (tmp_g.Length != 0) {
        tmp_l = tmp_l + L"." + tmp_g;
    }

    return (sign + tmp_l);
}

const bool Qentem::String::ToNumber(const String &str, float &number) noexcept {
    if (str.Length == 0) {
        return false;
    }

    wchar_t c;
    number       = 0.0f;
    size_t m     = 1;
    size_t start = 0;
    size_t len   = str.Length;

    while (str.Str[--len] == L' ') {
    }
    len += 1;

    while (str.Str[start++] == L' ') {
    }
    start -= 1;

    do {
        c = str.Str[--len];

        if ((c < 47) || (c > 58)) {
            if (c == L'-') {
                number *= -1.0f;
                break;
            } else if (c == L'.') {
                number /= m;
                // number += 0.00001f;
                m = 1;
                continue;
            } else {
                number = 0.0f;
                return false;
            }
        }

        number += ((c - 48) * m);

        if (len == start) {
            break;
        }

        m *= 10;
    } while (true);

    return true;
}

/**
 * @brief Get a part of the text.
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
