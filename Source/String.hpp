
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

    static void Copy(String &des, const wchar_t *src_p, UNumber start_at, const UNumber ln) noexcept {
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

            if (_tmp != nullptr) {
                delete[] _tmp;
            }
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

    String(const String &src) noexcept { // Copy
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

        if (this->Str != nullptr) {
            delete[] this->Str;
            this->Str = nullptr;
        }
    }

    void Reset() noexcept { // Reset
        if (this->Str != nullptr) {
            delete[] this->Str;
            this->Str = nullptr;
        }

        this->Capacity = 0;
        this->Length   = 0;
    }

    // TODO: implement char & wchar to prevent constructing and destructing often
    String &operator=(String &&src) noexcept { // Move
        if (this != &src) {
            if (this->Str != nullptr) {
                delete[] this->Str;
            }

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
            if (this->Capacity < src.Length) {
                if (this->Str != nullptr) {
                    delete[] this->Str;
                    this->Str = nullptr;
                }

                this->Length = this->Capacity = src.Length;

                this->Str = new wchar_t[this->Length + 1]; // 1 for /0

                for (UNumber j = 0; j <= this->Length; j++) { // <= to include \0
                    this->Str[j] = src.Str[j];
                }
            } else {
                this->Length = 0;
                Copy(*this, src.Str, this->Length, src.Length);
            }
        }

        return *this;
    }

    String &operator+=(String &&src) noexcept { // Move
        if (src.Length != 0) {
            Copy(*this, src.Str, this->Length, src.Length);

            delete[] src.Str;
            src.Str = nullptr;

            src.Capacity = 0;
            src.Length   = 0;
        }

        return *this;
    }

    String &operator+=(const String &src) noexcept { // Appand a string
        if (src.Length != 0) {
            Copy(*this, src.Str, this->Length, src.Length);
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

        if (src.Str != nullptr) {
            delete[] src.Str;
            src.Str = nullptr;
        }

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
        while ((i < this->Length) && (this->Str[i] == src.Str[i])) {
            ++i;
        }

        return (i == this->Length);
    }

    bool operator!=(const String &src) const noexcept { // Compare
        if ((this->Length != src.Length) || (this->Length == 0)) {
            return true;
        }

        UNumber i = 0;
        while ((i < this->Length) && (this->Str[i] == src.Str[i])) {
            ++i;
        }

        return (i != this->Length);
    }

    void SetLength(const UNumber size) noexcept {
        if (this->Str != nullptr) {
            delete[] this->Str;
        }

        this->Str      = new wchar_t[(size + 1)];
        this->Str[0]   = L'\0';
        this->Capacity = size;
        this->Length   = 0;
    }

    void ExpandTo(const UNumber size) noexcept {
        if (size > this->Capacity) {
            this->Capacity = size;
            wchar_t *tmp   = this->Str;

            this->Str = new wchar_t[this->Capacity + 1];

            for (UNumber n = 0; n < this->Length; n++) {
                this->Str[n] = tmp[n];
            }

            this->Str[this->Length] = L'\0';

            if (tmp != nullptr) {
                delete[] tmp;
            }
        }
    }

    static String Part(const String &src, UNumber offset, const UNumber limit) noexcept {
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
            ++i;
            ++start;
        }

        return hash;
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
        wchar_t ch;
        UNumber x = (str.Length - 1);

        for (UNumber i = 0; i < x; i++) {
            ch         = str.Str[x];
            str.Str[x] = str.Str[i];
            str.Str[i] = ch;
            --x;
        }
    }

    static String FromNumber(UNumber number, const UNumber min = 1) noexcept {
        String tnm;

        while (number > 0) {
            if (tnm.Length == tnm.Capacity) {
                tnm.ExpandTo((tnm.Length + 1) * 2);
            }

            tnm.Str[tnm.Length] = wchar_t((number % 10) + 48);
            ++tnm.Length;

            number /= 10;
        }

        if (tnm.Length == tnm.Capacity) {
            if (tnm.Length < min) {
                tnm.ExpandTo(tnm.Length + 1 + (min - tnm.Length));
            } else {
                tnm.ExpandTo(tnm.Length + 1);
            }
        }

        while (tnm.Length < min) {
            tnm.Str[tnm.Length] = L'0';
            ++tnm.Length;
        }

        tnm.Str[tnm.Length] = L'\0';

        Revers(tnm);
        return tnm;
    }

    // TODO: Fix FromNumber(0, 1, 0, 3); showing 0.000 and not 0
    static String FromNumber(double number, const UNumber min = 1, UNumber r_min = 0, UNumber r_max = 0) noexcept {
        String tnm;

        const bool negative = (number < 0);
        if (negative) {
            number *= -1;
        }

        if (number != 0) {
            UNumber num  = static_cast<UNumber>(number);
            UNumber num2 = num;
            number -= num;
            UNumber counter = 0;

            while (num != 0) {
                if (tnm.Length == tnm.Capacity) {
                    tnm.ExpandTo((tnm.Length + 1) * 2);
                }

                tnm.Str[tnm.Length] = wchar_t((num % 10) + 48);
                ++tnm.Length;

                num /= 10;
                counter++;
            }

            while (tnm.Length < min) {
                tnm += L'0';
            }

            if (negative) {
                tnm += L'-';
            }

            if (tnm.Length == 0) {
                tnm = L"";
            }

            if (number != 0) {
                String tnm2;

                number += 1.1; // Forcing the value to round up to it's original number.
                // The 0.1 is to prevent any leading zeros from being on the left
                number *= 1e15; // Moving everyting to the left.
                number -= 1e15; // Taking 1 back.

                num = static_cast<UNumber>(number);
                number -= num;

                // accuracy is impacted when bigger number in the left side
                UNumber len = 14 - ((counter > 1) ? (counter - 1) : 0);

                UNumber fnm = (num % 10);
                if ((number >= 0.5) && (fnm >= 5)) {
                    num += (10 - fnm); // Yep
                }

                while (counter > 1) {
                    num /= 10;
                    --counter;
                }

                // TODO: Cache (num % 10)
                fnm = (num % 10);
                while (((num % 10) == 0) && (len > 0)) {
                    --len;
                    num /= 10;
                }

                if (((num % 10) == 9) && (fnm >= 5) && (number >= 0.5)) {
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
                        tnm2.SetLength(len + 2);
                        for (UNumber w = 0; w < len; w++) {
                            tnm2.Str[tnm2.Length] = wchar_t((num % 10) + 48);
                            ++tnm2.Length;

                            num /= 10;
                            if (num == 0) {
                                break;
                            }
                        }

                        if (num != 11) {                            // 1.000000000
                            tnm2 += wchar_t(((num - 1) % 10) + 48); // (num - 1) taking 0.1 back.
                            tnm2 += L'.';
                            tnm = tnm2 + tnm;
                        } else {
                            tnm2.Reset();

                            tnm.Length = 0;

                            if (r_min != 0) {
                                tnm += L'.';
                            }

                            ++num2;
                            while (num2 != 0) {
                                tnm += wchar_t((num2 % 10) + 48);
                                num2 /= 10;
                            }
                        }
                    } else if (r_min > 0) {
                        tnm += L'.';
                        Revers(tnm);
                    }
                }

                Revers(tnm);

                if (r_min > tnm2.Length) {
                    if (tnm2.Length != 0) {
                        ++r_min -= tnm2.Length;
                    }

                    tnm.ExpandTo(tnm.Length + r_min);
                    while (r_min > 0) {
                        tnm += L'0';
                        --r_min;
                    }
                }
            } else if (r_min != 0) {
                Revers(tnm);
                tnm.ExpandTo(tnm.Length + r_min + 1);

                tnm += L'.';
                while (r_min > 0) {
                    tnm += L'0';
                    --r_min;
                }
            } else {
                Revers(tnm);
            }

            return tnm;
        }

        tnm.SetLength(min + 1 + r_min);

        while (tnm.Length < min) {
            tnm += L'0';
        }

        if (r_min != 0) {
            tnm += L'.';
            while (r_min > 0) {
                tnm += L'0';
                --r_min;
            }
        }

        if (tnm.Length == 0) {
            tnm = L'0';
        }

        return tnm;
    }

    static bool ToNumber(const String &str, UNumber &number, const UNumber offset = 0, UNumber limit = 0) noexcept {
        if (limit == 0) {
            limit = str.Length - offset;
        } else {
            limit = limit + offset;
        }

        wchar_t c;
        UNumber m = 1;

        number = 0;
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

    static bool ToNumber(const String &str, double &number, const UNumber offset = 0, UNumber limit = 0) noexcept {
        if (limit == 0) {
            limit = str.Length - offset;
        } else {
            limit = limit + offset;
        }

        wchar_t c;
        double  m = 1.0;

        number = 0.0;
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
};
} // namespace Qentem

#endif
