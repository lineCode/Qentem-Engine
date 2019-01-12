
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

using Qentem::String;

namespace Qentem {

enum VType { NullT = 0, StringT, ArrayT, QArrayT };

struct QArray {
    Array<String> Keys   = Array<String>();
    Array<size_t> RealID = Array<size_t>();
    Array<VType>  Types  = Array<VType>();
    // Feature: Might Add Array<float>
    Array<String>        Strings = Array<String>();
    Array<Array<String>> Arrays  = Array<Array<String>>();
    Array<QArray>        VArray  = Array<QArray>();

    explicit QArray() = default;

    void Add(const String &key, const String &_data) noexcept;
    void Add(const String &key, const Array<String> &_data) noexcept;
    void Add(const String &key, const QArray &_data) noexcept;

    bool    GetIndex(const String &key, size_t &index) const noexcept;
    String *GetValue(const String &key) noexcept;

    static bool DecodeKey(String &key, String &id, String &reminder) noexcept;
}; // namespace Qentem
} // namespace Qentem

#endif
