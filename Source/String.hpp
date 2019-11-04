/**
 * Qentem String
 *
 * @brief     String object for Qentem Engine.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Memory.hpp"

#ifndef QENTEM_STRING_H
#define QENTEM_STRING_H

namespace Qentem {

struct String {
    UNumber  Length   = 0;
    wchar_t *Str      = nullptr; // NULL terminated wchar_t
    UNumber  Capacity = 0;

    String() = default;

    String(char const *str) noexcept {
        while (str[Capacity] != L'\0') {
            ++Capacity;
        };

        Memory::Allocate<wchar_t>(&Str, (Capacity + 1));

        while (Length < Capacity) {
            Str[Length] = static_cast<wchar_t>(str[Length]);
            ++Length;
        }

        Str[Length] = L'\0';
    }

    String(char const str) noexcept { // one char
        Capacity = 1;

        Memory::Allocate<wchar_t>(&Str, 2); // 1 for /0

        Str[Length]   = static_cast<wchar_t>(str);
        Str[++Length] = L'\0';
    }

    String(wchar_t const str) noexcept { // one wchar
        Capacity = 1;

        Memory::Allocate<wchar_t>(&Str, 2); // 1 for /0

        Str[Length]   = str;
        Str[++Length] = L'\0';
    }

    String(wchar_t const *str) noexcept {
        while (str[Capacity] != L'\0') {
            ++Capacity;
        };

        Memory::Allocate<wchar_t>(&Str, (Capacity + 1));

        while (Length < Capacity) {
            Str[Length] = str[Length];
            ++Length;
        }

        Str[Length] = L'\0';
    }

    String(String &&src) noexcept { // Move
        Length       = src.Length;
        src.Length   = 0;
        Str          = src.Str;
        src.Str      = nullptr;
        Capacity     = src.Capacity;
        src.Capacity = 0;
    }

    String(String const &src) noexcept { // Copy
        Capacity = src.Length;

        Memory::Allocate<wchar_t>(&Str, (Capacity + 1));

        for (; Length < Capacity;) {
            Str[Length] = src[Length];
            ++Length;
        }

        Str[Length] = L'\0';
    }

    ~String() noexcept {
        Memory::Deallocate<wchar_t>(&Str);
    }

    inline void Reset() noexcept {
        Memory::Deallocate<wchar_t>(&Str);

        Length   = 0;
        Capacity = 0;
    }

    inline wchar_t *Eject() noexcept {
        wchar_t *_str = Str;
        Str           = nullptr;
        Length        = 0;
        Capacity      = 0;

        return _str;
    }

    inline static UNumber Count(wchar_t const *str) noexcept {
        UNumber _length = 0;
        while (str[_length] != L'\0') {
            ++_length;
        };

        return _length;
    }

    String &operator=(wchar_t const str) noexcept { // Copy
        Memory::Deallocate<wchar_t>(&Str);
        Memory::Allocate<wchar_t>(&Str, 2);

        Capacity = 1;
        Length   = 0;

        if (str != L'\0') {
            Str[Length++] = str;
        }

        Str[Length] = L'\0';

        return *this;
    }

    String &operator=(wchar_t const *str) noexcept { // Copy
        Memory::Deallocate<wchar_t>(&Str);
        Length   = 0;
        Capacity = 0;

        while (str[Capacity] != L'\0') {
            ++Capacity;
        };

        Memory::Allocate<wchar_t>(&Str, (Capacity + 1));

        for (; Length < Capacity;) {
            Str[Length] = str[Length];
            ++Length;
        }

        Str[Length] = L'\0';

        return *this;
    }

    String &operator=(String &&src) noexcept { // Move
        if (this != &src) {
            Memory::Deallocate<wchar_t>(&Str);

            Length       = src.Length;
            src.Length   = 0;
            Str          = src.Str;
            src.Str      = nullptr;
            Capacity     = src.Capacity;
            src.Capacity = 0;
        }

        return *this;
    }

    String &operator=(String const &src) noexcept { // Copy
        if (this != &src) {
            Memory::Deallocate<wchar_t>(&Str);
            Length   = 0;
            Capacity = src.Length;

            Memory::Allocate<wchar_t>(&Str, (Capacity + 1));

            for (; Length < Capacity;) {
                Str[Length] = src[Length];
                ++Length;
            }

            Str[Length] = L'\0';
        }

        return *this;
    }

    String &operator+=(wchar_t const _char) noexcept { // Appand a string
        if (Length == Capacity) {
            if (Capacity == 0) {
                Capacity = 2;
            }

            Resize(Capacity + 1);
        }

        Str[Length]   = _char;
        Str[++Length] = L'\0';

        return *this;
    }

    String &operator+=(wchar_t const *str) noexcept { // Appand a string
        UNumber _length = 0;
        while (str[_length] != L'\0') {
            ++_length;
        };

        Copy(*this, str, this->Length, _length);

        return *this;
    }

    String &operator+=(String &&src) noexcept { // Move
        Copy(*this, src.Str, Length, src.Length);

        Memory::Deallocate<wchar_t>(&src.Str);

        src.Length   = 0;
        src.Str      = nullptr;
        src.Capacity = 0;

        return *this;
    }

    inline String &operator+=(String const &src) noexcept { // Appand a string
        Copy(*this, src.Str, Length, src.Length);

        return *this;
    }

    String operator+(wchar_t const src) const noexcept {
        String ns;
        ns.SetLength(Length + 1);

        Copy(ns, Str, 0, Length);

        ns[ns.Length]   = src;
        ns[++ns.Length] = L'\0';

        return ns;
    }

    String operator+(wchar_t const *str) const noexcept {
        UNumber _length = 0;
        while (str[_length] != L'\0') {
            ++_length;
        };

        String ns;
        ns.SetLength(Length + _length);

        Copy(ns, Str, 0, Length);
        Copy(ns, str, ns.Length, _length);

        return ns;
    }

    // Appand a string by moving another into it
    String operator+(String &&src) const noexcept {
        String ns;

        ns.SetLength(Length + src.Length);
        Copy(ns, Str, 0, Length);

        if (src.Length != 0) {
            Copy(ns, src.Str, ns.Length, src.Length);
            src.Length = 0;
        }

        Memory::Deallocate<wchar_t>(&src.Str);
        src.Capacity = 0;

        return ns;
    }

    String operator+(String const &src) const noexcept { // Appand a string and return a new one
        String ns;

        ns.SetLength(Length + src.Length);
        Copy(ns, Str, 0, Length);

        if (src.Length != 0) {
            Copy(ns, src.Str, ns.Length, src.Length);
        }

        return ns;
    }

    bool operator==(wchar_t const _char) const noexcept { // Compare
        return ((Length == 1) && (Str[0] == _char));
    }

    inline bool operator!=(wchar_t const _char) const noexcept { // Compare
        return (!(*this == _char));
    }

    bool operator==(wchar_t const *string) const noexcept { // Compare
        UNumber i       = 0;
        UNumber _length = 0;
        while (string[_length] != L'\0') {
            ++_length;
        };

        if (Length != _length) {
            return false;
        }

        while ((i < Length) && (Str[i] == string[i])) {
            ++i;
        }

        return (i == Length);
    }

    inline bool operator!=(wchar_t const *string) const noexcept { // Compare
        return (!(*this == string));
    }

    bool operator==(String const &string) const noexcept { // Compare
        if (Length != string.Length) {
            return false;
        }

        UNumber i = 0;
        while ((i < Length) && (Str[i] == string[i])) {
            ++i;
        }

        return (i == Length);
    }

    inline bool operator!=(String const &string) const noexcept { // Compare
        return (!(*this == string));
    }

    inline wchar_t &operator[](UNumber const __index) const noexcept {
        return Str[__index];
    }

    static bool Compare(wchar_t const *left, wchar_t const *right) noexcept { // Compare
        UNumber i        = 0;
        UNumber r_length = 0;
        while (right[r_length] != L'\0') {
            ++r_length;
        };

        UNumber l_length = 0;
        while (left[l_length] != L'\0') {
            ++l_length;
        };

        if (r_length != l_length) {
            return false;
        }

        while ((i < r_length) && (left[i] == right[i])) {
            ++i;
        }

        return (i == r_length);
    }

    static bool Compare(wchar_t const *src_text, UNumber const src_index, UNumber const src_length, wchar_t const *des_text,
                        UNumber des_index, UNumber const des_length) noexcept {
        if (src_length != des_length) {
            return false;
        }

        UNumber i = src_index;
        while ((i < src_length) && (src_text[i] == des_text[des_index])) {
            ++i; // Only increment when there is a match
            ++des_index;
        }

        return (i == src_length);
    }

    bool Compare(String const &text, UNumber index, UNumber const length) const noexcept {
        if (Length != length) {
            return false;
        }

        UNumber i = 0;
        while ((i < length) && (Str[i] == text[index])) {
            ++i; // Only increment when there is a match
            ++index;
        }

        return (i == Length);
    }

    static void Copy(String &des, wchar_t const *src_p, UNumber start_at, UNumber const ln) noexcept {
        UNumber i = 0;

        UNumber const newlen = (ln + des.Length);

        if ((des.Capacity < newlen) || (des.Capacity == 0)) {
            // Copy any existing characters
            des.Capacity = newlen;

            wchar_t *_tmp = des.Str;
            Memory::Allocate<wchar_t>(&des.Str, (des.Capacity + 1));

            while (i < des.Length) {
                des[i] = _tmp[i];
                ++i;
            }

            Memory::Deallocate<wchar_t>(&_tmp);
        }

        // Add the new characters
        for (i = 0; i < ln; i++) {
            des[start_at++] = src_p[i];
        }

        des[start_at] = L'\0'; // Null ending.
        des.Length    = newlen;
    }

    inline void SetLength(UNumber const _size) noexcept {
        Capacity = _size;

        if (Length != 0) {
            Length = 0;
            Memory::Deallocate<wchar_t>(&Str);
        }

        Memory::Allocate<wchar_t>(&Str, (_size + 1));

        Str[0] = L'\0';
    }

    inline void Resize(UNumber const _size) noexcept {
        Capacity     = _size;
        wchar_t *tmp = Str;

        Memory::Allocate<wchar_t>(&Str, (_size + 1));

        if (_size < Length) {
            Length = _size;
        }

        for (UNumber n = 0; n < Length; n++) {
            Str[n] = tmp[n];
        }

        Str[Length] = L'\0';

        Memory::Deallocate<wchar_t>(&tmp);
    }

    inline static String Part(wchar_t const *str, UNumber offset, UNumber const limit) noexcept {
        String bit;
        bit.Capacity = limit;

        Memory::Allocate<wchar_t>(&bit.Str, (limit + 1));

        while (bit.Length != limit) {
            bit[bit.Length++] = str[offset++];
        }

        bit[bit.Length] = L'\0'; // To mark the end of a string.

        return bit;
    }

    inline static UNumber Hash(wchar_t const *str, UNumber start, UNumber const end_offset) noexcept {
        UNumber hash = 0;
        UNumber j    = 1;
        UNumber i    = 0;
        bool    fl   = false;

        while (start < end_offset) {
            if (fl) {
                j *= (++i);
            } else {
                ++j;
            }

            fl = !fl;

            hash += (((static_cast<UNumber>(str[start++]))) * (j++));
        }

        return hash;
    }

    inline static void SoftTrim(wchar_t const *str, UNumber &start, UNumber &limit) noexcept {
        UNumber end = limit + start;

        while ((str[start] == L' ') || (str[start] == L'\n') || (str[start] == L'\t') || (str[start] == L'\r')) {
            ++start;
            --limit;
        }

        while ((end > start) && ((str[--end] == L' ') || (str[end] == L'\n') || (str[end] == L'\t') || (str[end] == L'\r'))) {
            --limit;
        }
    }

    static String Trim(String const &str) noexcept {
        UNumber limit  = str.Length;
        UNumber offset = 0;

        SoftTrim(str.Str, offset, limit);

        return Part(str.Str, offset, limit);
    }

    // Revers a string
    inline static void Revers(wchar_t *str, UShort index, UShort limit) noexcept {
        wchar_t ch;

        while (index < limit) {
            ch           = str[limit];
            str[limit--] = str[index];
            str[index++] = ch;
        }
    }

    static String FromNumber(unsigned long number, UShort const min = 1) noexcept {
        wchar_t p1_str[23];
        UShort  str1_len = 0;

        while (number != 0) {
            p1_str[str1_len] = wchar_t((number % 10) + 48);
            ++str1_len;
            number /= 10;
        }

        while (str1_len < min) {
            p1_str[str1_len] = L'0';
            ++str1_len;
        }

        p1_str[str1_len] = L'\0';
        Revers(&(p1_str[0]), 0, (--str1_len));
        return &(p1_str[0]);
    }

    inline static String FromNumber(UShort number, UShort const min = 1) noexcept {
        return FromNumber(static_cast<unsigned long>(number), min);
    }

    inline static String FromNumber(unsigned int number, UShort const min = 1) noexcept {
        return FromNumber(static_cast<unsigned long>(number), min);
    }

    static String FromNumber(double number, UShort const min = 1, UShort r_min = 0, UShort r_max = 0) noexcept {
        UShort str1_len = 0;
        UShort str2_len = 0;

        wchar_t p1_str[19];
        wchar_t p2_str[23];

        bool const negative  = (number < 0);
        UShort     presision = 15;

        if (negative) {
            number *= -1;
        }

        if (number != 0) {
            unsigned long long num  = static_cast<unsigned long long>(number);
            unsigned long long num2 = num;

            while (num != 0) {
                p1_str[str1_len] = wchar_t((num % 10) + 48);
                ++str1_len;
                num /= 10;
            }

            number -= static_cast<double>(num2);
            if (number != 0) {
                number *= 1e15;
                num = static_cast<unsigned long int>(number);

                if (num != 0) {
                    while (((num % 10) == 0) && (presision != 0)) {
                        --presision;
                        num /= 10;
                    }

                    if ((r_max != 0) && (r_max < presision)) {
                        ++r_max;

                        while (r_max < presision) {
                            --presision;
                            num /= 10;
                        }

                        if ((num % 10) >= 5) {
                            num /= 10;
                            ++num;
                        } else {
                            num /= 10;
                        }

                        --presision;

                        while (((num % 10) == 0) && (presision != 0)) {
                            --presision;
                            num /= 10;
                        }

                        if ((num == 1) && (presision == 0)) {
                            ++num2;
                            str1_len = 0;

                            while (num2 != 0) {
                                p1_str[str1_len] = wchar_t((num2 % 10) + 48);
                                ++str1_len;
                                num2 /= 10;
                            }
                        }
                    }

                    while (r_min > presision) {
                        p2_str[str2_len] = L'0';
                        ++str2_len;
                        --r_min;
                    }

                    while (presision != 0) {
                        p2_str[str2_len] = wchar_t((num % 10) + 48);
                        num /= 10;
                        ++str2_len;
                        --presision;
                    }
                }
            }
        }

        while (str1_len < min) {
            p1_str[str1_len] = L'0';
            ++str1_len;
        }

        if (negative) {
            p1_str[str1_len] = L'-';
            ++str1_len;
        }

        while (str2_len < r_min) {
            p2_str[str2_len] = L'0';
            ++str2_len;
        }

        if (str2_len != 0) {
            p2_str[str2_len] = L'.';
            ++str2_len;

            for (UShort i = 0; i < str1_len; i++) {
                p2_str[str2_len] = p1_str[i];
                ++str2_len;
            }

            p2_str[str2_len] = L'\0';
            Revers(&p2_str[0], 0, (--str2_len));
            return &p2_str[0];
        }

        p1_str[str1_len] = L'\0';
        Revers(&(p1_str[0]), 0, (--str1_len));
        return &(p1_str[0]);
    }

    static bool ToNumber(String const &str, UNumber &number, UNumber const offset = 0, UNumber limit = 0) noexcept {
        limit += offset;
        if (limit == offset) {
            limit = str.Length - offset;
        }

        wchar_t c;
        UNumber m        = 1;
        UNumber exp      = 0;
        bool    negative = false;

        number = 0;
        for (;;) {
            c = str[--limit];

            if ((c <= 47) || (c >= 58)) {
                if ((c == L'e') || (c == L'E')) {
                    exp    = number;
                    number = 0;
                    m      = 1;
                    continue;
                }

                if ((c == L'+') || (negative = (c == L'-'))) {
                    continue;
                }

                number = 0;
                return false;
            }

            number += ((static_cast<UNumber>(c) - 48) * m);

            if (limit == offset) {
                break;
            }

            m *= 10;
        }

        if (exp != 0) {
            if (!negative) {
                for (UNumber i = 0; i < exp; i++) {
                    number *= 10;
                }
            } else {
                for (UNumber i = 0; i < exp; i++) {
                    number /= 10;
                }
            }
        }

        return true;
    }

    // TODO: rewrite
    static bool ToNumber(String const &str, double &number, UNumber const offset = 0, UNumber limit = 0) noexcept {
        limit += offset;
        if (limit == offset) {
            limit = str.Length - offset;
        }

        wchar_t c;
        double  m        = 1.0;
        UNumber exp      = 0;
        bool    negative = false;

        number = 0.0;
        for (;;) {
            c = str[--limit];

            if ((c <= 47) || (c >= 58)) {
                if (c == L'+') {
                    continue;
                }

                if (c == L'-') {
                    if (limit != offset) {
                        negative = true;
                        continue;
                    }

                    number *= -1.0;
                    break;
                }

                if ((c == L'e') || (c == L'E')) {
                    exp    = static_cast<UNumber>(number);
                    number = 0.0;
                    m      = 1.0;
                    continue;
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

        if (exp != 0) {
            if (!negative) {
                for (UNumber i = 0; i < exp; i++) {
                    number *= 10;
                }
            } else {
                for (UNumber i = 0; i < exp; i++) {
                    number /= 10;
                }
            }
        }

        return true;
    }
};

} // namespace Qentem

#endif
