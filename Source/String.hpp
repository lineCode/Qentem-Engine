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
    UNumber  Length{0};
    wchar_t *Str{nullptr}; // NULL terminated wchar_t
    UNumber  Capacity{0};

    explicit String() = default;

    explicit String(UNumber const _capacity) noexcept : Capacity(_capacity) {
        Memory::Allocate<wchar_t>(&Str, (Capacity + 1));
    }

    explicit String(wchar_t const *str, UNumber const _capacity) noexcept : Capacity(_capacity) {
        Memory::Allocate<wchar_t>(&Str, (Capacity + 1));

        while (Length < Capacity) {
            Str[Length] = str[Length];
            ++Length;
        }

        Str[Length] = L'\0';
    }

    String(char const *str) noexcept {
        while (str[Capacity] != '\0') {
            ++Capacity;
        };

        Memory::Allocate<wchar_t>(&Str, (Capacity + 1));

        while (Length < Capacity) {
            Str[Length] = static_cast<wchar_t>(str[Length]);
            ++Length;
        }

        Str[Length] = L'\0';
    }

    String(wchar_t const *str) noexcept : Capacity(Count(str)) {
        Memory::Allocate<wchar_t>(&Str, (Capacity + 1));

        while (Length < Capacity) {
            Str[Length] = str[Length];
            ++Length;
        }

        Str[Length] = L'\0';
    }

    String(String &&src) noexcept : Length(src.Length), Str(src.Str), Capacity(src.Capacity) { // Move
        src.Length   = 0;
        src.Str      = nullptr;
        src.Capacity = 0;
    }

    explicit String(String const &src) noexcept : Capacity(src.Length) { // Copy
        if (Capacity != 0) {
            Memory::Allocate<wchar_t>(&Str, (Capacity + 1));

            for (; Length < Capacity;) {
                Str[Length] = src[Length];
                ++Length;
            }

            Str[Length] = L'\0';
        }
    }

    ~String() noexcept {
        Memory::Deallocate<wchar_t>(&Str);
    }

    static UNumber Count(wchar_t const *str) noexcept {
        UNumber _length = 0;
        while (str[_length] != L'\0') {
            ++_length;
        };

        return _length;
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

    String &operator=(wchar_t const *str) noexcept { // Copy
        if (Str != nullptr) {
            Memory::Deallocate<wchar_t>(&Str);
            Length = 0;
        }

        Capacity = Count(str);

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
            if (Str != nullptr) {
                Memory::Deallocate<wchar_t>(&Str);
            }

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
            if (Str != nullptr) {
                Memory::Deallocate<wchar_t>(&Str);
                Length = 0;
            }

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

    String &operator+=(wchar_t const *str) noexcept { // Appand a string
        Copy(*this, str, this->Length, Count(str));

        return *this;
    }

    String &operator+=(String &&src) noexcept { // Move
        if (src.Length != 0) {
            Copy(*this, src.Str, Length, src.Length);

            Memory::Deallocate<wchar_t>(&src.Str);

            src.Length   = 0;
            src.Str      = nullptr;
            src.Capacity = 0;
        }

        return *this;
    }

    inline String &operator+=(String const &src) noexcept { // Appand a string
        if (src.Length != 0) {
            Copy(*this, src.Str, Length, src.Length);
        }

        return *this;
    }

    String operator+(wchar_t const *str) const noexcept {
        UNumber const _length = Count(str);

        String ns(Length + _length);

        Copy(ns, Str, 0, Length);
        Copy(ns, str, ns.Length, _length);

        return ns;
    }

    // Appand a string by moving another into it
    String operator+(String &&src) const noexcept {
        String ns(Length + src.Length);
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
        String ns(Length + src.Length);

        Copy(ns, Str, 0, Length);

        if (src.Length != 0) {
            Copy(ns, src.Str, ns.Length, src.Length);
        }

        return ns;
    }

    bool operator==(wchar_t const *str) const noexcept { // Compare
        UNumber const _length = Count(str);

        if (Length != _length) {
            return false;
        }

        UNumber i = 0;
        while ((i < Length) && (Str[i] == str[i])) {
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
        UNumber const l_length = Count(left);
        UNumber const r_length = Count(right);

        if (r_length != l_length) {
            return false;
        }

        UNumber i = 0;
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

    inline static String Part(wchar_t const *str, UNumber offset, UNumber const limit) noexcept {
        String _part(limit);

        while (_part.Length != limit) {
            _part[_part.Length++] = str[offset++];
        }

        _part[_part.Length] = L'\0'; // To mark the end of a string.

        return _part;
    }

    inline static UNumber Hash(wchar_t const *str, UNumber offset, UNumber limit) noexcept {
        UNumber hash = 0;
        UNumber base = 1;

        while (limit != 0) {
            hash += (static_cast<UNumber>(str[offset++]) + base);
            base += 256;
            --limit;
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

    static String FromNumber(unsigned long number, UShort min = 1) noexcept {
        static UShort constexpr num_len = 20;

        UShort  __len = num_len;
        wchar_t _str[num_len];

        while (number != 0) {
            _str[--__len] = wchar_t((number % 10) + 48);
            number /= 10;
        }

        min = static_cast<UShort>(num_len - min);

        while (__len > min) {
            _str[--__len] = L'0';
        }

        return String(&(_str[__len]), static_cast<UNumber>(num_len - __len));
    }

    static String FromNumber(double number, UShort min = 1, UShort r_min = 0, UShort r_max = 0) noexcept {
        static UNumber constexpr num_len = 22;

        UShort __len  = num_len;
        UShort p1_len = 0;

        wchar_t _str[num_len];

        bool const negative = (number < 0);

        if (negative) {
            number *= -1;
        }

        if (number != 0) {
            unsigned long num       = static_cast<unsigned long>(number);
            unsigned long num2      = num;
            UShort        presision = 15;

            number -= static_cast<double>(num2);
            if (number != 0) {
                number *= 1e15;
                num = static_cast<unsigned long>(number);

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
                        }
                    }

                    while (r_min > presision) {
                        _str[--__len] = L'0';
                        --r_min;
                    }
                    r_min = 0;

                    while (presision != 0) {
                        _str[--__len] = wchar_t((num % 10) + 48);
                        num /= 10;
                        --presision;
                    }

                    if (__len != num_len) {
                        _str[--__len] = L'.';
                    }
                }
            }

            while (num2 != 0) {
                _str[--__len] = wchar_t((num2 % 10) + 48);
                num2 /= 10;
                ++p1_len;
            }
        }

        if (r_min != 0) {
            do {
                _str[--__len] = L'0';
            } while (--r_min != 0);

            _str[--__len] = L'.';
        }

        while (p1_len < min) {
            _str[--__len] = L'0';
            ++p1_len;
        }

        if (negative) {
            _str[--__len] = L'-';
        }

        return String(&(_str[__len]), (num_len - __len));
    }

    inline static String FromNumber(UShort number, UShort const min = 1) noexcept {
        return FromNumber(static_cast<unsigned long>(number), min);
    }

    inline static String FromNumber(unsigned int number, UShort const min = 1) noexcept {
        return FromNumber(static_cast<unsigned long>(number), min);
    }

    inline static String FromNumber(int number, UShort const min = 1) noexcept {
        return FromNumber(static_cast<double>(number), min, 0, 0);
    }

    inline static String FromNumber(long number, UShort const min = 1) noexcept {
        return FromNumber(static_cast<double>(number), min, 0, 0);
    }

    static bool ToNumber(UNumber &number, wchar_t const *str, UNumber const offset, UNumber limit) noexcept {
        limit += offset;
        number = 0;

        UNumber exp     = 0;
        UNumber postion = 1;
        wchar_t c;

        while (limit != offset) {
            c = str[--limit];

            if ((c <= 47) || (c >= 58)) {
                switch (c) {
                    case L'e':
                    case L'E': {
                        exp     = number;
                        number  = 0;
                        postion = 1;
                        break;
                    }
                    case L'+':
                        break;
                    default:
                        return false;
                }
            } else {
                number += ((static_cast<UNumber>(c) - 48) * postion);
                postion *= 10;
            }
        }

        while (exp != 0) {
            number *= 10;
            --exp;
        }

        return (postion > 1);
    }

    static bool ToNumber(double &number, wchar_t const *str, UNumber const offset, UNumber limit) noexcept {
        number = 0.0;
        limit += offset;

        bool    negative_exp = false;
        UNumber exp          = 0;
        double  postion      = 1.0;
        wchar_t c;

        while (limit != offset) {
            c = str[--limit];

            if ((c <= 47) || (c >= 58)) {
                switch (c) {
                    case L'.': {
                        number /= postion;
                        postion = 1.0;
                        break;
                    }
                    case L'e':
                    case L'E': {
                        exp     = static_cast<UNumber>(number);
                        number  = 0.0;
                        postion = 1.0;
                        break;
                    }
                    case L'+':
                        break;
                    case L'-': {
                        if (limit != offset) {
                            negative_exp = true;
                        } else {
                            number *= -1.0;
                        }
                        break;
                    }
                    default:
                        return false;
                }
            } else {
                number += ((static_cast<double>(c) - 48) * postion);
                postion *= 10;
            }
        }

        if (negative_exp) {
            while (exp != 0) {
                number /= 10;
                --exp;
            }
        } else {
            while (exp != 0) {
                number *= 10;
                --exp;
            }
        }

        return (postion > 1.0);
    }
};

} // namespace Qentem

#endif
