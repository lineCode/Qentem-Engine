
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
using Qentem::Engine::Expression;
using Qentem::Engine::Expressions;
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

    explicit Hash() = default;

    void Move(Hash &src) noexcept {
        if (this != &src) {
            this->HashValue = src.HashValue;
            this->ExactID   = src.ExactID;
            this->Type      = src.Type;
            this->Key.Move(src.Key);
            this->Table.Move(src.Table);
        }
    }

    void Copy(const Hash &src) noexcept {
        if (this != &src) {
            this->HashValue = src.HashValue;
            this->ExactID   = src.ExactID;
            this->Type      = src.Type;
            this->Key       = src.Key;
            this->Table     = src.Table;
        }
    }

    Hash(Hash &&src) noexcept {
        Move(src);
    }

    Hash(const Hash &src) noexcept {
        Copy(src);
    }

    Hash &operator=(Hash &&src) noexcept {
        Move(src);
        return *this;
    }

    Hash &operator=(const Hash &src) noexcept {
        Copy(src);
        return *this;
    }
};

struct Field {
    String Key     = L"";
    Tree * Storage = nullptr;

    Field &operator=(const Field &) noexcept;

    Field &operator=(const double) noexcept;
    Field &operator=(const wchar_t value[]) noexcept;
    Field &operator=(const bool value) noexcept;
    Field &operator=(String &value) noexcept;
    Field &operator=(Array<double> &value) noexcept;
    Field &operator=(Array<String> &value) noexcept;
    Field &operator=(Tree &value) noexcept;
    Field &operator=(Array<Tree> &value) noexcept;

    Field operator[](const String &key) noexcept;
};

struct Tree {
    UNumber HashBase = 19; // Or 97. Choose prime numbers only!

    Array<Hash>          Table;
    Array<UNumber>       Hashes;
    Array<double>        Numbers;
    Array<Array<double>> ONumbers;
    Array<String>        Strings;
    Array<Array<String>> OStrings;
    Array<Tree>          Child;
    Array<Array<Tree>>   OChildren;

    static Expressions JsonQuot;

    static void SetJsonQuot() noexcept {
        if (JsonQuot.Size == 0) {
            static Expression _JsonQuot;
            _JsonQuot.Keyword = L"\"";
            _JsonQuot.Replace = L"\\\"";
            JsonQuot.Add(&_JsonQuot);
        }
    }

    explicit Tree() = default;

    void Move(Tree &src) noexcept {
        if (this != &src) {
            this->HashBase = src.HashBase;

            this->Table.Move(src.Table);
            this->Hashes.Move(src.Hashes);
            this->Numbers.Move(src.Numbers);
            this->ONumbers.Move(src.ONumbers);
            this->Strings.Move(src.Strings);
            this->OStrings.Move(src.OStrings);
            this->Child.Move(src.Child);
            this->OChildren.Move(src.OChildren);
        }
    }

    void Copy(const Tree &src) noexcept {
        if (this != &src) {
            this->HashBase = src.HashBase;

            this->Table     = src.Table;
            this->Hashes    = src.Hashes;
            this->Numbers   = src.Numbers;
            this->ONumbers  = src.ONumbers;
            this->Strings   = src.Strings;
            this->OStrings  = src.OStrings;
            this->Child     = src.Child;
            this->OChildren = src.OChildren;
        }
    }

    Tree(Tree &&src) noexcept {
        Move(src);
    }

    Tree(const Tree &src) noexcept {
        Copy(src);
    }

    Tree &operator=(Tree &&src) noexcept {
        Move(src);
        return *this;
    }

    Tree &operator=(const Tree &src) noexcept {
        Copy(src);
        return *this;
    }

    Field operator[](const String &key) noexcept {
        Field _field;
        _field.Key     = key;
        _field.Storage = this;
        return _field;
    }

    // void Set(Hash * _hash, const bool, UNumber offset, UNumber limit) noexcept;
    void Set(const String &key, UNumber offset, UNumber limit) noexcept;
    void Set(const String &key, const double value, UNumber offset, UNumber limit) noexcept;
    void Set(const String &key, const bool value, UNumber offset, UNumber limit) noexcept;
    void Set(const String &key, String &value, UNumber offset, UNumber limit, bool move) noexcept;
    void Set(const String &key, Array<double> &value, UNumber offset, UNumber limit, bool move) noexcept;
    void Set(const String &key, Tree &value, UNumber offset, UNumber limit, bool move) noexcept;
    void Set(const String &key, Array<String> &value, UNumber offset, UNumber limit, bool move) noexcept;
    void Set(const String &key, Array<Tree> &value, UNumber offset, UNumber limit, bool move) noexcept;

    void        InsertHash(const Hash &_hash) noexcept;
    void        Drop(const String &key, UNumber offset, UNumber limit) noexcept;
    static void Drop(Hash &hash, Tree &storage) noexcept;

    static bool GetID(UNumber &id, const String &key, UNumber offset, UNumber limit) noexcept;

    Tree *         GetChild(const String &key, UNumber offset = 0, UNumber limit = 0) noexcept;
    Tree *         GetInfo(Hash **hash, const String &key, UNumber offset = 0, UNumber limit = 0) noexcept;
    Array<String> *GetOStrings(const String &key, UNumber offset = 0, UNumber limit = 0) noexcept;
    bool           GetString(String &value, const String &key, UNumber offset = 0, UNumber limit = 0) noexcept;
    bool           GetNumber(UNumber &value, const String &key, UNumber offset = 0, UNumber limit = 0) noexcept;
    bool           GetDouble(double &value, const String &key, UNumber offset = 0, UNumber limit = 0) noexcept;
    bool           GetBool(bool &value, const String &key, UNumber offset = 0, UNumber limit = 0) noexcept;

    String       ToJSON() const noexcept;
    StringStream _ToJSON() const noexcept;

    static Tree FromJSON(const String &content) noexcept;
    static void MakeTree(Tree &tree, const String &content, const Array<Match> &items) noexcept;
};

} // namespace Qentem

#endif
