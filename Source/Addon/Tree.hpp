
/**
 * Qentem Tree
 *
 * @brief     Ordered/Unordered array with hasing capability and JSON build-in
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

enum VType { NullT = 0, BooleanT, NumberT, StringT, ChildT };

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

    Field operator[](const String &key) noexcept;

    Field &operator=(bool value) noexcept;
    Field &operator=(double value) noexcept;
    Field &operator=(Array<double> &value) noexcept;
    Field &operator=(const wchar_t *value) noexcept;
    Field &operator=(String &value) noexcept;
    Field &operator=(Array<String> &value) noexcept;
    Field &operator=(Tree &value) noexcept;
    Field &operator=(Array<Tree> &value) noexcept;
};

struct Tree {
    UNumber HashBase = 19;    // Or 97. Choose prime numbers only!
    bool    Ordered  = false; // ordered or not ordered array

    Array<Hash>    Table;
    Array<UNumber> Hashes; // TODO:: Array<Hash*>

    // Types for unordered
    Array<double> Numbers;
    Array<String> Strings;
    Array<Tree>   Child;

    static Expressions JsonQuot;
    static Expressions JsonDeQuot;

    inline static void SetJsonQuot() noexcept { // TODO:: needs clean up or moving
        if (JsonQuot.Size == 0) {
            static Expression _JsonQuot;
            _JsonQuot.Keyword = L"\"";
            _JsonQuot.Replace = L"\\\"";
            JsonQuot.Add(&_JsonQuot);

            static Expression _JsonDeQuot;
            _JsonDeQuot.Keyword = L"\\\"";
            _JsonDeQuot.Replace = L"\"";
            JsonDeQuot.Add(&_JsonDeQuot);
        }
    }

    explicit Tree() = default;

    void Move(Tree &src) noexcept {
        if (this != &src) {
            this->HashBase = src.HashBase;

            this->Table.Move(src.Table);
            this->Hashes.Move(src.Hashes);
            this->Numbers.Move(src.Numbers);
            this->Strings.Move(src.Strings);
            this->Child.Move(src.Child);
        }
    }

    void Copy(const Tree &src) noexcept {
        if (this != &src) {
            this->HashBase = src.HashBase;

            this->Table   = src.Table;
            this->Hashes  = src.Hashes;
            this->Numbers = src.Numbers;
            this->Strings = src.Strings;
            this->Child   = src.Child;
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

    static Expressions GetJsonExpres() noexcept;

    void Insert(const String &key, UNumber offset, UNumber limit, const VType type, void *ptr, const bool move) noexcept;
    void InsertHash(const Hash &_hash) noexcept;
    void Drop(const String &key, UNumber offset, UNumber limit) noexcept;
    static void Drop(Hash &hash, Tree &storage) noexcept;

    static bool GetID(UNumber &id, const String &key, UNumber offset, UNumber limit) noexcept;

    Tree *GetChild(const String &key, UNumber offset = 0, UNumber limit = 0) noexcept;
    Tree *GetInfo(Hash **hash, const String &key, UNumber offset = 0, UNumber limit = 0) noexcept;
    bool  GetString(String &value, const String &key, UNumber offset = 0, UNumber limit = 0) noexcept;
    bool  GetNumber(UNumber &value, const String &key, UNumber offset = 0, UNumber limit = 0) noexcept;
    bool  GetDouble(double &value, const String &key, UNumber offset = 0, UNumber limit = 0) noexcept;
    bool  GetBool(bool &value, const String &key, UNumber offset = 0, UNumber limit = 0) noexcept;

    String       ToJSON() const noexcept;
    StringStream _ToJSON() const noexcept;

    static Tree FromJSON(const String &content) noexcept;
    static void MakeTree(Tree &tree, const String &content, const Array<Match> &items) noexcept;
    static void _makeTree(Tree &tree, const String &content, const Array<Match> &items) noexcept;
};

} // namespace Qentem

#endif
