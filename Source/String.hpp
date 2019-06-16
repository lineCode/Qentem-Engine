
/**
 * Qentem String
 *
 * @brief     String object for Qentem Engine.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#ifndef QENTEM_STRING_H
#define QENTEM_STRING_H

#include "Common.hpp"

namespace Qentem {

struct String {
    UNumber  Length   = 0;
    UNumber  Capacity = 0;
    wchar_t *Str      = nullptr; // NULL terminated wchar_t

    String() = default;
    // TODO: implement share(const wchar_t *str)

    static void Copy(String &des, const wchar_t *src_p, UNumber start_at, UNumber ln) noexcept {
        // Copy any existing characters
        UNumber j = 0;
        if ((des.Capacity == 0) || (ln > (des.Capacity - des.Length))) {
            wchar_t *_tmp = des.Str;
            des.Capacity  = (ln + des.Length);
            des.Str       = new wchar_t[(des.Capacity + 1)];

            if (_tmp != nullptr) {
                for (UNumber i = 0; i < des.Length; i++) {
                    des.Str[i] = _tmp[j++];
                }
            }

            delete[] _tmp;
        }

        // Add the the new characters
        for (j = 0; j < ln;) {
            des.Str[start_at++] = src_p[j++];
        }

        des.Str[start_at] = L'\0'; // Null ending.

        // Update the index to be at the last character
        des.Length = (ln + des.Length);
    }

    String(const wchar_t *str) noexcept {
        UNumber _length = 0;
        while (str[_length] != L'\0') {
            ++_length;
        };

        if (_length != 0) {
            this->Length = this->Capacity = _length;

            this->Str = new wchar_t[_length + 1]; // 1 for /0
            for (UNumber j = 0; j <= _length; j++) {
                this->Str[j] = str[j];
            }
        }
    }

    String(const char *str) noexcept {
        UNumber _length = 0;
        while (str[_length] != L'\0') {
            ++_length;
        };

        if (_length != 0) {
            this->Length = this->Capacity = _length;

            this->Str = new wchar_t[_length + 1]; // 1 for /0
            for (UNumber j = 0; j <= _length; j++) {
                this->Str[j] = static_cast<wchar_t>(str[j]);
            }
        }
    }

    String(const wchar_t str) noexcept { // one wchar
        if (str != L'\0') {
            this->Length = this->Capacity = 1;

            this->Str    = new wchar_t[2]; // 1 for /0
            this->Str[0] = str;
            this->Str[1] = L'\0';
        }
    }

    String(const char str) noexcept { // one char
        if (str != '\0') {
            this->Length = this->Capacity = 1;

            this->Str    = new wchar_t[2]; // 1 for /0
            this->Str[0] = static_cast<wchar_t>(str);
            this->Str[1] = L'\0';
        }
    }

    String(String &&src) noexcept { // Move
        if (src.Length != 0) {
            this->Str      = src.Str;
            this->Capacity = src.Capacity;
            this->Length   = src.Length;

            src.Capacity = 0;
            src.Length   = 0;
            src.Str      = nullptr;
        }
    }
    // noexcept
    String(const String &src) { // Copy
        if (src.Length != 0) {
            this->Length = this->Capacity = src.Length;

            this->Str = new wchar_t[this->Capacity + 1]; // 1 for /0
            for (UNumber j = 0; j <= this->Capacity; j++) {
                this->Str[j] = src.Str[j];
            }
        }
    }

    ~String() noexcept {
        this->Capacity = 0;
        this->Length   = 0;
        delete[] this->Str;
        this->Str = nullptr;
    }

    String &operator=(String &&src) noexcept { // Move
        if (this != &src) {
            delete[] this->Str;

            this->Str      = src.Str;
            this->Capacity = src.Capacity;
            this->Length   = src.Length;

            src.Capacity = 0;
            src.Length   = 0;
            src.Str      = nullptr;
        }

        return *this;
    }

    String &operator=(const String &src) noexcept { // Copy
        if (this != &src) {
            delete[] this->Str;

            this->Str    = nullptr;
            this->Length = this->Capacity = src.Length;

            if (src.Length != 0) {
                this->Str = new wchar_t[this->Length + 1]; // 1 for /0
                for (UNumber j = 0; j <= this->Length; j++) {
                    this->Str[j] = src.Str[j];
                }
            }
        }

        return *this;
    }

    String &operator+=(String &&src) noexcept { // Move
        if ((src.Capacity != 0) && (src.Str != nullptr)) {
            // Copy(src.Str, this->Length, src.Capacity);
            Copy(*this, src.Str, this->Length, src.Capacity);

            delete[] src.Str;
            src.Str      = nullptr;
            src.Capacity = 0;
            src.Length   = 0;
        }

        return *this;
    }

    String &operator+=(const String &src) noexcept { // Appand a string
        if (src.Capacity != 0) {
            Copy(*this, src.Str, this->Length, src.Capacity);
        }

        return *this;
    }

    // Appand a string by moving another into it
    String operator+(String &&src) const noexcept {
        String ns;
        ns.SetLength(this->Length + src.Length);

        if (this->Length != 0) {
            Copy(ns, this->Str, 0, this->Length);
        }

        if (src.Length != 0) {
            Copy(ns, src.Str, ns.Length, src.Length);
        }

        delete[] src.Str;
        src.Str      = nullptr;
        src.Capacity = 0;
        src.Length   = 0;

        return ns;
    }

    String operator+(const String &src) const noexcept { // Appand a string and return a new one
        String ns;
        ns.SetLength(this->Length + src.Length);

        if (this->Length != 0) {
            Copy(ns, this->Str, 0, this->Length);
        }

        if (src.Length != 0) {
            Copy(ns, src.Str, ns.Length, src.Length);
        }

        return ns;
    }

    bool operator==(const String &src) const noexcept { // Compare
        if (this->Length != src.Length) {
            return false;
        }

        if (this->Length == 0) {
            return true;
        }

        UNumber i = 0;
        while ((this->Str[i] != L'\0') && (this->Str[i] == src.Str[i])) {
            ++i;
        }

        return (i == this->Length);
    }

    bool operator!=(const String &src) const noexcept { // Compare
        return (!(*this == src));
    }

    void SetLength(const UNumber size) noexcept {
        delete[] this->Str;
        this->Str      = new wchar_t[(size + 1)];
        this->Capacity = size;
        this->Length   = 0;
    }

    // Update the starting index and the ending one to be at the actual characters
    static void SoftTrim(const String &str, UNumber &start, UNumber &end) noexcept {
        if (start >= end) {
            return;
        }

        end -= 1;

        while ((str.Str[start] == L' ') || (str.Str[start] == L'\n')) {
            start += 1;
        }

        while ((end > start) && ((str.Str[end] == L' ') || (str.Str[end] == L'\n'))) {
            end -= 1;
        }
    }

    // Remove empty spaces
    static String Trim(const String &str) noexcept {
        UNumber start = 0;
        UNumber end   = (str.Length - start);

        SoftTrim(str, start, end);

        return Part(str, start, ((end + 1) - start));
    }

    // Revers a string
    static void Revers(String &str) noexcept {
        UNumber i = 0; // Start at 0
        UNumber x = (str.Length - 1);
        wchar_t ch;

        while (i < x) {
            ch         = str.Str[x];
            str.Str[x] = str.Str[i];
            str.Str[i] = ch;
            ++i;
            --x;
        }
    }

    // TODO: Using char[] for faster copying
    static String FromNumber(UNumber number, UNumber min = 1) noexcept {
        String tmp_l;
        while (number > 0) {
            tmp_l += wchar_t(((number % 10) + 48)); // TODO: Too slow. Use tmp_l.Str[i]
            number /= 10;
        }

        if (tmp_l.Length != 0) {
            Revers(tmp_l);
        }

        String min_str;
        for (UNumber i = tmp_l.Length; i < min; i++) {
            min_str += L'0'; // Too slow. Use Array<wchar>
        }

        return (min_str + tmp_l);
    }

    static String FromNumber(double number, UNumber min = 1, UNumber max = 0) noexcept {
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

            if (tmp_g.Length != 0) {
                Revers(tmp_g);
            }
        } else {
            num = static_cast<UNumber>(number);
        }

        String tmp_l; // TODO: Set capacity ahead
        while (num > 0) {
            tmp_l += wchar_t(((num % 10) + 48));
            num /= 10;
        }

        if (tmp_l.Length != 0) {
            Revers(tmp_l);
        }

        String min_str;
        for (UNumber i = tmp_l.Length; i < min; i++) {
            min_str += L'0';
        }
        tmp_l = min_str + tmp_l;

        if (tmp_g.Length != 0) {
            tmp_l += L'.';
            tmp_l += tmp_g;
        }

        return (sign.Length == 0) ? tmp_l : (sign + tmp_l);
    }

    static bool ToNumber(const String &str, UNumber &number, UNumber offset = 0, UNumber limit = 0) noexcept {
        if (limit == 0) {
            limit = str.Length - offset;
        } else {
            limit = limit + offset;
        }

        wchar_t c;
        number    = 0;
        UNumber m = 1;

        for (;;) {
            c = str.Str[--limit];

            if ((c <= 47) || (c >= 58)) {
                number = 0;
                return false;
            }

            number += ((static_cast<UNumber>(c) - 48) * m);

            if (limit == offset) {
                break;
            }

            m *= 10;
        }

        return true;
    }

    static bool ToNumber(const String &str, double &number, UNumber offset = 0, UNumber limit = 0) noexcept {
        if (limit == 0) {
            limit = str.Length - offset;
        } else {
            limit = limit + offset;
        }

        wchar_t c;
        number   = 0.0;
        double m = 1;

        for (;;) {
            c = str.Str[--limit];

            if ((c <= 47) || (c >= 58)) {
                if (c == L'-') {
                    number *= -1.0;
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

            number += static_cast<double>((static_cast<double>(c) - 48) * m);

            if (limit == offset) {
                break;
            }

            m *= 10;
        }

        return true;
    }

    static String Part(const String &src, UNumber offset, const UNumber limit) noexcept {
        // if ((limit > src.Capacity) || ((offset + limit) > src.Capacity)) {
        //     throw;
        // }

        String bit;
        bit.Str      = new wchar_t[(limit + 1)];
        bit.Capacity = limit;

        while (bit.Length < limit) {
            bit.Str[bit.Length++] = src.Str[offset++];
        }

        bit.Str[bit.Length] = L'\0'; // To mark the end of a string.

        return bit;
    }

    static UNumber Hash(const String &src, UNumber start, const UNumber end_offset) noexcept {
        UNumber i    = 0;
        UNumber j    = 1;
        UNumber hash = 0;
        bool    fl   = false;

        while (start < end_offset) {
            if (fl) {
                j = j * (i + 1);
            } else {
                j = (j + 1);
            }

            fl = !fl;

            hash += (((static_cast<UNumber>(src.Str[start]))) * j);
            i++;
            start++;
        }

        return hash;
    }

    void Reset() noexcept { // Reset
        delete[] this->Str;
        this->Str      = nullptr;
        this->Capacity = 0;
        this->Length   = 0;
    }
};
} // namespace Qentem

#endif
