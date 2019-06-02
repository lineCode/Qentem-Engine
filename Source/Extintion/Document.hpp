
/**
 * Qentem Document
 *
 * @brief     Ordered/Unordered array with hasing capability and JSON build-in
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#ifndef QENTEM_DOCUMENT_H
#define QENTEM_DOCUMENT_H

#include "Array.hpp"
#include "String.hpp"
#include "StringStream.hpp"
#include "Engine.hpp"

namespace Qentem {

using Qentem::StringStream;
using Qentem::Engine::Expression;
using Qentem::Engine::Expressions;
using Qentem::Engine::Match;

enum VType { UndefinedT = 0, NullT, BooleanT, NumberT, StringT, DocumentT };

struct Document;

struct Field {
    String    Key     = L"";
    Document *Storage = nullptr;

    Field &operator=(bool value) noexcept;
    Field &operator=(double value) noexcept;
    Field &operator=(const wchar_t *value) noexcept;
    Field &operator=(String &value) noexcept;
    Field &operator=(Document &value) noexcept;
    Field &operator=(Document &&value) noexcept;
    Field &operator=(Array<double> &value) noexcept;
    Field &operator=(Array<String> &value) noexcept;
    Field &operator=(Array<Document> &value) noexcept;
    Field  operator[](const String &key) noexcept;
};

struct Index {
    UNumber      ID   = 0;
    UNumber      Hash = 0;
    Array<Index> Table;
};

struct Entry {
    UNumber ID    = 0;
    UNumber KeyID = 0;
    VType   Type  = VType::UndefinedT;
};

struct Document {
    UNumber HashBase = 19; // Or 97. Choose prime numbers only!
    bool    Ordered  = false;

    Array<Entry> Entries;
    Array<Index> Table;

    Array<double>   Numbers;
    Array<String>   Strings;
    Array<String>   Keys;
    Array<Document> Documents;

    explicit Document() = default;

    Field operator[](const String &key) noexcept {
        Field _field;
        _field.Key     = key;
        _field.Storage = this;
        return _field;
    }

    // static void Drop(Hash &hash, Document &storage) noexcept;
    // void Drop(const String &key, UNumber offset, UNumber limit) noexcept;

    static bool ExtractID(UNumber &id, const String &key, UNumber offset, UNumber limit) noexcept;

    Document *GetSource(Entry **_entry, const String &key, UNumber offset = 0, UNumber limit = 0) noexcept;
    bool      GetString(String &value, const String &key, UNumber offset = 0, UNumber limit = 0) noexcept;
    bool      GetNumber(UNumber &value, const String &key, UNumber offset = 0, UNumber limit = 0) noexcept;
    bool      GetDouble(double &value, const String &key, UNumber offset = 0, UNumber limit = 0) noexcept;
    bool      GetBool(bool &value, const String &key, UNumber offset = 0, UNumber limit = 0) noexcept;
    Document *GetDocument(const String &key, UNumber offset = 0, UNumber limit = 0) noexcept;

    // void InsertHash(const Hash &_hash) noexcept;
    Entry *Exist(const UNumber hash, const UNumber level, const Array<Index> &_table) const noexcept;
    void   InsertIndex(const Index _index, const UNumber level, Array<Index> &_table) noexcept;
    void Insert(const String &key, UNumber offset, UNumber limit, const VType type, void *ptr, const bool move) noexcept;

    String       ToJSON() const noexcept;
    StringStream _ToJSON() const noexcept;

    static Expressions GetJsonExpres() noexcept;

    static void     _makeNumberedDocument(Document &document, const String &content, const Match &item) noexcept;
    static void     _makeDocument(Document &document, const String &content, const Array<Match> &items) noexcept;
    static void     MakeDocument(Document &document, const String &content, const Array<Match> &items) noexcept;
    static Document FromJSON(const String &content) noexcept;

    static Expressions JsonQuot;
    static Expressions JsonDeQuot;

    inline static void SetJsonQuot() noexcept { // TODO: needs clean up or moving
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
};

} // namespace Qentem

#endif
