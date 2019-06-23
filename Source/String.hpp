
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

#include "Memory.hpp"

namespace Qentem {

struct String {
    wchar_t *Str      = nullptr; // NULL terminated wchar_t
    UNumber  Length   = 0;
    UNumber  Capacity = 0;
    bool     Shared   = false;

    String() = default;

    static void Copy(String &des, const wchar_t *src_p, UNumber start_at, const UNumber ln) noexcept {
        // Copy any existing characters
        UNumber j = 0;
        if ((des.Capacity == 0) || (ln > (des.Capacity - des.Length))) {
            des.Capacity = (ln + des.Length);

            wchar_t *_tmp = des.Str;
            Memory::Allocate<wchar_t>(&des.Str, (des.Capacity + 1));

            for (UNumber i = 0; i < des.Length;) {
                des[i] = _tmp[j];
                ++j;
                ++i;
            }

            Memory::Deallocate<wchar_t>(&_tmp);
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

    String(const wchar_t str) noexcept { // one wchar
        if (str != L'\0') {
            Length = Capacity = 1;

            Memory::Allocate<wchar_t>(&Str, 2); // 1 for /0

            Str[0] = str;
            Str[1] = L'\0';
        }
    }

    String(const char str) noexcept { // one char
        if (str != '\0') {
            Length = Capacity = 1;

            Memory::Allocate<wchar_t>(&Str, 2);

            Str[0] = static_cast<wchar_t>(str);
            Str[1] = L'\0';
        }
    }

    String(const wchar_t *str) noexcept {
        UNumber _length = 0;
        while (str[_length] != L'\0') {
            ++_length;
        };

        if (_length != 0) {
            Length = Capacity = _length;

            Memory::Allocate<wchar_t>(&Str, (_length + 1));

            for (UNumber j = 0; j <= _length; j++) {
                Str[j] = str[j];
            }
        }
    }

    String(const char *str) noexcept {
        UNumber _length = 0;
        while (str[_length] != L'\0') {
            ++_length;
        };

        if (_length != 0) {
            Length = Capacity = _length;

            Memory::Allocate<wchar_t>(&Str, (_length + 1)); // 1 for /0

            for (UNumber j = 0; j <= _length; j++) {
                Str[j] = static_cast<wchar_t>(str[j]);
            }
        }
    }

    String(String &&src) noexcept { // Move
        if (src.Length != 0) {
            Str      = src.Str;
            Capacity = src.Capacity;
            Length   = src.Length;

            src.Capacity = 0;
            src.Length   = 0;
            src.Str      = nullptr;
        }
    }

    String(const String &src) noexcept { // Copy
        if (src.Length != 0) {
            Length = Capacity = src.Length;

            Memory::Allocate<wchar_t>(&Str, (Capacity + 1));

            for (UNumber j = 0; j <= Capacity; j++) {
                Str[j] = src[j];
            }
        }
    }

    ~String() noexcept {
        if (!Shared) {
            Reset();
        }
    }

    inline void Share(wchar_t *_str, UNumber _size) noexcept {
        Str = _str;

        Capacity = _size;
        Length   = _size;
        Shared   = true;
    }

    inline void Reset() noexcept {
        Memory::Deallocate<wchar_t>(&Str);

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

    String &operator=(const wchar_t src) noexcept { // Copy
        if (src != L'\0') {
            if (Capacity == 0) {
                Resize(1);
            }

            Length = 1;
            Str[0] = src;
            Str[1] = L'\0';
        };

        return *this;
    }

    String &operator=(const wchar_t *src) noexcept { // Copy
        UNumber _length = 0;
        while (src[_length] != L'\0') {
            ++_length;
        };

        if (_length != 0) {
            Copy(*this, src, 0, _length);
        }

        return *this;
    }

    String &operator=(String &&src) noexcept { // Move
        if (this != &src) {
            Memory::Deallocate<wchar_t>(&Str);

            Str      = src.Str;
            Capacity = src.Capacity;
            Length   = src.Length;

            src.Capacity = 0;
            src.Length   = 0;
            src.Str      = nullptr;
        }

        return *this;
    }

    String &operator=(const String &src) noexcept { // Copy
        if (this != &src) {
            if (Capacity < src.Length) {
                Memory::Deallocate<wchar_t>(&Str);
                Memory::Allocate<wchar_t>(&Str, (src.Length + 1));

                Length   = src.Length;
                Capacity = src.Length;

                for (UNumber j = 0; j <= src.Length;) { // <= to include \0
                    Str[j] = src[j];
                    ++j;
                }
            } else {
                Length = 0;
                Copy(*this, src.Str, Length, src.Length);
            }
        }

        return *this;
    }

    String &operator+=(const wchar_t src) noexcept { // Appand a string
        if (Length == Capacity) {
            Resize((Length + 1) * 3);
        }

        Str[Length++] = src;
        Str[Length]   = L'\0';

        return *this;
    }

    String &operator+=(const wchar_t *src) noexcept { // Appand a string
        UNumber _length = 0;
        while (src[_length] != L'\0') {
            ++_length;
        };

        if (_length != 0) {
            Copy(*this, src, this->Length, _length);
        }

        return *this;
    }

    String &operator+=(String &&src) noexcept { // Move
        if (src.Length != 0) {
            Copy(*this, src.Str, Length, src.Length);

            Memory::Deallocate<wchar_t>(&src.Str);

            src.Str = nullptr;

            src.Capacity = 0;
            src.Length   = 0;
        }

        return *this;
    }

    inline String &operator+=(const String &src) noexcept { // Appand a string
        if (src.Length != 0) {
            Copy(*this, src.Str, Length, src.Length);
        }

        return *this;
    }

    String operator+(const wchar_t src) const noexcept {
        String ns;
        ns.SetLength(Length + 1);

        if (Length != 0) {
            Copy(ns, Str, 0, Length);
        }

        ns[ns.Length++] = src;
        ns[ns.Length]   = L'\0';

        return ns;
    }

    String operator+(const wchar_t *src) const noexcept {
        String ns;

        UNumber _length = 0;
        while (src[_length] != L'\0') {
            ++_length;
        };

        if (_length != 0) {
            ns.SetLength(Length + _length);

            if (Length != 0) {
                Copy(ns, Str, 0, Length);
            }

            Copy(ns, src, ns.Length, _length);
        }

        return ns;
    }

    // Appand a string by moving another into it
    String operator+(String &&src) const noexcept {
        String ns;
        ns.SetLength(Length + src.Length);

        if (Length != 0) {
            Copy(ns, Str, 0, Length);
        }

        if (src.Length != 0) {
            Copy(ns, src.Str, ns.Length, src.Length);
        }

        Memory::Deallocate<wchar_t>(&src.Str);

        src.Capacity = 0;
        src.Length   = 0;

        return ns;
    }

    String operator+(const String &src) const noexcept { // Appand a string and return a new one
        String ns;
        ns.SetLength(Length + src.Length);

        if (Length != 0) {
            Copy(ns, Str, 0, Length);
        }

        if (src.Length != 0) {
            Copy(ns, src.Str, ns.Length, src.Length);
        }

        return ns;
    }

    bool operator==(const String &src) const noexcept { // Compare
        if (Length != src.Length) {
            return false;
        }

        UNumber i = 0;
        while ((i < Length) && (Str[i] == src[i])) {
            ++i;
        }

        return (i == Length);
    }

    bool operator!=(const String &src) const noexcept { // Compare
        if (Length != src.Length) {
            return true;
        }

        UNumber i = 0;
        while ((i < Length) && (Str[i] == src[i])) {
            ++i;
        }

        return (i != Length);
    }

    inline wchar_t &operator[](const UNumber __index) const noexcept { // Compare
        return Str[__index];
    }

    inline void SetLength(const UNumber size) noexcept {
        Length   = 0;
        Capacity = size;

        Memory::Deallocate<wchar_t>(&Str);
        Memory::Allocate<wchar_t>(&Str, (size + 1));

        Str[0] = L'\0';
    }

    inline void Resize(const UNumber size) noexcept {
        Capacity     = size;
        wchar_t *tmp = Str;

        Memory::Allocate<wchar_t>(&Str, (size + 1));

        if (size < Length) {
            Length = size;
        }

        for (UNumber n = 0; n < Length; n++) {
            Str[n] = tmp[n];
        }

        Str[Length] = L'\0';

        Memory::Deallocate<wchar_t>(&tmp);
    }

    inline static String Part(const String &src, UNumber offset, const UNumber limit) noexcept {
        String bit;

        Memory::Allocate<wchar_t>(&bit.Str, (limit + 1));

        bit.Capacity = limit;

        while (bit.Length < limit) {
            bit[bit.Length++] = src[offset++];
        }

        bit[bit.Length] = L'\0'; // To mark the end of a string.

        return bit;
    }

    inline static UNumber Hash(const String &src, UNumber start, const UNumber end_offset) noexcept {
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

            hash += (((static_cast<UNumber>(src[start++]))) * j++);
        }

        return hash;
    }

    // Update the starting index and the ending one to be at the actual characters
    inline static void SoftTrim(const String &str, UNumber &start, UNumber &end) noexcept {
        --end;

        while ((str[start] == L' ') || (str[start] == L'\n')) {
            ++start;
        }

        while ((end > start) && ((str[end] == L' ') || (str[end] == L'\n'))) {
            --end;
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
    inline static void Revers(String &str) noexcept {
        wchar_t ch;
        UNumber x = (str.Length - 1);

        for (UNumber i = 0; i < x;) {
            ch     = str[x];
            str[x] = str[i];
            str[i] = ch;
            --x;
            ++i;
        }
    }

    static String FromNumber(UNumber number, const UNumber min = 1) noexcept {
        String tnm;

        // Local cache.
        wchar_t p_str[30];
        tnm.Str      = p_str;
        tnm.Capacity = 30; // TODO: Choose the right size
        tnm.Shared   = true;

        while (number > 0) {
            tnm[tnm.Length] = wchar_t((number % 10) + 48);
            ++tnm.Length;

            number /= 10;
        }

        while (tnm.Length < min) {
            tnm[tnm.Length] = L'0';
            ++tnm.Length;
        }

        tnm[tnm.Length] = L'\0';

        Revers(tnm);
        return String(tnm.Str);
    }

    static String FromNumber(double number, const UNumber min = 1, UNumber r_min = 0, UNumber r_max = 0) noexcept {
        String tnm;
        String tnm2;

        // Local cache.
        wchar_t p_str[20];
        tnm.Str      = p_str;
        tnm.Capacity = 20; // TODO: Choose the right size
        tnm.Shared   = true;

        wchar_t p_str2[30];
        tnm2.Str      = p_str2;
        tnm2.Capacity = 30; // TODO: Choose the right size
        tnm2.Shared   = true;

        const bool negative = (number < 0);
        if (negative) {
            number *= -1;
        }

        if (number != 0) {
            UNumber counter = 0;
            UNumber num     = static_cast<UNumber>(number);
            UNumber num2    = num;

            while (num != 0) {
                tnm[tnm.Length] = wchar_t((num % 10) + 48);
                ++tnm.Length;

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
                while (num3 == 0) {
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

                    if (num != 10) { // Means .00000000000
                        for (UNumber w = 0; w < len;) {
                            tnm2[tnm2.Length] = wchar_t((num % 10) + 48);
                            ++tnm2.Length;

                            num /= 10;
                            if (num == 0) {
                                break;
                            }

                            ++w;
                        }

                        if (num != 11) {
                            tnm2 += wchar_t(((num - 1) % 10) + 48); // (num - 1) taking 0.1 back.
                        } else {
                            tnm.Length  = 0;
                            tnm2.Length = 0;

                            ++num2;
                            while (num2 != 0) {
                                tnm += wchar_t((num2 % 10) + 48);
                                num2 /= 10;
                            }
                        }
                    }
                }
            }
        }

        while (tnm.Length < min) {
            tnm.Str[tnm.Length] = L'0';
            ++tnm.Length;
        }

        if (negative) {
            tnm.Str[tnm.Length] = L'-';
            ++tnm.Length;
        }

        while (tnm2.Length < r_min) {
            tnm2.Str[tnm2.Length] = L'0';
            ++tnm2.Length;
        }

        if (tnm2.Length != 0) {
            tnm2.Str[tnm2.Length] = L'.';
            ++tnm2.Length;
            tnm2 += tnm;

            Revers(tnm2);
            return String(tnm2.Str);
        }

        tnm.Str[tnm.Length] = L'\0';
        Revers(tnm);
        return String(tnm.Str);
    }

    static bool ToNumber(const String &str, UNumber &number, const UNumber offset = 0, UNumber limit = 0) noexcept {
        limit += offset;
        if (limit == offset) {
            limit = str.Length - offset;
        }

        wchar_t c;
        UNumber m = 1;

        number = 0;
        for (;;) {
            c = str[--limit];

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

    inline static bool ToNumber(const String &str, double &number, const UNumber offset = 0,
                                UNumber limit = 0) noexcept {
        limit += offset;
        if (limit == offset) {
            limit = str.Length - offset;
        }

        wchar_t c;
        double  m = 1.0;

        number = 0.0;
        for (;;) {
            c = str[--limit];

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
};
} // namespace Qentem

#endif
