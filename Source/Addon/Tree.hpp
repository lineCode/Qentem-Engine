
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
#include "StringStream.hpp"
#include "Engine.hpp"

namespace Qentem {

using Qentem::StringStream;
using Qentem::Engine::Match;

struct Tree;

enum VType { NullT = 0, BooleanT, NumberT, ONumbersT, StringT, OStringsT, ChildT, OChildrenT };

struct Hash {
    UNumber     HashValue = 0;
    UNumber     ExactID   = 0;
    VType       Type      = VType::NullT;
    String      Key       = L"";
    Array<Hash> Table;

    void  Set(const Hash &_hash, const UNumber, const UNumber) noexcept;
    Hash *Get(const UNumber, const UNumber, const UNumber) noexcept;
};

struct Field {
    String Key     = L"";
    Tree * Storage = nullptr;

    Field &operator=(const Field &) noexcept;

    Field &operator=(const double) noexcept;
    Field &operator=(const Array<double> &value) noexcept;
    Field &operator=(const String &value) noexcept;
    Field &operator=(const wchar_t value[]) noexcept;
    Field &operator=(const Array<String> &value) noexcept;
    Field &operator=(const Tree &value) noexcept;
    Field &operator=(const Array<Tree> &value) noexcept;
    Field &operator=(const bool value) noexcept;

    Field operator[](const String &key) noexcept;
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
    Array<Array<Tree>>   OChildren;

    explicit Tree() = default;

    Field operator[](const String &key) {
        Field _field;
        _field.Key     = key;
        _field.Storage = this;
        return _field;
    }

    // void Set(Hash * _hash, const bool, UNumber offset, UNumber limit) noexcept;
    void Set(const String &key, UNumber offset, UNumber limit) noexcept;
    void Set(const String &key, const double value, UNumber offset, UNumber limit) noexcept;
    void Set(const String &key, const Array<double> &value, UNumber offset, UNumber limit) noexcept;
    void Set(const String &key, const String &value, UNumber offset, UNumber limit) noexcept;
    void Set(const String &key, const Array<String> &value, UNumber offset, UNumber limit) noexcept;
    void Set(const String &key, const Tree &value, UNumber offset, UNumber limit) noexcept;
    void Set(const String &key, const Array<Tree> &value, UNumber offset, UNumber limit) noexcept;
    void Set(const String &key, const bool value, UNumber offset, UNumber limit) noexcept;

    void        InsertHash(const Hash &_hash) noexcept;
    void        Drop(const String &key, UNumber offset, UNumber limit) noexcept;
    static void Drop(Hash &hash, Tree &storage) noexcept;

    static const bool GetID(UNumber &id, const String &key, UNumber offset, UNumber limit) noexcept;

    Tree *         GetChild(const String &key, UNumber offset = 0, UNumber limit = 0) const noexcept;
    Tree *         GetInfo(Hash **hash, const String &key, UNumber offset = 0, UNumber limit = 0) const noexcept;
    Array<String> *GetOStrings(const String &key, UNumber offset = 0, UNumber limit = 0) const noexcept;
    bool const     GetString(String &value, const String &key, UNumber offset = 0, UNumber limit = 0) const noexcept;
    bool const     GetNumber(UNumber &value, const String &key, UNumber offset = 0, UNumber limit = 0) const noexcept;
    bool const     GetDouble(double &value, const String &key, UNumber offset = 0, UNumber limit = 0) const noexcept;
    bool const     GetBool(bool &value, const String &key, UNumber offset = 0, UNumber limit = 0) const noexcept;

    String ToJSON() const noexcept;

    static Tree FromJSON(const String &content) noexcept;
    static Tree MakeTree(const String &content, const Array<Match> &items) noexcept;
};

} // namespace Qentem

#endif
