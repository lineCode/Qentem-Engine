
/**
 * Qentem String
 *
 * @brief     String class for Qentem Engine.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#ifndef QENTEM_STRING_H
#define QENTEM_STRING_H

#include "Global.hpp"

namespace Qentem {

// NULL terminated wchar_t
class String {
  private:
    size_t _index = 0;

  public:
    wchar_t *Str    = nullptr;
    size_t   Length = 0;

    explicit String() = default;
    String(const wchar_t[]) noexcept; // Copy
    String(const wchar_t) noexcept;   // Copy
    String(String &&) noexcept;       // Move
    String(const String &) noexcept;  // Copy
    virtual ~String() noexcept;
    void SetSize(size_t) noexcept;

    static String Trim(const String &) noexcept;
    static String Revers(const String &) noexcept;
    static bool   ToNumber(const String &, double &) noexcept;
    static String ToString(double, size_t min = 1, size_t max = 0) noexcept;
    static void   Add(const wchar_t *, String *, size_t, size_t) noexcept;
    static void   Reset(String *) noexcept;

    String Part(size_t, size_t) const;

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
