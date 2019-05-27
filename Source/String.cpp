
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

void Qentem::String::Copy(const wchar_t *str_p, UNumber start_at, UNumber ln) noexcept {
    if ((ln == 0) && (str_p != nullptr)) {
        while (str_p[ln++] != L'\0') {
            // Counting (getting the length).
        };

        --ln;
    }

    // Copy any existing characters
    UNumber j = 0;
    if ((this->Length == 0) || (ln > (this->Length - this->_index))) {
        wchar_t *_tmp = this->Str;
        this->Str     = new wchar_t[(ln + this->_index + 1)];

        if (_tmp != nullptr) {
            for (UNumber i = 0; i < this->_index; i++) {
                this->Str[i] = _tmp[j++];
            }
        }

        delete[] _tmp;
    }

    // Add the the new characters
    for (j = 0; j < ln;) {
        this->Str[start_at++] = str_p[j++];
    }

    this->Str[start_at] = L'\0'; // Null ending.

    // Update the index to be at the last character
    this->_index = this->Length = ln + this->_index;
}

// Moveing a string
void Qentem::String::Move(String &src) noexcept {
    delete[] this->Str;

    if (src.Str != nullptr) {
        this->Str    = src.Str;
        this->Length = src.Length;
        this->_index = src._index;

        src.Length = 0;
        src._index = 0;
        src.Str    = nullptr;
    } else {
        this->Length = 0;
        this->_index = 0;
        this->Str    = nullptr;
    }
}

Qentem::String::String(const wchar_t *str) noexcept { // init of point to wchar_t
    Copy(str, 0, 0);
}

Qentem::String::String(wchar_t str) noexcept { // init of wchar_t
    Copy(&str, 0, 1);
}

Qentem::String::String(String &&src) noexcept { // Move
    if ((this != &src) && (src.Str != nullptr)) {
        Move(src);
    }
}

Qentem::String::String(const String &src) noexcept { // Copy
    if ((this != &src) && (src.Str != nullptr)) {
        Copy(src.Str, 0, src.Length);
    }
}

Qentem::String::~String() noexcept { // Destruct
    delete[] this->Str;
    this->Str = nullptr;
}

Qentem::String &Qentem::String::operator=(String &&src) noexcept { // Move
    if ((this != &src) && (src.Str != nullptr)) {
        Move(src);
    }

    return *this;
}

Qentem::String &Qentem::String::operator=(const String &src) noexcept { // Copy
    if ((this != &src) && (src.Str != nullptr)) {
        this->Clear();
        Copy(src.Str, 0, src.Length);
    } else {
        this->Clear();
    }

    return *this;
}

Qentem::String &Qentem::String::operator+=(String &&src) noexcept { // Move
    if ((src.Length != 0) && (src.Str != nullptr)) {
        Copy(src.Str, this->_index, src.Length);
        src.Clear();
    }

    return *this;
}

Qentem::String &Qentem::String::operator+=(const String &src) noexcept { // Appand a string
    if (src.Length != 0) {
        Copy(src.Str, this->_index, src.Length);
    }

    return *this;
}

// Appand a string by moving another into it
Qentem::String Qentem::String::operator+(String &&src) const noexcept {
    String ns = *this;

    if (src.Length != 0) {
        ns += src;
        src.Clear();
    }

    return ns;
}

Qentem::String Qentem::String::operator+(const String &src) const noexcept { // Appand a string and return a new one
    String ns = *this;

    if (src.Length != 0) {
        ns += src;
    }

    return ns;
}

bool Qentem::String::operator==(const String &src) const noexcept { // Compare
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

constexpr bool Qentem::String::operator!=(const String &src) const noexcept { // Compare
    return (!(*this == src));
}

void Qentem::String::SetSize(const UNumber size) noexcept {
    wchar_t *_tmp = this->Str;
    this->Str     = new wchar_t[(size + 1)];
    UNumber i     = 0;

    if (size > this->_index) {
        for (; i < this->_index; i++) {
            this->Str[i] = _tmp[i];
        }
    } else {
        for (; i < size; i++) {
            this->Str[i] = _tmp[i];
        }
    }

    this->Str[i] = L'\0'; // To mark the end of a string.
    this->_index = i;
    this->Length = size;

    delete[] _tmp;
}

// Update the starting index and the ending one to be at the actual characters
void Qentem::String::SoftTrim(const String &str, UNumber &start, UNumber &end) noexcept {
    if (start >= end) {
        return;
    }
    end -= 1;

    while (str.Str[start] == L' ') {
        start += 1;
    }

    while ((end > start) && (str.Str[end] == L' ')) {
        end -= 1;
    }
}

// Remove empty spaces
Qentem::String Qentem::String::Trim(const String &str) noexcept {
    UNumber start = 0;
    UNumber end   = (str.Length - start);
    SoftTrim(str, start, end);

    return String::Part(str, start, ((end + 1) - start));
}

// Revers string
Qentem::String Qentem::String::Revers(const String &str) noexcept {
    if (str.Length < 2) {
        return str;
    }

    String tmp;
    tmp.SetSize(str.Length);

    for (UNumber g = str.Length; g > 0;) {
        tmp += str.Str[--g];
    }

    return tmp;
}

Qentem::String Qentem::String::FromNumber(UNumber number, UNumber min) noexcept {
    String sign;
    if (number < 0) {
        sign = L"-";
        number *= -1;
    }

    String tmp_l;
    while (number > 0) {
        tmp_l += wchar_t(((number % 10) + 48));
        number /= 10;
    }

    tmp_l = String::Revers(tmp_l);

    String min_str;
    for (UNumber i = tmp_l.Length; i < min; i++) {
        min_str += L"0";
    }
    tmp_l = min_str + tmp_l;

    if (sign.Length == 0) {
        return tmp_l;
    }

    return (sign + tmp_l);
}

Qentem::String Qentem::String::FromNumber(double number, UNumber min, UNumber max) noexcept {
    String sign;
    if (number < 0.0) {
        sign = L'-';
        number *= -1.0;
    }

    UNumber num;
    String  tmp_g;
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

    String tmp_l;
    while (num > 0.0) {
        tmp_l += wchar_t(((num % 10) + 48));
        num /= 10;
    }

    tmp_l = String::Revers(tmp_l);

    String min_str;
    for (UNumber i = tmp_l.Length; i < min; i++) {
        min_str += L'0';
    }
    tmp_l = min_str + tmp_l;

    if (tmp_g.Length != 0) {
        tmp_l += L'.';
        tmp_l += tmp_g;
    }

    if (sign.Length == 0) {
        return tmp_l;
    }

    return (sign + tmp_l);
}

bool Qentem::String::ToNumber(const String &str, UNumber &number, UNumber offset, UNumber limit) noexcept {
    if (limit == 0) {
        limit = str.Length - offset;
    } else {
        limit = limit + offset;
    }

    wchar_t c;
    number    = 0;
    UNumber m = 1;

    do {
        c = str.Str[--limit];

        if ((c <= 47) || (c >= 58)) {
            if (c == L'-') {
                number *= -1;
                if (limit > offset) {
                    number = 0;
                    return false;
                }
                break;
            }

            if (c == L'.') {
                number = 0;
                m      = 1;
                continue;
            }

            number = 0;
            return false;
        }

        number += ((c - 48) * m);

        if (limit == offset) {
            break;
        }

        m *= 10;
    } while (true);

    return true;
}

bool Qentem::String::ToNumber(const String &str, double &number, UNumber offset, UNumber limit) noexcept {
    if (limit == 0) {
        limit = str.Length - offset;
    } else {
        limit = limit + offset;
    }

    wchar_t c;
    number    = 0.0;
    UNumber m = 1;

    do {
        c = str.Str[--limit];

        if ((c <= 47) || (c >= 58)) {
            if (c == L'-') {
                number *= -1.0;
                if (limit > offset) {
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

        number += static_cast<double>((c - 48) * m);

        if (limit == offset) {
            break;
        }

        m *= 10;
    } while (true);

    return true;
}

Qentem::String Qentem::String::Part(const String &src, UNumber offset, const UNumber limit) {
    if ((limit > src.Length) || ((offset + limit) > src.Length)) {
        throw;
    }

    if (limit == 0) {
        return L"";
    }

    String bit;
    bit.SetSize(limit);

    UNumber i = 0;
    while (i < limit) {
        bit.Str[i++] = src.Str[offset++];
    }

    bit.Str[i] = L'\0'; // To mark the end of a string.
    bit._index = i;

    return bit;
}

UNumber Qentem::String::Hash(const String &src, UNumber start, const UNumber end_offset) noexcept {
    bool    fl   = false;
    UNumber i    = 0;
    UNumber j    = 1;
    UNumber hash = 0;

    while (start < end_offset) {
        if (fl) {
            j  = j * (i + 1);
            fl = false;
        } else {
            j  = (j + 1);
            fl = true;
        }

        hash += (((static_cast<UNumber>(src.Str[start]))) * j);
        i++;
        start++;
    }

    return hash;
}

void Qentem::String::Clear() noexcept { // Reset
    delete[] this->Str;
    this->Str    = nullptr;
    this->Length = 0;
    this->_index = 0;
}
