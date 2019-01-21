
/**
 * Qentem String
 *
 * @brief     String object for Qentem Engine.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "String.hpp"

Qentem::String::String(const String &src) noexcept {
    if (this != &src) {
        Add(src.Str, this, 0, src.Length);
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
    Add(&_str[0], this, 0, 0);
}

Qentem::String::String(const wchar_t _str) noexcept {
    Add(&_str, this, this->_index, 1); // one char
}

Qentem::String::~String() noexcept {
    delete[] this->Str;
}

Qentem::String &Qentem::String::operator=(const String &src) noexcept { // Copy
    if (this != &src) {
        Clear(this);
        Add(src.Str, this, 0, src.Length);
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

Qentem::String &Qentem::String::operator+=(String &&src) noexcept { // Move
    if ((this != &src) && (src.Length != 0)) {
        Add(src.Str, this, this->_index, src.Length);

        src.Length = 0;
        src._index = 0;
        delete[] src.Str;
        src.Str = nullptr;
    }

    return *this;
}

Qentem::String &Qentem::String::operator+=(const String &src) noexcept {
    if ((this != &src) && (src.Length != 0)) {
        Add(src.Str, this, this->_index, src.Length);
    }

    return *this;
}

const Qentem::String Qentem::String::operator+(String &&src) const noexcept {
    String ns = *this;
    if (this != &src) {

        if (src.Length != 0) {
            Add(src.Str, &ns, ns._index, src.Length);
        }

        src.Length = 0;
        src._index = 0;
        delete[] src.Str;
        src.Str = nullptr;
    }

    return ns;
}

const Qentem::String Qentem::String::operator+(const String &src) const noexcept {
    String ns = *this;

    if (src.Length != 0) {
        Add(src.Str, &ns, ns._index, src.Length);
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

    UNumber i = 0;
    while ((this->Str[i] == src.Str[i]) && (++i < this->Length)) {
        // Nothing is needed here.
    }

    return (i == this->Length);
}

const bool Qentem::String::operator!=(const String &src) const noexcept {
    return (!(*this == src));
}

void Qentem::String::SetSize(String *src, UNumber _length) noexcept {
    auto _tmp = new wchar_t[(_length + 1)];

    UNumber i = 0;

    if (_length > src->Length) {
        for (; i < src->Length; i++) {
            _tmp[i] = src->Str[i];
        }
    } else {
        for (; i < _length; i++) {
            _tmp[i] = src->Str[i];
        }
    }
    _tmp[i] = '\0'; // To mark the end of string.

    delete[] src->Str;
    src->Str    = _tmp;
    src->_index = i;
    src->Length = _length;
}

Qentem::String Qentem::String::Trim(const String &str) noexcept {
    UNumber start = 0;
    UNumber len   = str.Length;

    while (str.Str[start++] == L' ') {
    }
    start -= 1;

    while (str.Str[--len] == L' ') {
    }
    len += 1;

    String tmp = String::Part(str, start, len - start);
    return tmp;
}

Qentem::String Qentem::String::Revers(const String &str) noexcept {
    String tmp = L"";
    String::SetSize(&tmp, str.Length);

    for (UNumber g = str.Length; g > 0; --g) {
        tmp += str.Str[g - 1];
    }

    return tmp;
}

Qentem::String Qentem::String::FromNumber(double number, UNumber min, UNumber max) noexcept {
    String sign = L"";
    if (number < 0.0) {
        sign = L"-";
        number *= -1.0;
    }

    UNumber num;
    String  tmp_g = L"";
    if (max != 0) {
        double nuw = 1;
        for (UNumber i = 0; i <= max; i++) {
            nuw *= 10;
        }

        num = static_cast<UNumber>(number * nuw);

        UNumber di = (num % 10);
        num /= 10;

        if (di >= 5) {
            num += 1;
        }

        for (UNumber g = 0; g < max; g++) {
            di = (num % 10);
            tmp_g += wchar_t(di + 48);
            num /= 10;
        }

        tmp_g = String::Revers(tmp_g);
    } else {
        num = static_cast<UNumber>(number);
    }

    String tmp_l = L"";
    while (num > 0.0) {
        tmp_l += wchar_t(((num % 10) + 48));
        num /= 10;
    }

    tmp_l = String::Revers(tmp_l);

    String min_str = L"";
    for (UNumber i = tmp_l.Length; i < min; i++) {
        min_str += L"0";
    }
    tmp_l = min_str + tmp_l;

    if (tmp_g.Length != 0) {
        tmp_l = tmp_l + L"." + tmp_g;
    }

    return (sign + tmp_l);
}

bool Qentem::String::ToNumber(const String &str, double &number) noexcept {
    if (str.Length == 0) {
        return false;
    }

    wchar_t c;
    number        = 0.0;
    UNumber m     = 1;
    UNumber start = 0;
    UNumber len   = str.Length;

    while (str.Str[--len] == L' ') {
    }
    len += 1;

    while (str.Str[start++] == L' ') {
    }
    start -= 1;

    do {
        c = str.Str[--len];

        if ((c <= 47) || (c >= 58)) {
            if (c == L'-') {
                number *= -1.0;
                if (len > start) {
                    number = 0.0;
                    return false;
                }
                break;
            }

            if (c == L'.') {
                number /= m;
                m = 1;
                continue;
            }

            number = 0.0;
            return false;
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
 * @brief Get a part of a text.
 *
 * @param offset An index to start from.
 * @param limit The number of characters to copy.
 * @return The requested string.
 */
Qentem::String Qentem::String::Part(const String &src, UNumber offset, UNumber limit) {
    if ((limit > src.Length) || (offset + limit) > src.Length) {
        throw;
    }

    String bit;
    String::SetSize(&bit, limit);

    UNumber i = 0;
    while (i < limit) {
        bit.Str[i++] = src.Str[offset++];
    }

    bit.Str[i] = '\0'; // To mark the end of a string.
    bit._index = i;

    return bit;
}

void Qentem::String::Clear(String *str) noexcept {
    delete[] str->Str;
    str->Str    = nullptr;
    str->Length = 0;
    str->_index = 0;
}

void Qentem::String::Add(const wchar_t *str_p, String *to, UNumber start_at, UNumber ln) noexcept {
    if ((ln == 0) && (str_p != nullptr)) {
        while (str_p[ln++] != '\0') {
            // Counting (getting the length).
        };

        --ln;
    }

    UNumber j = 0;
    if ((to->Length == 0) || (ln > (to->Length - to->_index))) {
        auto *_tmp = new wchar_t[(ln + to->_index + 1)];

        if (to->Str != nullptr) {
            for (UNumber i = 0; i < to->_index; i++) {
                _tmp[i] = to->Str[j++];
            }

            delete[] to->Str;
        }

        to->Str = _tmp;
    }

    for (j = 0; j < ln;) {
        to->Str[start_at++] = str_p[j++];
    }

    to->Str[start_at] = '\0'; // Null ending.

    to->Length = ln + to->_index;
    to->_index = to->Length;
}
