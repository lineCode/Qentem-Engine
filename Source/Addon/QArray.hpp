
/**
 * Qentem QArray
 *
 * @brief     Dynamic array for Array<String, String>; Map
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#ifndef QENTEM_QARRAY_H
#define QENTEM_QARRAY_H

#include "Array.hpp"
#include "String.hpp"

namespace Qentem {

using Qentem::String;

enum VType { NullT = 0, StringT, ArrayT, QArrayT };

struct QArray {
    Array<String> Keys    = Array<String>();
    Array<size_t> ExactID = Array<size_t>();
    Array<VType>  Types   = Array<VType>();
    // Feature: Might Add Array<double>
    Array<String>        Strings = Array<String>();
    Array<Array<String>> Arrays  = Array<Array<String>>();
    Array<QArray>        VArray  = Array<QArray>();

    explicit QArray() = default;

    void Add(const String &, const String &) noexcept;
    void Add(const String &, const Array<String> &) noexcept;
    void Add(const String &, const QArray &) noexcept;

    bool    GetIndex(const String &, size_t &) const noexcept;
    String *GetValue(const String &) noexcept;

    static bool DecodeKey(String &, String &, String &) noexcept;
};
} // namespace Qentem

#endif
