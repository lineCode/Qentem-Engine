
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

enum VType { NullT = 0, NumberT, StringT, ArrayT, TreeT };

struct Hash {
    UNumber     HashValue = 0;
    UNumber     ExactID   = 0;
    UNumber     level     = 1;
    String      Key;
    VType       Type;
    Array<Hash> Table;

    Hash &get(UNumber _hash_value, UNumber base) {
        if ((this->HashValue != _hash_value) && (this->Table.Size != 0)) {
            return this->Table[((this->HashValue + level) % base)].get(_hash_value, base);
        }
        return *this;
    }

    void set(Hash &_hash, UNumber base = 19) {
        if (HashValue == 0) {
            this->HashValue = _hash.HashValue;
            this->ExactID   = _hash.ExactID;
            this->Type      = _hash.Type;
            this->Key       = _hash.Key;
            return;
        }

        if (this->Table.Size == 0) {
            this->Table.SetSize(base);
            this->Table.Size = base;
        }

        level++;
        this->Table[((this->HashValue + level) % base)].set(_hash, base);
    }
};

class Tree {
  private:
    UNumber _base = 19;

  public:
    Array<UNumber>       Index;
    Array<Hash>          Table;
    Array<double>        Numbers;
    Array<String>        Strings;
    Array<Array<String>> Arrays;
    Array<Tree>          VArray;

    explicit Tree() = default;

    void Add(const String &, double) noexcept;
    void Add(const String &, const String &) noexcept;
    void Add(const String &, const Array<String> &) noexcept;
    void Add(const String &, const Tree &) noexcept;

    void    InsertHash(Hash &) noexcept;
    Hash *  GetInfo(const String &) const noexcept;
    String *GetValue(const String &) const noexcept;

    static bool DecodeKey(String &, String &, String &) noexcept;
};
} // namespace Qentem

#endif
