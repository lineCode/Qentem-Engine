
/**
 * Qentem Tree
 *
 * @brief     Ordered Array With Hasing capability And JSON build-in
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

enum VType { NullT = 0, NumberT, ONumbersT, StringT, OStringsT, Child };
struct Tree;

struct Hash {
    UNumber     HashValue = 0;
    UNumber     ExactID;
    VType       Type;
    String      Key = L"";
    Array<Hash> Table;

    void  Set(Hash *, const UNumber, const UNumber) noexcept;
    Hash *Get(const UNumber, const UNumber, const UNumber) noexcept;
};

struct Field {
    Hash  Info;
    Tree *storage = nullptr;
};

struct Tree {
    UNumber              HashBase = 19; // OR 97. Choose prime numbers only!
    Array<UNumber>       Index;
    Array<Hash>          Table;
    Array<UNumber>       Hashes;
    Array<double>        Numbers;
    Array<Array<double>> ONumbers;
    Array<String>        Strings;
    Array<Array<String>> OStrings;
    Array<Tree>          Child;

    explicit Tree() = default;

    void Set(const String &, const double) noexcept;
    void Set(const String &, const Array<double> &) noexcept;
    void Set(const String &, const String &) noexcept;
    void Set(const String &, const Array<String> &) noexcept;
    void Set(const String &, const Tree &) noexcept;

    void InsertHash(Hash *) noexcept;
    void Drop(const String &key, UNumber offset, UNumber limit) noexcept;

    static bool GetID(UNumber &id, const String &key, UNumber offset, UNumber limit) noexcept;

    const Tree *         GetInfo(Hash **hash, const String &key, UNumber offset = 0, UNumber limit = 0) const noexcept;
    const Tree *         GetChild(const String &key, UNumber offset = 0, UNumber limit = 0) const noexcept;
    const Array<String> *GetOStrings(const String &key, UNumber offset = 0, UNumber limit = 0) const noexcept;

    bool GetString(String &value, const String &key, UNumber offset = 0, UNumber limit = 0) const noexcept;
    bool GetNumber(UNumber &value, const String &key, UNumber offset = 0, UNumber limit = 0) const noexcept;
    bool GetDouble(double &value, const String &key, UNumber offset = 0, UNumber limit = 0) const noexcept;

    String ToJSON() const noexcept;

    static Tree FromJSON(String &content) noexcept;
};
} // namespace Qentem

#endif
