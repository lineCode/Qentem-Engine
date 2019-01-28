
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

    explicit String() = default;

    String(const wchar_t[]) noexcept; // Copy
    String(const wchar_t) noexcept;   // Copy
    String(String &&) noexcept;       // Move
    String(const String &) noexcept;  // Copy
    virtual ~String() noexcept;

    static String  Part(const String &, UNumber, const UNumber);
    static UNumber Hash(const String &, UNumber, const UNumber);
    static void    SoftTrim(const String &, UNumber &, UNumber &) noexcept;
    static String  Trim(const String &) noexcept;
    static String  Revers(const String &) noexcept;
    static String  FromNumber(double, UNumber min = 1, UNumber max = 0) noexcept;
    static String  FromNumber(UNumber, UNumber min = 1) noexcept;
    static bool    ToNumber(const String &, UNumber &, UNumber offset = 0, UNumber limit = 0) noexcept;
    static bool    ToNumber(const String &, double &, UNumber offset = 0, UNumber limit = 0) noexcept;
    static void    Add(const wchar_t *, String *, UNumber, UNumber) noexcept;
    static void    SetSize(String *, UNumber) noexcept;

    void    Clear() noexcept;
    String &operator=(String &&) noexcept;      // Move
    String &operator=(const String &) noexcept; // Copy

    String &operator+=(String &&) noexcept;      // Move
    String &operator+=(const String &) noexcept; // Add +=

    const String operator+(String &&) const noexcept;
    const String operator+(const String &) const noexcept;

    const bool operator==(const String &) const noexcept; // ==
    const bool operator!=(const String &) const noexcept; // ==
};
} // namespace Qentem

#endif
