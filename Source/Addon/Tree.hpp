
/**
 * Qentem Tree
 *
 * @brief     Dynamic Array for Array<String, String>; Map
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#ifndef QENTEM_TREE_H
#define QENTEM_TREE_H

#include "Array.hpp"
#include "String.hpp"

namespace Qentem {

using Qentem::String;

enum VType { NullT = 0, StringT, ArrayT, TreeT };

struct Tree {
    Array<String>  Keys    = Array<String>();
    Array<UNumber> ExactID = Array<UNumber>();
    Array<VType>   Types   = Array<VType>();
    // Feature: Might Add Array<double>
    Array<String>        Strings = Array<String>();
    Array<Array<String>> Arrays  = Array<Array<String>>();
    Array<Tree>          VArray  = Array<Tree>();

    explicit Tree() = default;

    void Add(const String &, const String &) noexcept;
    void Add(const String &, const Array<String> &) noexcept;
    void Add(const String &, const Tree &) noexcept;

    bool    GetIndex(const String &, UNumber &) const noexcept;
    String *GetValue(const String &) noexcept;

    static bool DecodeKey(String &, String &, String &) noexcept;
};
} // namespace Qentem

#endif
