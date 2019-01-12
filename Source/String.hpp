
/**
 * Qentem Engine
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
    static void _reset(String *str) noexcept;
    static void _add(const wchar_t *str_p, String *to, size_t start_at, size_t ln) noexcept;
    size_t      _index = 0;

  public:
    wchar_t *Str    = nullptr;
    size_t   Length = 0;

    explicit String() = default;
    String(const wchar_t _str[]) noexcept; // Copy
    String(String &&src) noexcept;         // Move
    String(const String &src) noexcept;    // Copy
    virtual ~String() noexcept;
    void SetSize(size_t _length) noexcept;

    static const float ToNumber(const String &str) noexcept;
    static String      ToString(float number) noexcept;

    String Part(size_t offset, size_t limit) const noexcept;

    String &operator=(const wchar_t _str[]) noexcept; // Copy
    String &operator=(String &&src) noexcept;         // Move
    String &operator=(const String &src) noexcept;    // Copy

    String &operator+=(const wchar_t _str[]) noexcept; // Add +=
    String &operator+=(const wchar_t _str) noexcept;   // Add += One wchar_t
    String &operator+=(const String &src) noexcept;    // Add +=

    const String operator+(const wchar_t _str[]) const noexcept;
    const String operator+(const String &src) const noexcept;

    const bool operator==(const String &src) const noexcept; // ==
    const bool operator!=(const String &src) const noexcept; // ==
};
} // namespace Qentem

#endif
