
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

// NULL terminated wchar_t
class String {
  private:
    UNumber _index = 0;

  public:
    wchar_t *Str    = nullptr;
    UNumber  Length = 0;

    void Move(String &) noexcept;
    void Copy(const wchar_t *, UNumber, UNumber) noexcept;
    void SetSize(UNumber) noexcept;

    explicit String() = default;

    String(const wchar_t *str) noexcept;
    String(wchar_t str) noexcept;

    String(String &&) noexcept;
    String(const String &) noexcept;
    virtual ~String() noexcept;

    static String  Part(const String &, UNumber, const UNumber);
    static UNumber Hash(const String &, UNumber, const UNumber) noexcept;
    static void    SoftTrim(const String &, UNumber &, UNumber &) noexcept;
    static String  Trim(const String &) noexcept;
    static String  Revers(const String &) noexcept;
    static String  FromNumber(double, UNumber min = 1, UNumber max = 0) noexcept;
    static String  FromNumber(UNumber, UNumber min = 1) noexcept;
    static bool    ToNumber(const String &, UNumber &, UNumber offset = 0, UNumber limit = 0) noexcept;
    static bool    ToNumber(const String &, double &, UNumber offset = 0, UNumber limit = 0) noexcept;

    void Clear() noexcept;

    String &operator=(String &&src) noexcept;      // Move
    String &operator=(const String &src) noexcept; // Copy

    String &operator+=(String &&src) noexcept;      // Move
    String &operator+=(const String &src) noexcept; // Copy

    String operator+(String &&src) const noexcept;      // Move
    String operator+(const String &src) const noexcept; // Copy

    bool           operator==(const String &src) const noexcept; // compare; equal
    constexpr bool operator!=(const String &src) const noexcept; // compare: not equal
};
} // namespace Qentem

#endif
