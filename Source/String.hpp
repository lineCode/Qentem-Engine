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
    UNumber Length{0};
    char *  Str{nullptr}; // NULL terminated char
    UNumber Capacity{0};

    explicit String() = default;

    explicit String(const UNumber capacity) noexcept : Capacity(capacity) {
        Memory::Allocate<char>(&Str, (Capacity + 1));
    }

    explicit String(const char *str, const UNumber capacity) noexcept : Capacity(capacity) {
        Memory::Allocate<char>(&Str, (Capacity + 1));

        while (Length < Capacity) {
            Str[Length] = str[Length];
            ++Length;
        }

        Str[Length] = '\0';
    }

    String(const char *str) noexcept : Capacity(Count(str)) {
        Memory::Allocate<char>(&Str, (Capacity + 1));

        while (Length < Capacity) {
            Str[Length] = str[Length];
            ++Length;
        }

        Str[Length] = '\0';
    }

    String(String &&src) noexcept : Length(src.Length), Str(src.Str), Capacity(src.Capacity) {
        src.Length   = 0;
        src.Str      = nullptr;
        src.Capacity = 0;
    }

    explicit String(const String &src) noexcept : Capacity(src.Length) {
        if (Capacity != 0) {
            Memory::Allocate<char>(&Str, (Capacity + 1));

            for (; Length < Capacity;) {
                Str[Length] = src[Length];
                ++Length;
            }

            Str[Length] = '\0';
        }
    }

    ~String() noexcept {
        Memory::Deallocate<char>(&Str);
    }

    static UNumber Count(const char *str) noexcept {
        UNumber length = 0;
        while (str[length] != '\0') {
            ++length;
        };

        return length;
    }

    void Reset() noexcept {
        Memory::Deallocate<char>(&Str);

        Length   = 0;
        Capacity = 0;
    }

    char *Eject() noexcept {
        char *str = Str;
        Str       = nullptr;
        Length    = 0;
        Capacity  = 0;

        return str;
    }

    String &operator=(const char *str) noexcept {
        const UNumber length = Count(str);

        if ((Capacity == 0) || (Capacity < length)) {
            Memory::Deallocate<char>(&Str);
            Length   = 0;
            Capacity = length;
            Memory::Allocate<char>(&Str, (Capacity + 1));
        }

        while (Length < Capacity) {
            Str[Length] = str[Length];
            ++Length;
        }

        Str[Length] = '\0';

        return *this;
    }

    String &operator=(const String &src) noexcept {
        if (this != &src) {
            if ((Capacity == 0) || (Capacity < src.Length)) {
                Memory::Deallocate<char>(&Str);
                Length   = 0;
                Capacity = src.Length;
                Memory::Allocate<char>(&Str, (Capacity + 1));
            }

            while (Length < Capacity) {
                Str[Length] = src[Length];
                ++Length;
            }

            Str[Length] = '\0';
        }

        return *this;
    }

    String &operator=(String &&src) noexcept {
        if (this != &src) {
            if (Str != nullptr) {
                Memory::Deallocate<char>(&Str);
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

    String &operator+=(const char *str) noexcept {
        Appand(*this, str, Length, Count(str));

        return *this;
    }

    String &operator+=(String &&src) noexcept {
        if (src.Length != 0) {
            Appand(*this, src.Str, Length, src.Length);

            Memory::Deallocate<char>(&src.Str);

            src.Length   = 0;
            src.Str      = nullptr;
            src.Capacity = 0;
        }

        return *this;
    }

    String &operator+=(const String &src) noexcept {
        if (src.Length != 0) {
            Appand(*this, src.Str, Length, src.Length);
        }

        return *this;
    }

    String operator+(const char *str) const noexcept {
        const UNumber length = Count(str);

        String  ns(Length + length);
        UNumber i = 0;

        for (; i < Length; i++) {
            ns[i] = Str[i];
        }
        ns.Length = Length;

        for (i = 0; i < length; i++) {
            ns[ns.Length] = str[i];
            ++ns.Length;
        }

        ns[ns.Length] = '\0';

        return ns;
    }

    String operator+(String &&src) const noexcept {
        String  ns(Length + src.Length);
        UNumber i = 0;

        for (; i < Length; i++) {
            ns[i] = Str[i];
        }
        ns.Length = Length;

        for (i = 0; i < src.Length; i++) {
            ns[ns.Length] = src.Str[i];
            ++ns.Length;
        }

        ns[ns.Length] = '\0';

        Memory::Deallocate<char>(&src.Str);
        src.Capacity = 0;

        return ns;
    }

    String operator+(const String &src) const noexcept {
        String  ns(Length + src.Length);
        UNumber i = 0;

        for (; i < Length; i++) {
            ns[i] = Str[i];
        }
        ns.Length = Length;

        for (i = 0; i < src.Length; i++) {
            ns[ns.Length] = src.Str[i];
            ++ns.Length;
        }

        ns[ns.Length] = '\0';

        return ns;
    }

    bool operator==(const char *str) const noexcept {
        const UNumber length = Count(str);

        if (Length != length) {
            return false;
        }

        UNumber i = 0;
        while ((i < Length) && (Str[i] == str[i])) {
            ++i;
        }

        return (i == Length);
    }

    inline bool operator!=(const char *string) const noexcept {
        return (!(*this == string));
    }

    bool operator==(const String &string) const noexcept {
        if (Length != string.Length) {
            return false;
        }

        UNumber i = 0;
        while ((i < Length) && (Str[i] == string[i])) {
            ++i;
        }

        return (i == Length);
    }

    inline bool operator!=(const String &string) const noexcept {
        return (!(*this == string));
    }

    inline char &operator[](const UNumber index) const noexcept {
        return Str[index];
    }

    static bool Compare(const char *left, const char *right) noexcept {
        const UNumber l_length = Count(left);
        const UNumber r_length = Count(right);

        if (r_length != l_length) {
            return false;
        }

        UNumber i = 0;
        while ((i < r_length) && (left[i] == right[i])) {
            ++i;
        }

        return (i == r_length);
    }

    static bool Compare(const char *src_text, UNumber src_index, const UNumber src_length, const char *des_text, UNumber des_index,
                        const UNumber des_length) noexcept {
        if (src_length != des_length) {
            return false;
        }

        UNumber i = 0;
        while ((i < src_length) && (src_text[src_index++] == des_text[des_index++])) {
            ++i; // Only increment when there is a match
        }

        return (i == src_length);
    }

    bool Compare(const String &text, UNumber index, const UNumber length) const noexcept {
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

    static void Appand(String &des, const char *src_p, UNumber start_at, const UNumber ln) noexcept {
        UNumber i = 0;

        const UNumber newlen = (ln + des.Length);

        if ((des.Capacity < newlen) || (des.Capacity == 0)) {
            // Copy any existing characters
            des.Capacity = newlen;

            char *tmp = des.Str;
            Memory::Allocate<char>(&des.Str, (des.Capacity + 1));

            while (i < des.Length) {
                des[i] = tmp[i];
                ++i;
            }

            Memory::Deallocate<char>(&tmp);
        }

        // Add characters
        for (i = 0; i < ln; i++) {
            des[start_at++] = src_p[i];
        }

        des[start_at] = '\0'; // Null ending.
        des.Length    = newlen;
    }

    void SetLength(const UNumber size) noexcept {
        Capacity = size;

        if (Length != 0) {
            Length = 0;
            Memory::Deallocate<char>(&Str);
        }

        Memory::Allocate<char>(&Str, (size + 1));

        Str[0] = '\0';
    }

    static String Part(const char *str, UNumber offset, const UNumber limit) noexcept {
        String s_part(limit);

        while (s_part.Length != limit) {
            s_part[s_part.Length++] = str[offset++];
        }

        s_part[s_part.Length] = '\0'; // To mark the end of a string.

        return s_part;
    }

    static UNumber Hash(const char *str, UNumber offset, UNumber limit) noexcept {
        UNumber hash = 0;
        UNumber base = 1;

        while (limit != 0) {
            hash += (static_cast<UNumber>(str[offset++]) + base);
            base += 256;
            --limit;
        }

        return hash;
    }

    static void SoftTrim(const char *str, UNumber &offset, UNumber &limit) noexcept {
        UNumber end = limit + offset;

        while ((str[offset] == ' ') || (str[offset] == '\n') || (str[offset] == '\t') || (str[offset] == '\r')) {
            ++offset;
        }

        while ((--end > offset) && ((str[end] == ' ') || (str[end] == '\n') || (str[end] == '\t') || (str[end] == '\r'))) {
        }
        ++end;

        limit = end - offset;
    }

    static String Trim(const String &str) noexcept {
        UNumber limit  = str.Length;
        UNumber offset = 0;

        SoftTrim(str.Str, offset, limit);

        return Part(str.Str, offset, limit);
    }

    static String FromNumber(unsigned long number, UShort min = 1) noexcept {
        static constexpr UShort num_len = 20;

        UShort len = num_len;
        char   str[num_len];

        while (number != 0) {
            str[--len] = char((number % 10) + 48);
            number /= 10;
        }

        min = static_cast<UShort>(num_len - min);

        while (len > min) {
            str[--len] = '0';
        }

        return String(&(str[len]), static_cast<UNumber>(num_len - len));
    }

    static String FromNumber(double number, UShort min = 1, UShort r_min = 0, UShort r_max = 0) noexcept {
        static constexpr UNumber num_len = 22;

        UShort len    = num_len;
        UShort p1_len = 0;

        char str[num_len];

        const bool negative = (number < 0.0);

        if (negative) {
            number *= -1;
        }

        if (number != 0) {
            unsigned long left = static_cast<unsigned long>(number);
            number -= static_cast<double>(left);

            if (number != 0) {
                number *= 1e15;
                unsigned long right = static_cast<unsigned long>(number);

                if (right != 0) {
                    UShort presision = 15;
                    while (((right % 10) == 0) && (presision != 0)) {
                        --presision;
                        right /= 10;
                    }

                    if ((r_max != 0) && (r_max < presision)) {
                        ++r_max;

                        while (r_max < presision) {
                            --presision;
                            right /= 10;
                        }

                        if ((right % 10) >= 5) {
                            right /= 10;
                            ++right;
                        } else {
                            right /= 10;
                        }

                        --presision;

                        while (((right % 10) == 0) && (presision != 0)) {
                            --presision;
                            right /= 10;
                        }

                        if ((right == 1) && (presision == 0)) {
                            ++left;
                        }
                    }

                    while (r_min > presision) {
                        str[--len] = '0';
                        --r_min;
                    }
                    r_min = 0;

                    while (presision != 0) {
                        str[--len] = char((right % 10) + 48);
                        right /= 10;
                        --presision;
                    }

                    if (len != num_len) {
                        str[--len] = '.';
                    }
                }
            }

            while (left != 0) {
                str[--len] = char((left % 10) + 48);
                left /= 10;
                ++p1_len;
            }
        }

        if (r_min != 0) {
            do {
                str[--len] = '0';
            } while (--r_min != 0);

            str[--len] = '.';
        }

        while (p1_len < min) {
            str[--len] = '0';
            ++p1_len;
        }

        if (negative) {
            str[--len] = '-';
        }

        return String(&(str[len]), (num_len - len));
    }

    inline static String FromNumber(UShort number, const UShort min = 1) noexcept {
        return FromNumber(static_cast<unsigned long>(number), min);
    }

    inline static String FromNumber(unsigned int number, const UShort min = 1) noexcept {
        return FromNumber(static_cast<unsigned long>(number), min);
    }

    inline static String FromNumber(int number, const UShort min = 1) noexcept {
        return FromNumber(static_cast<double>(number), min, 0, 0);
    }

    inline static String FromNumber(long number, const UShort min = 1) noexcept {
        return FromNumber(static_cast<double>(number), min, 0, 0);
    }

    static bool ToNumber(UNumber &number, const char *str, const UNumber offset, UNumber limit) noexcept {
        limit += offset;
        number = 0;

        UNumber exp     = 0;
        UNumber postion = 1;
        char    c;

        while (limit != offset) {
            c = str[--limit];

            if ((c <= 47) || (c >= 58)) {
                switch (c) {
                    case 'e':
                    case 'E': {
                        exp     = number;
                        number  = 0;
                        postion = 1;
                        break;
                    }
                    case '+':
                        break;
                    default:
                        number = 0.0;
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

    static bool ToNumber(double &number, const char *str, const UNumber offset, UNumber limit) noexcept {
        number = 0.0;
        limit += offset;

        bool    negative_exp = false;
        UNumber exp          = 0;
        double  postion      = 1.0;
        char    c;

        while (limit != offset) {
            c = str[--limit];

            if ((c <= 47) || (c >= 58)) {
                switch (c) {
                    case '.': {
                        number /= postion;
                        postion = 1.0;
                        break;
                    }
                    case 'e':
                    case 'E': {
                        exp     = static_cast<UNumber>(number);
                        number  = 0.0;
                        postion = 1.0;
                        break;
                    }
                    case '+':
                        break;
                    case '-': {
                        if (limit != offset) {
                            negative_exp = true;
                        } else {
                            number *= -1.0;
                        }
                        break;
                    }
                    default:
                        number = 0.0;
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
