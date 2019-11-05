/**
 * Qentem Document
 *
 * @brief     Ordered & Unordered array with hasing capability and JSON build-in
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Engine.hpp"

#ifndef QENTEM_DOCUMENT_H
#define QENTEM_DOCUMENT_H

namespace Qentem {

using Engine::Expression;
using Engine::Expressions;
using Engine::Match;

enum VType { UndefinedT = 0, NullT = 1, BooleanT = 2, NumberT = 3, StringT = 4, DocumentT = 5 };

struct Index {
    UNumber      EntryID = 0;
    UNumber      Hash    = 0;
    Array<Index> Table;
};

struct Entry {
    VType   Type;
    UNumber KeyID;
    UNumber ArrayID;
};

struct {
    wchar_t const *fss1   = L"{";
    wchar_t const *fss2   = L"}";
    wchar_t const *fss3   = L",";
    wchar_t const *fss4   = L"[";
    wchar_t const *fss5   = L"]";
    wchar_t const *fss6   = L"\"";
    wchar_t const *fsc1   = L":";
    wchar_t const *fNull  = L"null";
    wchar_t const *fTrue  = L"true";
    wchar_t const *fFalse = L"false";
} static JFX;

struct Document {
    bool Ordered = false;

    UNumber       HashBase = 17; // Or 97; a prime number only!
    Array<String> Keys;
    Array<Index>  Table;
    Array<Entry>  Entries;

    Array<double>   Numbers;
    Array<String>   Strings;
    Array<Document> Documents;

    UNumber        LastKeyLen = 0;
    wchar_t const *LastKey    = nullptr;

    Document()                    = default;
    virtual ~Document()           = default;
    Document(Document &&doc)      = default;
    Document(Document const &doc) = default;

    Document(Array<double> const &_numbers) noexcept {
        Numbers = _numbers;
        Entries.SetCapacity(_numbers.Size);

        for (UNumber i = 0; i < _numbers.Size; i++) {
            Entries += Entry({VType::NumberT, 0, i});
        }

        Ordered = true;
    }

    Document(Array<String> const &_strings) noexcept {
        Strings = _strings;
        Entries.SetCapacity(_strings.Size);

        for (UNumber i = 0; i < _strings.Size; i++) {
            Entries += Entry({VType::StringT, 0, i});
        }

        Ordered = true;
    }

    Document(Array<Document> const &_documents) noexcept {
        Documents = _documents;
        Entries.SetCapacity(_documents.Size);

        for (UNumber i = 0; i < _documents.Size; i++) {
            Entries += Entry({VType::DocumentT, 0, i});
        }

        Ordered = true;
    }

    Document(Array<double> &&_numbers) noexcept {
        Numbers = static_cast<Array<double> &&>(_numbers);
        Entries.SetCapacity(_numbers.Size);

        for (UNumber i = 0; i < _numbers.Size; i++) {
            Entries += Entry({VType::NumberT, 0, i});
        }

        Ordered = true;
    }

    Document(Array<String> &&_strings) noexcept {
        Strings = static_cast<Array<String> &&>(_strings);
        Entries.SetCapacity(_strings.Size);

        for (UNumber i = 0; i < _strings.Size; i++) {
            Entries += Entry({VType::StringT, 0, i});
        }

        Ordered = true;
    }

    Document(Array<Document> &&_documents) noexcept {
        Documents = static_cast<Array<Document> &&>(_documents);
        Entries.SetCapacity(_documents.Size);

        for (UNumber i = 0; i < _documents.Size; i++) {
            Entries += Entry({VType::DocumentT, 0, i});
        }

        Ordered = true;
    }

    Document(wchar_t const *value) noexcept {
        if (value != nullptr) {
            Array<Match> const items = Engine::Search(value, _getJsonExpres(), 0, String::Count(value));

            if (items.Size != 0) {
                if (items[0].Expr->ID == 1) {
                    _makeList(*this, value, items[0]);
                } else {
                    _makeOrderedList(*this, value, items[0]);
                }
            } else {
                // Just a string.
                Ordered = true;
                Strings += value;
                Entries += Entry({VType::StringT, 0, 0});
            }
        } else {
            Ordered = true;
            Entries += Entry({VType::NullT, 0, 0});
        }
    }

    Document(String const &value) noexcept {
        Array<Match> const items = Engine::Search(value.Str, _getJsonExpres(), 0, value.Length);

        if (items.Size != 0) {
            if (items[0].Expr->ID == 1) {
                _makeList(*this, value.Str, items[0]);
            } else {
                _makeOrderedList(*this, value.Str, items[0]);
            }
        } else {
            Ordered = true;
            Strings += value;
            Entries += Entry({VType::StringT, 0, 0});
        }
    }

    Document(String &&value) noexcept {
        Array<Match> const items = Engine::Search(value.Str, _getJsonExpres(), 0, value.Length);

        if (items.Size != 0) {
            if (items[0].Expr->ID == 1) {
                _makeList(*this, value.Str, items[0]);
            } else {
                _makeOrderedList(*this, value.Str, items[0]);
            }
        } else {
            Ordered = true;
            Strings += static_cast<String &&>(value);
            Entries += Entry({VType::StringT, 0, 0});
        }
    }

    void Reset() noexcept {
        Table.Reset();
        Entries.Reset();
        Keys.Reset();
        Numbers.Reset();
        Strings.Reset();
        Documents.Reset();

        LastKey    = nullptr;
        LastKeyLen = 0;
    }

    static void Drop(Entry &_entry, Document &storage) noexcept {
        _entry.Type = VType::UndefinedT;

        if (!storage.Ordered) {
            storage.Keys[_entry.KeyID].Reset();
        }

        switch (_entry.Type) {
            // case VType::NumberT:
            //     storage.Numbers[_entry.ArrayID] = 0; // Waste of time.
            //     break;
            case VType::StringT:
                storage.Strings[_entry.ArrayID].Reset();
                break;
            case VType::DocumentT:
                storage.Documents[_entry.ArrayID].Reset();
                break;
            default:
                break;
        }
    }

    void Drop(wchar_t const *key) noexcept {
        Entry *   _entry;
        Document *storage = GetSource(&_entry, key, 0, String::Count(key));

        if (_entry != nullptr) {
            Drop(*_entry, *storage);
        }
    }

    void InsertIndex(Index const &_index, UNumber const level, Array<Index> &_table) noexcept {
        UNumber const id = ((_index.Hash + level) % HashBase);

        if (_table.Capacity <= id) {
            _table.Resize(id + 1);
            _table.Size = _table.Capacity;
        }

        if (_table[id].Hash == 0) {
            _table[id] = _index;
            return;
        }

        InsertIndex(_index, (level + id + 2), _table[id].Table);
    }

    Entry *Exist(UNumber hash, UNumber const level, Array<Index> const &_table) const noexcept {
        UNumber const id = ((hash + level) % HashBase);

        if ((_table.Size > id) && (_table[id].Hash != 0)) {
            if (_table[id].Hash == hash) {
                return &(Entries[_table[id].EntryID]);
            }

            return Exist(hash, (level + id + 2), _table[id].Table);
        }

        return nullptr;
    }

    void Rehash(UNumber const newBase, bool const children = false) noexcept {
        Table.Reset();
        HashBase = newBase;

        Index   _index;
        String *key;

        for (UNumber i = 0; i < Keys.Size; i++) {
            key            = &(Keys[Entries[i].KeyID]);
            _index.Hash    = String::Hash(key->Str, 0, key->Length);
            _index.EntryID = i;

            InsertIndex(_index, 0, Table);

            if (children && (Entries[i].Type == VType::DocumentT)) {
                Documents[Entries[i].ArrayID].Rehash(HashBase, true);
            }
        }
    }

    void Insert(UNumber entryID, VType const type, void *ptr, bool const move) noexcept {
        if (entryID < Entries.Size) {
            Entry & _ent = Entries[entryID];
            UNumber id   = 0;

            if (_ent.Type != type) {
                // New item.
                switch (type) {
                    case VType::DocumentT: {
                        id = Documents.Size;
                        if (move) {
                            Documents += static_cast<Document &&>(*(static_cast<Document *>(ptr)));
                        } else {
                            Documents += *(static_cast<Document *>(ptr));
                        }
                        break;
                    }
                    case VType::StringT: {
                        id = Strings.Size;
                        if (move) {
                            Strings += static_cast<String &&>(*(static_cast<String *>(ptr)));
                        } else {
                            Strings += *(static_cast<String *>(ptr));
                        }
                        break;
                    }
                    case VType::BooleanT:
                    case VType::NumberT: {
                        id = Numbers.Size;
                        Numbers += *(static_cast<double *>(ptr));
                        break;
                    }
                    default:
                        break;
                }

                // Clearing any existing value.
                switch (_ent.Type) {
                    case VType::StringT: {
                        Strings[_ent.ArrayID].Reset();
                    } break;
                    case VType::DocumentT: {
                        Documents[_ent.ArrayID].Reset();
                    } break;
                    default:
                        break;
                }

                _ent.ArrayID = id;
                _ent.Type    = type;
            } else {
                // Updating existing item.
                switch (type) {
                    case VType::DocumentT: {
                        if (move) {
                            Documents[_ent.ArrayID] = static_cast<Document &&>(*(static_cast<Document *>(ptr)));
                        } else {
                            Documents[_ent.ArrayID] = *(static_cast<Document *>(ptr));
                        }
                        break;
                    }
                    case VType::StringT: {
                        if (move) {
                            Strings[_ent.ArrayID] = static_cast<String &&>(*(static_cast<String *>(ptr)));
                        } else {
                            Strings[_ent.ArrayID] = *(static_cast<String *>(ptr));
                        }
                        break;
                    }
                    case VType::NumberT:
                    case VType::BooleanT: {
                        Numbers[_ent.ArrayID] = *(static_cast<double *>(ptr));
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }

    void Insert(wchar_t const *key, UNumber const offset, UNumber const limit, VType const type, void *ptr, bool const move,
                bool const check = true) noexcept {
        UNumber       id    = 0;
        UNumber const _hash = String::Hash(key, offset, (offset + limit));
        Entry *       _ent  = (!check ? nullptr : Exist(_hash, 0, Table));

        if ((_ent == nullptr) || (_ent->Type != type)) {
            // New item.
            switch (type) {
                case VType::DocumentT: {
                    id = Documents.Size;
                    if (move) {
                        Documents += static_cast<Document &&>(*(static_cast<Document *>(ptr)));
                    } else {
                        Documents += *(static_cast<Document *>(ptr));
                    }
                    break;
                }
                case VType::StringT: {
                    id = Strings.Size;
                    if (move) {
                        Strings += static_cast<String &&>(*(static_cast<String *>(ptr)));
                    } else {
                        Strings += *(static_cast<String *>(ptr));
                    }
                    break;
                }
                case VType::BooleanT:
                case VType::NumberT: {
                    id = Numbers.Size;
                    Numbers += *(static_cast<double *>(ptr));
                    break;
                }
                default:
                    break;
            }
        } else {
            // Updating existing item.
            switch (type) {
                case VType::DocumentT: {
                    if (move) {
                        Documents[_ent->ArrayID] = static_cast<Document &&>(*(static_cast<Document *>(ptr)));
                    } else {
                        Documents[_ent->ArrayID] = *(static_cast<Document *>(ptr));
                    }
                    break;
                }
                case VType::StringT: {
                    if (move) {
                        Strings[_ent->ArrayID] = static_cast<String &&>(*(static_cast<String *>(ptr)));
                    } else {
                        Strings[_ent->ArrayID] = *(static_cast<String *>(ptr));
                    }
                    break;
                }
                case VType::NumberT:
                case VType::BooleanT: {
                    Numbers[_ent->ArrayID] = *(static_cast<double *>(ptr));
                    break;
                }
                default:
                    break;
            }
        }

        if (_ent != nullptr) {
            // If exists ...
            if (_ent->Type != type) {
                // Clearing any existing value.
                switch (_ent->Type) {
                    case VType::StringT: {
                        Strings[_ent->ArrayID].Reset();
                    } break;
                    case VType::DocumentT: {
                        Documents[_ent->ArrayID].Reset();
                    } break;
                    default:
                        break;
                }

                _ent->ArrayID = id;
                _ent->Type    = type;
            }

            return;
        }

        Index _index;
        _index.Hash    = _hash;
        _index.EntryID = Entries.Size;

        Entries += Entry({type, Keys.Size, id});
        Keys += String::Part(key, offset, limit);

        InsertIndex(_index, 0, Table);
    }

    static void _makeOrderedList(Document &document, wchar_t const *content, Match const &item) noexcept {
        Array<Match> const &_items = item.NestMatch;

        document.Ordered = true;

        Match * subItem;
        UNumber limit;
        double  number;

        bool          pass    = false;
        UNumber       _itemID = 0;
        UNumber       offset  = (item.Offset + 1); // the starting char [
        UNumber       start   = offset;
        UNumber const end     = (item.Length + item.Offset);

        for (; offset < end; offset++) {
            while (content[offset] == L' ') {
                ++offset;
            }

            switch (content[offset]) {
                case L',':
                case L']': {
                    if (!pass) {
                        // A Number, true/false or null
                        limit = (offset - start);
                        String::SoftTrim(content, start, limit);

                        switch (content[start]) {
                            case L't': {
                                // True
                                document.Entries += Entry({VType::BooleanT, 0, document.Numbers.Size});
                                document.Numbers += 1;
                                break;
                            }
                            case L'f': {
                                // False
                                document.Entries += Entry({VType::BooleanT, 0, document.Numbers.Size});
                                document.Numbers += 0;
                                break;
                            }
                            case L'n': {
                                // Null
                                document.Entries += Entry({VType::NullT, 0, 0});
                                break;
                            }
                            default: {
                                // A number
                                if (String::ToNumber(number, content, start, limit)) {
                                    document.Entries += Entry({VType::NumberT, 0, document.Numbers.Size});
                                    document.Numbers += number;
                                }
                                break;
                            }
                        }
                    }

                    start = (offset + 1);
                    pass  = false;
                    break;
                }
                case L'"': {
                    subItem = &(_items.operator[](_itemID++));

                    document.Entries += Entry({VType::StringT, 0, document.Strings.Size});

                    if (subItem->NestMatch.Size == 0) {
                        document.Strings += String::Part(content, (subItem->Offset + 1), (subItem->Length - 2));
                    } else {
                        document.Strings += Engine::Parse(content, subItem->NestMatch, (subItem->Offset + 1), (subItem->Length - 2));
                    }

                    offset = (subItem->Offset + subItem->Length) - 1;
                    pass   = true;
                    break;
                }
                case L'{': {
                    subItem = &(_items.operator[](_itemID++));

                    document.Entries += Entry({VType::DocumentT, 0, document.Documents.Size});

                    Document doc;
                    _makeList(doc, content, *subItem);
                    document.Documents += static_cast<Document &&>(doc);

                    offset = (subItem->Offset + subItem->Length) - 1;
                    pass   = true;
                    break;
                }
                case L'[': {
                    subItem = &(_items.operator[](_itemID++));

                    document.Entries += Entry({VType::DocumentT, 0, document.Documents.Size});

                    Document doc;
                    _makeOrderedList(doc, content, *subItem);
                    document.Documents += static_cast<Document &&>(doc);

                    offset = (subItem->Offset + subItem->Length) - 1;
                    pass   = true;
                    break;
                }
            }
        }
    }

    static void _makeList(Document &document, wchar_t const *content, Match const &item) noexcept {
        Array<Match> const &_items = item.NestMatch;

        bool done = false;

        Match * subItem;
        UNumber limit;
        double  number;

        String tmpString;

        UNumber start = 0;
        UNumber j;
        Match * key;

        UNumber const _length = (item.Offset + item.Length);

        for (UNumber i = 0; i < _items.Size; i++) {
            key = &(_items.operator[](i));
            j   = (key->Offset + key->Length);

            for (; j < _length; j++) {
                while (content[j] == L' ') {
                    ++j;
                }

                switch (content[j]) {
                    case L',':
                    case L'}': {
                        // A true, false, null or number value.
                        limit = j - start;
                        String::SoftTrim(content, start, limit);

                        switch (content[start]) {
                            case L't': {
                                // True
                                number = 1;
                                document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::BooleanT, &number, false, false);
                                break;
                            }
                            case L'f': {
                                // False
                                number = 0;
                                document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::BooleanT, &number, false, false);
                                break;
                            }
                            case L'n': {
                                // Null
                                document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::NullT, nullptr, false, false);
                                break;
                            }
                            default: {
                                if (String::ToNumber(number, content, start, limit)) {
                                    // Number
                                    document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::NumberT, &number, false, false);
                                }
                                break;
                            }
                        }

                        done = true;
                        break;
                    }
                    case L'"': {
                        ++i;

                        subItem = &(_items.operator[](i));
                        if (subItem->NestMatch.Size == 0) {
                            tmpString = String::Part(content, (subItem->Offset + 1), (subItem->Length - 2));
                        } else {
                            tmpString = Engine::Parse(content, subItem->NestMatch, (subItem->Offset + 1), (subItem->Length - 2));
                        }

                        document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::StringT, &tmpString, true, false);

                        done = true;
                        break;
                    }
                    case L':': {
                        start = j + 1;
                        continue;
                    }
                    case L'{': {
                        ++i;

                        Document uno_document;
                        _makeList(uno_document, content, _items.operator[](i));

                        document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::DocumentT, &uno_document, true, false);

                        done = true;
                        break;
                    }
                    case L'[': {
                        ++i;

                        Document o_document;
                        _makeOrderedList(o_document, content, _items.operator[](i));

                        document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::DocumentT, &o_document, true, false);

                        done = true;
                        break;
                    }
                }

                if (done) {
                    done = false;
                    break;
                }
            }
        }
    }

    static Document FromJSON(wchar_t const *content, UNumber const offset, UNumber const limit, bool const comments = false) noexcept {
        Document document;
        if (limit == 0) {
            return document;
        }

        Array<Match> items;

        String n_content;

        static Expressions const &json_expres = _getJsonExpres();

        // C style comments
        if (!comments) {
            items = Engine::Search(content, json_expres, offset, limit);
        } else {
            static Expressions __comments;
            if (__comments.Size == 0) {
                static Expression comment1;
                static Expression comment_next1;
                comment1.SetKeyword(L"/*");
                comment_next1.SetKeyword(L"*/");
                comment_next1.SetReplace(L"\n");
                comment1.Connected = &comment_next1;

                static Expression comment2;
                static Expression comment_next2;
                comment2.SetKeyword(L"//");
                comment_next2.SetKeyword(L"\n");
                comment_next2.SetReplace(L"\n");
                comment2.Connected = &comment_next2;

                __comments = Expressions().Add(&comment1).Add(&comment2);
            }

            n_content = Engine::Parse(content, Engine::Search(content, __comments, offset, limit), offset, limit);
            items     = Engine::Search(n_content.Str, json_expres, 0, n_content.Length);
        }

        if (items.Size != 0) {
            if (items[0].Expr->ID == 1) {
                _makeList(document, (n_content.Length == 0) ? content : n_content.Str, items[0]);
            } else {
                _makeOrderedList(document, (n_content.Length == 0) ? content : n_content.Str, items[0]);
            }
        }

        return document;
    }

    static Document FromJSON(String const &content, bool const comments = false) noexcept {
        return FromJSON(content.Str, 0, content.Length, comments);
    }

    static bool ExtractID(UNumber &id, wchar_t const *key, UNumber const offset, UNumber limit) noexcept {
        UNumber end = (offset + (limit - 1));

        while ((end > offset) && (key[--end] != L'[')) {
        }
        ++end;
        --limit;

        return String::ToNumber(id, key, end, (limit - (end - offset)));
    }

    // TODO: Rewrite
    // Key form can be: name, name[id1], name[id1][sub-id2], name[id1][sub-id2][sub-sub-idx]...
    Document *GetSource(Entry **_entry, wchar_t const *key, UNumber offset, UNumber limit) noexcept {
        UNumber end_offset = (offset + limit);
        UNumber _tmpOffset;
        UNumber _id;

        if (key[offset] == L'[') {
            ++offset;
            end_offset = offset;

            _tmpOffset = (offset + limit);
            while (key[end_offset] != L']') {
                ++end_offset;

                if (end_offset > _tmpOffset) {
                    return nullptr;
                }
            }

            _id = String::Hash(key, offset, end_offset);
            ++end_offset;
            --offset;
        } else if (key[(end_offset - 1)] == L']') {
            end_offset = offset;

            _tmpOffset = (offset + limit);
            while (key[end_offset] != L'[') {
                ++end_offset;

                if (end_offset > _tmpOffset) {
                    return nullptr;
                }
            }
            _id = String::Hash(key, offset, end_offset);
        } else {
            _id = String::Hash(key, offset, end_offset);
        }

        *_entry = Exist(_id, 0, Table);
        if (*_entry != nullptr) {
            if ((*_entry)->Type == VType::DocumentT) {
                limit -= (end_offset - offset);
                Document *doc = &(Documents[(*_entry)->ArrayID]);

                if (limit != 0) {
                    if (doc->Ordered) {
                        if (ExtractID(_id, key, end_offset, limit) && (doc->Entries.Size > _id)) {
                            *_entry = &(doc->Entries[_id]);
                        }
                    } else {
                        return doc->GetSource(_entry, key, end_offset, limit);
                    }
                }

                return doc;
            }

            return this;
        }

        return nullptr;
    }

    bool GetString(String &value, wchar_t const *key, UNumber const offset, UNumber const limit) noexcept {
        value.Reset();

        Entry *         _entry;
        Document const *storage = GetSource(&_entry, key, offset, limit);

        if (storage != nullptr) {
            if (_entry->Type == VType::StringT) {
                value = storage->Strings[_entry->ArrayID];
                return true;
            }

            if (_entry->Type == VType::BooleanT) {
                value = ((storage->Numbers[_entry->ArrayID] == 1.0) ? L"true" : L"false");
                return true;
            }

            if (_entry->Type == VType::NullT) {
                value = L"null";
                return true;
            }

            if (_entry->Type == VType::NumberT) {
                value = String::FromNumber(storage->Numbers[_entry->ArrayID], 1, 0, 3);
                return true;
            }
        }

        return false;
    }

    bool GetString(String &value, wchar_t const *key) noexcept {
        return GetString(value, key, 0, String::Count(key));
    }

    bool GetNumber(UNumber &value, wchar_t const *key, UNumber const offset, UNumber const limit) noexcept {
        value = 0;

        Entry *         _entry;
        Document const *storage = GetSource(&_entry, key, offset, limit);

        if (storage != nullptr) {
            if (_entry->Type == VType::StringT) {
                String const &st = storage->Strings[_entry->ArrayID];
                return String::ToNumber(value, st.Str, 0, st.Length);
            }

            if (_entry->Type == VType::BooleanT) {
                value = ((storage->Numbers[_entry->ArrayID] == 1.0) ? 1 : 0);
                return true;
            }

            if (_entry->Type == VType::NullT) {
                value = 0;
                return true;
            }

            if (_entry->Type == VType::NumberT) {
                value = static_cast<UNumber>(storage->Numbers[_entry->ArrayID]);
                return true;
            }
        }

        return false;
    }

    bool GetDouble(double &value, wchar_t const *key, UNumber const offset, UNumber const limit) noexcept {
        value = 0.0;

        Entry *         _entry;
        Document const *storage = GetSource(&_entry, key, offset, limit);

        if (storage != nullptr) {
            if (_entry->Type == VType::StringT) {
                String const &st = storage->Strings[_entry->ArrayID];
                return String::ToNumber(value, st.Str, 0, st.Length);
            }

            if (_entry->Type == VType::BooleanT) {
                value = ((storage->Numbers[_entry->ArrayID] == 1.0) ? 1.0 : 0.0);
                return true;
            }

            if (_entry->Type == VType::NullT) {
                value = 0;
                return true;
            }

            if (_entry->Type == VType::NumberT) {
                value = storage->Numbers[_entry->ArrayID];
                return true;
            }
        }

        return false;
    }

    bool GetBool(bool &value, wchar_t const *key, UNumber const offset, UNumber const limit) noexcept {
        Entry *         _entry;
        Document const *storage = GetSource(&_entry, key, offset, limit);

        if (storage != nullptr) {
            if ((_entry->Type == VType::BooleanT) || (_entry->Type == VType::NumberT)) {
                value = (storage->Numbers[_entry->ArrayID] == 1);
                return true;
            }

            if (_entry->Type == VType::StringT) {
                value = (storage->Strings[_entry->ArrayID] == L"true");
                return true;
            }

            if (_entry->Type == VType::NullT) {
                value = false;
                return true;
            }
        }

        return false;
    }

    Document *GetDocument(wchar_t const *key, UNumber const offset, UNumber const limit) noexcept {
        Entry *   _entry;
        Document *storage = GetSource(&_entry, key, offset, limit);

        if ((storage != nullptr) && (_entry->Type == VType::DocumentT)) {
            return storage;
        }

        return nullptr;
    }

    String ToJSON() const noexcept {
        static Expressions const &to_json_expres = _getToJsonExpres();

        StringStream ss;
        Array<Match> tmpMatchs;
        String *     str_ptr;
        Entry *      _entry;

        if (Ordered) {
            ss += JFX.fss4;

            for (UNumber i = 0; i < Entries.Size; i++) {
                _entry = &(Entries[i]);

                if (ss.Length != 1) {
                    ss += JFX.fss3;
                }

                switch (_entry->Type) {
                    case VType::NullT: {
                        ss += JFX.fNull;
                        break;
                    }
                    case VType::BooleanT: {
                        ss += (Numbers[_entry->ArrayID] != 0 ? JFX.fTrue : JFX.fFalse);
                        break;
                    }
                    case VType::NumberT: {
                        ss += String::FromNumber(Numbers[_entry->ArrayID]);
                        break;
                    }
                    case VType::StringT: {
                        ss += JFX.fss6;

                        str_ptr   = &(Strings[_entry->ArrayID]);
                        tmpMatchs = Engine::Search(str_ptr->Str, to_json_expres, 0, str_ptr->Length);
                        if (tmpMatchs.Size == 0) {
                            ss += *str_ptr;
                        } else {
                            ss += Engine::Parse(str_ptr->Str, tmpMatchs, 0, str_ptr->Length);
                        }

                        ss += JFX.fss6;
                        break;
                    }
                    case VType::DocumentT: {
                        ss += Documents[_entry->ArrayID].ToJSON();
                        break;
                    }
                    default:
                        break;
                }
            }

            ss += JFX.fss5;
        } else {
            ss += JFX.fss1;

            for (UNumber i = 0; i < Entries.Size; i++) {
                _entry = &(Entries[i]);

                if (ss.Length != 1) {
                    ss += JFX.fss3;
                }

                switch (_entry->Type) {
                    case VType::NullT: {
                        ss += JFX.fss6;
                        ss += Keys[_entry->KeyID];
                        ss += JFX.fss6;
                        ss += JFX.fsc1;
                        ss += JFX.fNull;
                        break;
                    }
                    case VType::BooleanT: {
                        ss += JFX.fss6;
                        ss += Keys[_entry->KeyID];
                        ss += JFX.fss6;
                        ss += JFX.fsc1;
                        ss += (Numbers[_entry->ArrayID] != 0 ? JFX.fTrue : JFX.fFalse);
                        break;
                    }
                    case VType::NumberT: {
                        ss += JFX.fss6;
                        ss += Keys[_entry->KeyID];
                        ss += JFX.fss6;
                        ss += JFX.fsc1;
                        ss += String::FromNumber(Numbers[_entry->ArrayID]);
                        break;
                    }
                    case VType::StringT: {
                        ss += JFX.fss6;
                        ss += Keys[_entry->KeyID];
                        ss += JFX.fss6;
                        ss += JFX.fsc1;
                        ss += JFX.fss6;

                        str_ptr   = &(Strings[_entry->ArrayID]);
                        tmpMatchs = Engine::Search(str_ptr->Str, to_json_expres, 0, str_ptr->Length);
                        if (tmpMatchs.Size == 0) {
                            ss += *str_ptr;
                        } else {
                            ss += Engine::Parse(str_ptr->Str, tmpMatchs, 0, str_ptr->Length);
                        }

                        ss += JFX.fss6;
                        break;
                    }
                    case VType::DocumentT: {
                        ss += JFX.fss6;
                        ss += Keys[_entry->KeyID];
                        ss += JFX.fss6;
                        ss += JFX.fsc1;
                        ss += Documents[_entry->ArrayID].ToJSON();
                        break;
                    }
                    default:
                        break;
                }
            }

            ss += JFX.fss2;
        }

        return ss.Eject();
    }

    static Expressions const &_getToJsonExpres() noexcept {
        static Expression  _JsonEsc;
        static Expression  _JsonQuot;
        static Expressions tags;

        if (tags.Size == 0) {
            _JsonEsc.SetKeyword(L"\\");
            _JsonEsc.MatchCB =
                ([](wchar_t const *content, UNumber &offset, UNumber const endOffset, Match &item, Array<Match> &items) noexcept -> void {
                    if ((content[offset] == L'\\') || (content[offset] == L' ') || (offset == endOffset)) {
                        // If there is a space after \ or \ or it's at the end, then it's a match.
                        items += static_cast<Match &&>(item);
                    }
                });
            _JsonEsc.SetReplace(L"\\\\");

            _JsonQuot.SetKeyword(L"\"");
            _JsonQuot.SetReplace(L"\\\"");

            tags += &_JsonEsc;
            tags += &_JsonQuot;
        }

        return tags;
    }

    static Expressions const &_getJsonExpres() noexcept {
        static Expression esc_quotation;
        static Expression esc_esc;

        static Expression quotation_start;
        static Expression quotation_end;

        static Expression opened_square_bracket;
        static Expression closed_square_bracket;

        static Expression opened_curly_bracket;
        static Expression closed_curly_bracket;

        static Expressions tags;

        if (tags.Size == 0) {
            esc_esc.SetKeyword(L"\\\\");
            esc_esc.SetReplace(L"\\");

            esc_quotation.SetKeyword(L"\\\"");
            esc_quotation.SetReplace(L"\"");

            quotation_start.SetKeyword(L"\"");
            quotation_end.SetKeyword(L"\"");
            quotation_end.ID          = 3;
            quotation_start.Connected = &quotation_end;
            quotation_end.NestExprs += &esc_esc;
            quotation_end.NestExprs += &esc_quotation;

            opened_curly_bracket.SetKeyword(L"{");
            closed_curly_bracket.SetKeyword(L"}");
            closed_curly_bracket.ID        = 1;
            opened_curly_bracket.Connected = &closed_curly_bracket;

            closed_curly_bracket.NestExprs = Expressions().Add(&opened_curly_bracket).Add(&quotation_start).Add(&opened_square_bracket);

            opened_square_bracket.SetKeyword(L"[");
            closed_square_bracket.SetKeyword(L"]");
            closed_square_bracket.ID        = 2;
            opened_square_bracket.Connected = &closed_square_bracket;

            closed_square_bracket.NestExprs = Expressions().Add(&opened_square_bracket).Add(&quotation_start).Add(&opened_curly_bracket);

            tags += &opened_curly_bracket;
            tags += &opened_square_bracket;
        }

        return tags;
    }

    void operator+=(Document const &doc) noexcept {
        if (Ordered != doc.Ordered) {
            // Not the same type!
            return;
        }

        Entry *_entry;

        if (Ordered) {
            for (UNumber i = 0; i < doc.Entries.Size; i++) {
                _entry = &(doc.Entries[i]);

                switch (_entry->Type) {
                    case VType::StringT: {
                        Entries += Entry({VType::StringT, 0, Strings.Size});
                        Strings += doc.Strings[_entry->ArrayID];
                        break;
                    }
                    case VType::DocumentT: {
                        Entries += Entry({VType::DocumentT, 0, Documents.Size});
                        Documents += doc.Documents[_entry->ArrayID];
                        break;
                    }
                    case VType::NumberT: {
                        Entries += Entry({VType::NumberT, 0, Numbers.Size});
                        Numbers += doc.Numbers[_entry->ArrayID];
                        break;
                    }
                    case VType::BooleanT: {
                        Entries += Entry({VType::BooleanT, 0, Numbers.Size});
                        Numbers += doc.Numbers[_entry->ArrayID];
                        break;
                    }
                    case VType::NullT: {
                        Entries += Entry({VType::NullT, 0, 0});
                        break;
                    }
                    default:
                        break;
                }
            }
        } else {
            String *_key;

            for (UNumber i = 0; i < doc.Entries.Size; i++) {
                _entry = &(doc.Entries[i]);
                _key   = &(doc.Keys[doc.Entries[i].KeyID]);

                switch (_entry->Type) {
                    case VType::StringT: {
                        Insert(_key->Str, 0, _key->Length, _entry->Type, &doc.Strings[_entry->ArrayID], false, true);
                        break;
                    }
                    case VType::DocumentT: {
                        Insert(_key->Str, 0, _key->Length, _entry->Type, &doc.Documents[_entry->ArrayID], false, true);
                        break;
                    }
                    case VType::NumberT: {
                        Insert(_key->Str, 0, _key->Length, _entry->Type, &doc.Numbers[_entry->ArrayID], false, true);
                        break;
                    }
                    case VType::BooleanT: {
                        Insert(_key->Str, 0, _key->Length, _entry->Type, &doc.Numbers[_entry->ArrayID], false, true);
                        break;
                    }
                    case VType::NullT: {
                        Insert(_key->Str, 0, _key->Length, _entry->Type, nullptr, false, true);
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }

    void operator+=(Document &&doc) noexcept {
        if (Ordered != doc.Ordered) {
            // Not the same type!
            return;
        }

        Entry *_entry;

        if (Ordered) {
            for (UNumber i = 0; i < doc.Entries.Size; i++) {
                _entry = &(doc.Entries[i]);

                switch (_entry->Type) {
                    case VType::StringT: {
                        Entries += Entry({VType::StringT, 0, Strings.Size});
                        Strings += static_cast<String &&>(doc.Strings[_entry->ArrayID]);
                        break;
                    }
                    case VType::DocumentT: {
                        Entries += Entry({VType::DocumentT, 0, Documents.Size});
                        Documents += static_cast<Document &&>(doc.Documents[_entry->ArrayID]);
                        break;
                    }
                    case VType::NumberT: {
                        Entries += Entry({VType::NumberT, 0, Numbers.Size});
                        Numbers += doc.Numbers[_entry->ArrayID];
                        break;
                    }
                    case VType::BooleanT: {
                        Entries += Entry({VType::BooleanT, 0, Numbers.Size});
                        Numbers += doc.Numbers[_entry->ArrayID];
                        break;
                    }
                    case VType::NullT: {
                        Entries += Entry({VType::NullT, 0, 0});
                        break;
                    }
                    default:
                        break;
                }
            }
        } else {
            String *_key;

            for (UNumber i = 0; i < doc.Entries.Size; i++) {
                _entry = &(doc.Entries[i]);
                _key   = &(doc.Keys[doc.Entries[i].KeyID]);

                switch (_entry->Type) {
                    case VType::StringT: {
                        Insert(_key->Str, 0, _key->Length, _entry->Type, &doc.Strings[_entry->ArrayID], true, true);
                        break;
                    }
                    case VType::DocumentT: {
                        Insert(_key->Str, 0, _key->Length, _entry->Type, &doc.Documents[_entry->ArrayID], true, true);
                        break;
                    }
                    case VType::NumberT: {
                        Insert(_key->Str, 0, _key->Length, _entry->Type, &doc.Numbers[_entry->ArrayID], false, true);
                        break;
                    }
                    case VType::BooleanT: {
                        Insert(_key->Str, 0, _key->Length, _entry->Type, &doc.Numbers[_entry->ArrayID], false, true);
                        break;
                    }
                    case VType::NullT: {
                        Insert(_key->Str, 0, _key->Length, _entry->Type, nullptr, false, true);
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }

    Document &operator=(Document &&doc) noexcept {
        if (LastKeyLen == 0) {
            Ordered   = doc.Ordered;
            HashBase  = doc.HashBase;
            Keys      = static_cast<Array<String> &&>(doc.Keys);
            Table     = static_cast<Array<Index> &&>(doc.Table);
            Entries   = static_cast<Array<Entry> &&>(doc.Entries);
            Numbers   = static_cast<Array<double> &&>(doc.Numbers);
            Strings   = static_cast<Array<String> &&>(doc.Strings);
            Documents = static_cast<Array<Document> &&>(doc.Documents);
            return *this;
        }

        Document *storage = nullptr;

        if (Ordered) {
            if (LastKeyLen < Entries.Size) {
                Insert(--LastKeyLen, VType::DocumentT, &doc, true);
                storage = &(Documents[Entries[LastKeyLen].ArrayID]);
            } else {
                storage = this;
            }
        } else {
            Insert(LastKey, 0, LastKeyLen, VType::DocumentT, &doc, true, true);
            Entry *_entry;
            storage = GetSource(&_entry, LastKey, 0, LastKeyLen);
        }

        LastKey    = nullptr;
        LastKeyLen = 0;

        return *storage;
    }

    Document &operator=(Document const &doc) noexcept {
        if (LastKeyLen == 0) {
            Ordered   = doc.Ordered;
            HashBase  = doc.HashBase;
            Keys      = doc.Keys;
            Table     = doc.Table;
            Entries   = doc.Entries;
            Numbers   = doc.Numbers;
            Strings   = doc.Strings;
            Documents = doc.Documents;
            return *this;
        }

        Document *storage  = nullptr;
        Document  doc_copy = doc;

        if (Ordered) {
            if (LastKeyLen < Entries.Size) {
                Insert(--LastKeyLen, VType::DocumentT, &doc_copy, true);
                storage = &(Documents[Entries[LastKeyLen].ArrayID]);
            } else {
                storage = this;
            }
        } else {
            Insert(LastKey, 0, LastKeyLen, VType::DocumentT, &doc_copy, true, true);

            Entry *_entry;
            storage = GetSource(&_entry, LastKey, 0, LastKeyLen);
        }

        LastKey    = nullptr;
        LastKeyLen = 0;

        return *storage;
    }

    Document &operator=(wchar_t const *str) noexcept {
        Document doc;

        if (str != nullptr) {
            Array<Match> items = Engine::Search(str, _getJsonExpres(), 0, String::Count(str));

            if (items.Size != 0) {
                if ((doc.Ordered = (items[0].Expr->ID == 2))) {
                    _makeOrderedList(doc, str, items[0]);
                } else {
                    _makeList(doc, str, items[0]);
                }
            }

            items.Reset();
        }

        if (LastKeyLen == 0) {
            *this = doc;
            return *this;
        }

        if (Ordered) {
            --LastKeyLen;

            if (str != nullptr) {
                if (doc.Entries.Size != 0) {
                    Insert(LastKeyLen, VType::DocumentT, &doc, true);
                } else {
                    String string = str;
                    Insert(LastKeyLen, VType::StringT, &string, true);
                }
            } else {
                Insert(LastKeyLen, VType::NullT, nullptr, false);
            }
        } else {
            if (str != nullptr) {
                if (doc.Entries.Size != 0) {
                    Insert(LastKey, 0, LastKeyLen, VType::DocumentT, &doc, true, true);
                } else {
                    String string = str;
                    Insert(LastKey, 0, LastKeyLen, VType::StringT, &string, true, true);
                }
            } else {
                Insert(LastKey, 0, LastKeyLen, VType::NullT, nullptr, false, true);
            }
        }

        LastKey    = nullptr;
        LastKeyLen = 0;

        return *this;
    }

    Document &operator=(double number) noexcept {
        if (LastKeyLen != 0) {

            if (Ordered) {
                Insert(--LastKeyLen, VType::NumberT, &number, false);
            } else {
                Insert(LastKey, 0, LastKeyLen, VType::NumberT, &number, false, true);
            }

            LastKey    = nullptr;
            LastKeyLen = 0;
        }

        return *this;
    }

    Document &operator=(int const num) noexcept {
        if (LastKeyLen != 0) {
            double number = static_cast<double>(num);

            if (Ordered) {
                Insert(--LastKeyLen, VType::NumberT, &number, false);
            } else {
                Insert(LastKey, 0, LastKeyLen, VType::NumberT, &number, false, true);
            }

            LastKey    = nullptr;
            LastKeyLen = 0;
        }

        return *this;
    }

    Document &operator=(long const num) noexcept {
        if (LastKeyLen != 0) {
            double number = static_cast<double>(num);

            if (Ordered) {
                Insert(--LastKeyLen, VType::NumberT, &number, false);
            } else {
                Insert(LastKey, 0, LastKeyLen, VType::NumberT, &number, false, true);
            }

            LastKey    = nullptr;
            LastKeyLen = 0;
        }

        return *this;
    }

    Document &operator=(bool const value) noexcept {
        if (LastKeyLen != 0) {
            double _bool = value ? 1.0 : 0.0;

            if (Ordered) {
                Insert(--LastKeyLen, VType::BooleanT, &_bool, false);
            } else {
                Insert(LastKey, 0, LastKeyLen, VType::BooleanT, &_bool, false, true);
            }

            LastKey    = nullptr;
            LastKeyLen = 0;
        }

        return *this;
    }

    void operator+=(bool const value) noexcept {
        if (Ordered) {
            Entries += Entry({VType::BooleanT, 0, Numbers.Size});
            Numbers += value ? 1.0 : 0.0;
        }
    }

    void operator+=(int const num) noexcept {
        if (Ordered) {
            Entries += Entry({VType::NumberT, 0, Numbers.Size});
            Numbers += static_cast<double>(num);
        }
    }

    void operator+=(long const num) noexcept {
        if (Ordered) {
            Entries += Entry({VType::NumberT, 0, Numbers.Size});
            Numbers += static_cast<double>(num);
        }
    }

    void operator+=(double const num) noexcept {
        if (Ordered) {
            Entries += Entry({VType::NumberT, 0, Numbers.Size});
            Numbers += num;
        }
    }

    void operator+=(wchar_t const *str) noexcept {
        if (str != nullptr) {
            Array<Match> const items = Engine::Search(str, _getJsonExpres(), 0, String::Count(str));

            if (items.Size != 0) {
                Document doc;

                if ((doc.Ordered = (items[0].Expr->ID == 2))) {
                    _makeOrderedList(doc, str, items[0]);
                } else {
                    _makeList(doc, str, items[0]);
                }

                *this += doc;
            } else if (Ordered) {
                Entries += Entry({VType::StringT, 0, Strings.Size});
                Strings += str;
            }
        } else if (Ordered) {
            Entries += Entry({VType::NullT, 0, 0});
        }
    }

    void operator+=(String const &string) noexcept {
        Array<Match> const items = Engine::Search(string.Str, _getJsonExpres(), 0, string.Length);

        if (items.Size != 0) {
            Document doc;

            if (items[0].Expr->ID == 1) {
                _makeList(doc, string.Str, items[0]);
            } else {
                _makeOrderedList(doc, string.Str, items[0]);
            }

            *this += doc;
        } else if (Ordered) {
            Entries += Entry({VType::StringT, 0, Strings.Size});
            Strings += string;
        }
    }

    void operator+=(String &&string) noexcept {
        Array<Match> const items = Engine::Search(string.Str, _getJsonExpres(), 0, string.Length);

        if (items.Size != 0) {
            Document doc;

            if (items[0].Expr->ID == 1) {
                _makeList(doc, string.Str, items[0]);
            } else if (items[0].NestMatch.Size != 0) {
                _makeOrderedList(doc, string.Str, items[0]);
            }

            *this += doc;
        } else if (Ordered) {
            Entries += Entry({VType::StringT, 0, Strings.Size});
            Strings += static_cast<String &&>(string);
        }
    }

    void operator+=(Array<double> const &_numbers) noexcept {
        if (Ordered) {
            UNumber id = Numbers.Size;

            for (UNumber i = 0; i < _numbers.Size; i++) {
                Entries += Entry({VType::NumberT, 0, id++});
            }

            Numbers += _numbers;
        }
    }

    void operator+=(Array<String> const &_strings) noexcept {
        if (Ordered) {
            UNumber id = Strings.Size;

            for (UNumber i = 0; i < _strings.Size; i++) {
                Entries += Entry({VType::StringT, 0, id++});
            }

            Strings += _strings;
        }
    }

    void operator+=(Array<Document> const &_documents) noexcept {
        if (Ordered) {
            UNumber id = Documents.Size;

            for (UNumber i = 0; i < _documents.Size; i++) {
                Entries += Entry({VType::DocumentT, 0, id++});
            }

            Documents += _documents;
        }
    }

    void operator+=(Array<double> &&_numbers) noexcept {
        if (Ordered) {
            UNumber id = Numbers.Size;

            for (UNumber i = 0; i < _numbers.Size; i++) {
                Entries += Entry({VType::NumberT, 0, id++});
            }

            Numbers += _numbers;
        }
    }

    void operator+=(Array<String> &&_strings) noexcept {
        if (Ordered) {
            UNumber id = Strings.Size;

            for (UNumber i = 0; i < _strings.Size; i++) {
                Entries += Entry({VType::StringT, 0, id++});
            }

            Strings += static_cast<Array<String> &&>(_strings);
        }
    }

    void operator+=(Array<Document> &&_documents) noexcept {
        if (Ordered) {
            UNumber id = Documents.Size;

            for (UNumber i = 0; i < _documents.Size; i++) {
                Entries += Entry({VType::DocumentT, 0, id++});
            }

            Documents += static_cast<Array<Document> &&>(_documents);
        }
    }

    Document &operator[](wchar_t const *key) noexcept {
        UNumber const len = String::Count(key);

        Entry *   _entry;
        Document *src = GetSource(&_entry, key, 0, len);

        if ((src != nullptr) && (src != this)) {
            src->LastKey    = key;
            src->LastKeyLen = len;

            LastKey    = nullptr;
            LastKeyLen = 0;

            return *src;
        }

        LastKey    = key;
        LastKeyLen = len;

        return *this;
    }

    Document &operator[](String const &key) noexcept {
        Entry *   _entry;
        Document *src = GetSource(&_entry, key.Str, 0, key.Length);

        if ((src != nullptr) && (src != this)) {
            src->LastKey    = key.Str;
            src->LastKeyLen = key.Length;

            LastKey    = nullptr;
            LastKeyLen = 0;

            return *src;
        }

        LastKey    = key.Str;
        LastKeyLen = key.Length;

        return *this;
    }

    Document &operator[](UNumber const id) noexcept {
        if (Ordered) {
            LastKeyLen = id + 1;
        } else {
            LastKey    = Keys[Entries[id].KeyID].Str;
            LastKeyLen = Keys[Entries[id].KeyID].Length;

            Entry *   _entry;
            Document *src = GetSource(&_entry, LastKey, 0, LastKeyLen);

            if ((src != nullptr) && (src != this)) {
                src->LastKey    = LastKey;
                src->LastKeyLen = LastKeyLen;

                LastKey    = nullptr;
                LastKeyLen = 0;

                return *src;
            }
        }

        return *this;
    }

    Document &operator[](int const id) noexcept {
        if (Ordered) {
            LastKeyLen = static_cast<UNumber>(id) + 1;
        } else {
            LastKeyLen = static_cast<UNumber>(id);
            LastKey    = Keys[Entries[LastKeyLen].KeyID].Str;
            LastKeyLen = Keys[Entries[LastKeyLen].KeyID].Length;

            Entry *   _entry;
            Document *src = GetSource(&_entry, LastKey, 0, LastKeyLen);

            if ((src != nullptr) && (src != this)) {
                src->LastKey    = LastKey;
                src->LastKeyLen = LastKeyLen;

                LastKey    = nullptr;
                LastKeyLen = 0;

                return *src;
            }
        }

        return *this;
    }
};

} // namespace Qentem

#endif
