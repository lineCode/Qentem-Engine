
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
    wchar_t *Str      = nullptr; // NULL terminated wchar_t
    UNumber  Length   = 0;
    UNumber  Capacity = 0;

    String() = default;

    String(wchar_t const str) noexcept { // one wchar
        Capacity = 1;

        Memory<wchar_t>::Allocate(&Str, 2); // 1 for /0

        if (str != L'\0') {
            Str[Length++] = str;
        }

        Str[Length] = L'\0';
    }

    String(char const str) noexcept { // one char
        Length = Capacity = 1;

        Memory<wchar_t>::Allocate(&Str, 2);

        if (str != L'\0') {
            Str[Length++] = static_cast<wchar_t>(str);
        }

        Str[Length] = L'\0';
    }

    String(wchar_t const *str) noexcept {
        UNumber _length = Count(str);

        Capacity = _length;

        Memory<wchar_t>::Allocate(&Str, (_length + 1));

        for (; Length < _length; Length++) {
            Str[Length] = str[Length];
        }

        Str[Length] = L'\0';
    }

    String(char const *str) noexcept {
        UNumber _length = 0;
        while (str[_length] != L'\0') {
            ++_length;
        };

        Capacity = _length;

        Memory<wchar_t>::Allocate(&Str, (_length + 1));

        for (; Length < _length; Length++) {
            Str[Length] = static_cast<wchar_t>(str[Length]);
        }

        Str[Length] = L'\0';
    }

    String(String &&src) noexcept { // Move
        if (src.Length != 0) {
            Str      = src.Str;
            Capacity = src.Capacity;
            Length   = src.Length;

            src.Capacity = 0;
            src.Length   = 0;
            src.Str      = nullptr;

            return;
        }

        Capacity = 1;
        Memory<wchar_t>::Allocate(&Str, 1); // 1 for /0
        Str[Length] = L'\0';
    }

    String(String const &src) noexcept { // Copy
        Capacity = src.Length;

        Memory<wchar_t>::Allocate(&Str, (Capacity + 1));

        for (; Length < src.Length; Length++) {
            Str[Length] = static_cast<wchar_t>(src[Length]);
        }

        Str[Length] = L'\0';
    }

    ~String() noexcept {
        Reset();
    }

    inline static UNumber Count(wchar_t const *str) {
        UNumber _length = 0;
        while (str[_length] != L'\0') {
            ++_length;
        };

        return _length;
    }

    inline void Reset() noexcept {
        Memory<wchar_t>::Deallocate(&Str);

        Capacity = 0;
        Length   = 0;
    }

    inline wchar_t *Eject() noexcept {
        wchar_t *_str = Str;
        Str           = nullptr;
        Capacity      = 0;
        Length        = 0;
        return _str;
    }

    String &operator=(wchar_t const str) noexcept { // Copy
        Length = 0;

        if (Capacity == 0) {
            Resize(1);
        }

        if (str != L'\0') {
            Str[Length++] = str;
        }

        Str[Length] = L'\0';

        return *this;
    }

    String &operator=(wchar_t const *str) noexcept { // Copy
        UNumber _length = Count(str);

        Copy(*this, str, 0, _length);

        return *this;
    }

    String &operator=(String &&src) noexcept { // Move
        if (this != &src) {
            Memory<wchar_t>::Deallocate(&Str);

            Str      = src.Str;
            Capacity = src.Capacity;
            Length   = src.Length;

            src.Capacity = 0;
            src.Length   = 0;
            src.Str      = nullptr;
        }

        return *this;
    }

    String &operator=(String const &src) noexcept { // Copy
        if (this != &src) {
            if (Capacity < src.Length) {
                Capacity = src.Length;
                Memory<wchar_t>::Deallocate(&Str);
                Memory<wchar_t>::Allocate(&Str, (Capacity + 1));

                Length = 0;
                for (; Length < src.Length;) {
                    Str[Length] = src[Length];
                    ++Length;
                }
                Str[Length] = L'\0';
            } else {
                Length = 0;
                Copy(*this, src.Str, Length, src.Length);
            }
        }

        return *this;
    }

    String &operator+=(wchar_t const src) noexcept { // Appand a string
        if (Length == Capacity) {
            if (Capacity == 0) {
                Capacity = 2;
            }

            Resize(Capacity * 2);
        }

        Str[Length]   = src;
        Str[++Length] = L'\0';

        return *this;
    }

    String &operator+=(wchar_t const *str) noexcept { // Appand a string
        UNumber _length = Count(str);

        Copy(*this, str, this->Length, _length);

        return *this;
    }

    String &operator+=(String &&src) noexcept { // Move
        Copy(*this, src.Str, Length, src.Length);

        Memory<wchar_t>::Deallocate(&src.Str);

        src.Str = nullptr;

        src.Capacity = 0;
        src.Length   = 0;

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
        String ns;

        UNumber _length = Count(str);

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
        Copy(ns, src.Str, ns.Length, src.Length);

        Memory<wchar_t>::Deallocate(&src.Str);

        src.Capacity = 0;
        src.Length   = 0;

        return ns;
    }

    String operator+(String const &src) const noexcept { // Appand a string and return a new one
        String ns;

        ns.SetLength(Length + src.Length);
        Copy(ns, Str, 0, Length);
        Copy(ns, src.Str, ns.Length, src.Length);

        return ns;
    }

    bool operator==(String const &src) const noexcept { // Compare
        if (Length != src.Length) {
            return false;
        }

        UNumber i = 0;
        while ((i < Length) && (Str[i] == src[i])) {
            ++i;
        }

        return (i == Length);
    }

    bool operator!=(String const &src) const noexcept { // Compare
        return (!(*this == src));
    }

    inline wchar_t &operator[](UNumber const __index) const noexcept {
        return Str[__index];
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
        // Copy any existing characters
        UNumber j = 0;
        if ((des.Capacity == 0) || (ln > (des.Capacity - des.Length))) {
            des.Capacity = (ln + des.Length);

            wchar_t *_tmp = des.Str;
            Memory<wchar_t>::Allocate(&des.Str, (des.Capacity + 1));

            for (UNumber i = 0; i < des.Length;) {
                des[i] = _tmp[j];
                ++j;
                ++i;
            }

            Memory<wchar_t>::Deallocate(&_tmp);
        }

        // Add the new characters
        for (j = 0; j < ln;) {
            des[start_at] = src_p[j];
            ++start_at;
            ++j;
        }

        des[start_at] = L'\0'; // Null ending.

        // Update the index to be at the last character
        des.Length = (ln + des.Length);
    }

    inline void SetLength(UNumber const _size) noexcept {
        Length   = 0;
        Capacity = _size;

        Memory<wchar_t>::Deallocate(&Str);
        Memory<wchar_t>::Allocate(&Str, (_size + 1));

        Str[0] = L'\0';
    }

    inline void Resize(UNumber const _size) noexcept {
        Capacity     = _size;
        wchar_t *tmp = Str;

        Memory<wchar_t>::Allocate(&Str, (_size + 1));

        if (_size < Length) {
            Length = _size;
        }

        for (UNumber n = 0; n < Length; n++) {
            Str[n] = tmp[n];
        }

        Str[Length] = L'\0';

        Memory<wchar_t>::Deallocate(&tmp);
    }

    inline static String Part(wchar_t const *str, UNumber offset, UNumber const limit) noexcept {
        String bit;

        Memory<wchar_t>::Allocate(&bit.Str, (limit + 1));

        bit.Capacity = limit;

        while (bit.Length < limit) {
            bit[bit.Length++] = str[offset++];
        }

        bit[bit.Length] = L'\0'; // To mark the end of a string.

        return bit;
    }

    inline static UNumber Hash(wchar_t const *str, UNumber start, UNumber const end_offset) noexcept {
        UNumber j    = 1;
        UNumber hash = 0;
        bool    fl   = false;

        for (UNumber i = 0; start < end_offset;) {
            if (fl) {
                j *= (++i);
            } else {
                ++j;
            }

            fl = !fl;

            hash += (((static_cast<UNumber>(str[start++]))) * j++);
        }

        return hash;
    }

    inline static void SoftTrim(wchar_t const *str, UNumber &start, UNumber &limit) noexcept {
        UNumber end = limit + start;

        while ((str[start] == L' ') || (str[start] == L'\n') || (str[start] == L'\r') || (str[start] == L'\t')) {
            ++start;
            --limit;
        }

        while ((end > start) &&
               ((str[--end] == L' ') || (str[end] == L'\n') || (str[end] == L'\r') || (str[end] == L'\t'))) {
            --limit;
        }
    }

    // Remove empty spaces
    static String Trim(String const &str) noexcept {
        UNumber offset = 0;
        UNumber limit  = str.Length;

        SoftTrim(str.Str, offset, limit);

        return Part(str.Str, offset, limit);
    }

    // Revers a string
    inline static void Revers(wchar_t *str, UNumber index, UNumber limit) noexcept {
        wchar_t ch;

        for (UNumber i = index; i < limit;) {
            ch         = str[limit];
            str[limit] = str[i];
            str[i]     = ch;
            --limit;
            ++i;
        }
    }

    static String FromNumber(UNumber number, UNumber const min = 1) noexcept {
        wchar_t p1_str[41]; // 39 + "sign" + "\0"
        UNumber str1_len = 0;

        while (number != 0) {
            p1_str[str1_len++] = wchar_t((number % 10) + 48);
            number /= 10;
        }

        while (str1_len < min) {
            p1_str[str1_len++] = L'0';
        }

        p1_str[str1_len] = L'\0';
        Revers(&(p1_str[0]), 0, (str1_len - 1));
        return &(p1_str[0]);
    }

    static String FromNumber(double number, UNumber const min = 1, UNumber r_min = 0, UNumber r_max = 0) noexcept {
        UNumber str1_len = 0;
        UNumber str2_len = 0;

        wchar_t p2_str[41]; // 38 + "sign" + " . "+ "\0"
        wchar_t p1_str[40]; // No "." needed.

        bool const negative = (number < 0);
        if (negative) {
            number *= -1;
        }

        if (number != 0) {
            UNumber            counter = 0;
            unsigned long long num     = static_cast<unsigned long long>(number);
            unsigned long long num2    = num;

            while (num != 0) {
                p1_str[str1_len++] = wchar_t((num % 10) + 48);
                num /= 10;
                ++counter;
            }

            number -= static_cast<double>(num2);
            if (number != 0) {
                UNumber num3 = 0; // For the reminder of 10

                number += 1.1; // Forcing the value to round up to it's original number.
                // The 0.1 is to prevent any leading zeros from being on the left
                number *= 1e15; // Moving everyting to the left.
                number -= 1e15; // Taking 1 back.

                num         = static_cast<UNumber>(number);
                UNumber len = 14 - ((counter > 1) ? (counter - 1) : 0);
                UNumber fnm = (num % 10);

                number -= static_cast<double>(num);
                if ((number >= 0.5) && (fnm >= 5)) {
                    num += (10 - fnm); // Yep
                }

                while (counter > 1) {
                    num /= 10;
                    --counter;
                }

                fnm = num3 = (num % 10);
                while ((num3 == 0) && (len > 0)) {
                    --len;
                    num /= 10;
                    num3 = (num % 10);
                }

                if ((num3 == 9) && (fnm >= 5) && (number >= 0.5)) {
                    ++num;
                    while (((num % 10) == 0) && (len > 0)) {
                        --len;
                        num /= 10;
                    }
                }

                if (num != 1) {
                    if (num != 10) { // Means .00000000000
                        if (r_max != 0) {
                            while (r_max < len) {
                                --len;
                                num /= 10;
                            }

                            if ((num % 10) >= 5) {
                                num /= 10;
                                ++num;
                            } else {
                                num /= 10;
                            }

                            --len;
                        }

                        for (UNumber w = 0; w < len;) {
                            p2_str[str2_len++] = wchar_t((num % 10) + 48);

                            num /= 10;
                            if (num == 0) {
                                break;
                            }

                            ++w;
                        }
                    }

                    if (num != 11) {
                        p2_str[str2_len++] = wchar_t(((num - 1) % 10) + 48); // (num - 1) taking 0.1 back.
                    } else {
                        str1_len = 0;
                        str2_len = 0;

                        ++num2;
                        while (num2 != 0) {
                            p1_str[str1_len++] = wchar_t((num2 % 10) + 48);
                            num2 /= 10;
                        }
                    }
                }
            }
        }

        while (str1_len < min) {
            p1_str[str1_len++] = L'0';
        }

        if (negative) {
            p1_str[str1_len++] = L'-';
        }

        while (str2_len < r_min) {
            p2_str[str2_len++] = L'0';
        }

        if (str2_len != 0) {
            p2_str[str2_len++] = L'.';

            for (UNumber i = 0; i < str1_len; i++) {
                p2_str[str2_len++] = p1_str[i];
            }

            p2_str[str2_len] = L'\0';
            Revers(&p2_str[0], 0, (str2_len - 1));
            return &p2_str[0];
        }

        p1_str[str1_len] = L'\0';
        Revers(&(p1_str[0]), 0, (str1_len - 1));
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
