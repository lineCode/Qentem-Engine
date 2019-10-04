
/**
 * Qentem Document
 *
 * @brief     Ordered/Unordered array with hasing capability and JSON build-in
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

enum VType { UndefinedT = 0, NullT, BooleanT, NumberT, StringT, DocumentT };

struct Index {
    UNumber      EntryID = 0;
    UNumber      Hash    = 0;
    Array<Index> Table;
};

struct Entry {
    UNumber ArrayID = 0;
    UNumber KeyID   = 0;
    VType   Type    = VType::UndefinedT;

    Entry() = default;
    Entry(UNumber d, UNumber k, VType t) : ArrayID(d), KeyID(k), Type(t) {
    }
};

struct _JsonFixedString {
    String const fss1   = L'{';
    String const fss2   = L'}';
    String const fss3   = L',';
    String const fss4   = L'[';
    String const fss5   = L']';
    String const fss6   = L'"';
    String const fsc1   = L':';
    String const fNull  = L"null";
    String const fTrue  = L"true";
    String const fFalse = L"false";
} static JFX;

struct Document {
    UNumber HashBase = 17; // Or 97; a prime number only!
    bool    Ordered  = false;

    Array<Entry> Entries;
    Array<Index> Table;

    Array<double>   Numbers;
    Array<String>   Strings;
    Array<String>   Keys;
    Array<Document> Documents;

    String lastKey;
    Document()          = default;
    virtual ~Document() = default;

    Document(Document &&doc) noexcept {
        HashBase  = doc.HashBase;
        Ordered   = doc.Ordered;
        Entries   = static_cast<Array<Entry> &&>(doc.Entries);
        Table     = static_cast<Array<Index> &&>(doc.Table);
        Numbers   = static_cast<Array<double> &&>(doc.Numbers);
        Strings   = static_cast<Array<String> &&>(doc.Strings);
        Keys      = static_cast<Array<String> &&>(doc.Keys);
        Documents = static_cast<Array<Document> &&>(doc.Documents);
    }

    Document(Document const &doc) noexcept {
        HashBase  = doc.HashBase;
        Ordered   = doc.Ordered;
        Entries   = doc.Entries;
        Table     = doc.Table;
        Numbers   = doc.Numbers;
        Strings   = doc.Strings;
        Keys      = doc.Keys;
        Documents = doc.Documents;
    }

    Document(Array<double> const &_numbers) noexcept {
        Numbers = _numbers;
        Entries.SetCapacity(_numbers.Index);

        for (UNumber i = 0; i < _numbers.Index; i++) {
            Entries += Entry(i, 0, VType::NumberT);
        }

        Ordered = true;
    }

    Document(Array<String> const &_strings) noexcept {
        Strings = _strings;
        Entries.SetCapacity(_strings.Index);

        for (UNumber i = 0; i < _strings.Index; i++) {
            Entries += Entry(i, 0, VType::StringT);
        }

        Ordered = true;
    }

    Document(Array<Document> const &_documents) noexcept {
        Documents = _documents;
        Entries.SetCapacity(_documents.Index);

        for (UNumber i = 0; i < _documents.Index; i++) {
            Entries += Entry(i, 0, VType::DocumentT);
        }

        Ordered = true;
    }

    Document(Array<double> &&_numbers) noexcept {
        Numbers = static_cast<Array<double> &&>(_numbers);
        Entries.SetCapacity(_numbers.Index);

        for (UNumber i = 0; i < _numbers.Index; i++) {
            Entries += Entry(i, 0, VType::NumberT);
        }

        Ordered = true;
    }

    Document(Array<String> &&_strings) noexcept {
        Strings = static_cast<Array<String> &&>(_strings);
        Entries.SetCapacity(_strings.Index);

        for (UNumber i = 0; i < _strings.Index; i++) {
            Entries += Entry(i, 0, VType::StringT);
        }

        Ordered = true;
    }

    Document(Array<Document> &&_documents) noexcept {
        Documents = static_cast<Array<Document> &&>(_documents);
        Entries.SetCapacity(_documents.Index);

        for (UNumber i = 0; i < _documents.Index; i++) {
            Entries += Entry(i, 0, VType::DocumentT);
        }

        Ordered = true;
    }

    Document(String const &value) noexcept {
        Array<Match> items = Engine::Search(value, _getJsonExpres(), 0, value.Length);

        if (items.Index == 0) {
            Ordered = true;
            Strings += value;
            Entries += Entry(0, 0, VType::StringT);
        } else {
            if ((Ordered = (items[0].Expr->ID == 2))) {
                _makeOrderedList(*this, value, items[0]);
            } else if (items[0].NestMatch.Index != 0) {
                _makeList(*this, value, items[0].NestMatch);
            }
        }
    }

    Document(wchar_t const *value) noexcept {
        String str = value;

        Array<Match> items = Engine::Search(str, _getJsonExpres(), 0, str.Length);

        if (items.Index == 0) {
            // Just a string.
            Ordered = true;
            Strings += str;
            Entries += Entry(0, 0, VType::StringT);
        } else {
            if ((Ordered = (items[0].Expr->ID == 2))) {
                _makeOrderedList(*this, str, items[0]);
            } else if (items[0].NestMatch.Index != 0) {
                _makeList(*this, str, items[0].NestMatch);
            }
        }
    }

    void Reset() noexcept {
        Entries.Reset();
        Table.Reset();
        Numbers.Reset();
        Strings.Reset();
        Keys.Reset();
        Documents.Reset();
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

    void Drop(String const &key, UNumber offset, UNumber limit) noexcept {
        Entry *   _entry;
        Document *storage = GetSource(&_entry, key, offset, limit);

        if (_entry != nullptr) {
            Drop(*_entry, *storage);
        }
    }

    void InsertIndex(Index const &_index, UNumber level, Array<Index> &_table) noexcept {
        UNumber id = ((_index.Hash + level) % HashBase);

        if (_table.Capacity <= id) {
            _table.Resize(id + 1);
            _table.Index = _table.Capacity;
        }

        if (_table[id].Hash == 0) {
            _table[id] = _index;
            return;
        }

        InsertIndex(_index, (level + id + 2), _table[id].Table);
    }

    Entry *Exist(UNumber hash, UNumber level, Array<Index> const &_table) const noexcept {
        UNumber id = ((hash + level) % HashBase);

        if ((_table.Index > id) && (_table[id].Hash != 0)) {
            if (_table[id].Hash == hash) {
                return &(Entries[_table[id].EntryID]);
            }

            return Exist(hash, (level + id + 2), _table[id].Table);
        }

        return nullptr;
    }

    void Insert(String const &key, UNumber offset, UNumber limit, VType const type, void *ptr, bool const move,
                bool const check = true) noexcept {
        UNumber       id    = 0;
        UNumber const _hash = String::Hash(key.Str, offset, (offset + limit));
        Entry *       _ent  = (!check) ? nullptr : Exist(_hash, 0, Table);

        if ((_ent == nullptr) || (_ent->Type != type)) {
            // New item.
            switch (type) {
                case VType::StringT: {
                    id = Strings.Index;
                    if (move) {
                        Strings += static_cast<String &&>(*(static_cast<String *>(ptr)));
                    } else {
                        Strings += *(static_cast<String *>(ptr));
                    }
                } break;
                case VType::NumberT:
                case VType::BooleanT: {
                    id = Numbers.Index;
                    Numbers += *(static_cast<double *>(ptr));
                } break;
                case VType::DocumentT: {
                    id = Documents.Index;
                    if (move) {
                        Documents += static_cast<Document &&>(*(static_cast<Document *>(ptr)));
                    } else {
                        Documents += static_cast<Document const &>(*(static_cast<Document *>(ptr)));
                    }
                } break;
                default:
                    break;
            }
        } else {
            // Updating an existing item.
            switch (type) {
                case VType::BooleanT:
                case VType::NumberT: {
                    Numbers[_ent->ArrayID] = *(static_cast<double *>(ptr));
                } break;
                case VType::StringT: {
                    if (move) {
                        Strings[_ent->ArrayID] = static_cast<String &&>(*(static_cast<String *>(ptr)));
                    } else {
                        Strings[_ent->ArrayID] = *(static_cast<String *>(ptr));
                    }
                } break;
                case VType::DocumentT: {
                    if (move) {
                        Documents[_ent->ArrayID] = static_cast<Document &&>(*(static_cast<Document *>(ptr)));
                    } else {
                        Documents[_ent->ArrayID] = *(static_cast<Document *>(ptr));
                    }
                } break;
                default:
                    break;
            }
        }

        if (_ent != nullptr) {
            // If exists ...
            if (_ent->Type != type) {
                // Clearing an existing value.
                switch (type) {
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
        _index.EntryID = Entries.Index;

        Entries += Entry(id, Keys.Index, type);
        Keys += String::Part(key.Str, offset, limit);

        InsertIndex(_index, 0, Table);
    }

    void Rehash(UNumber newBase, bool const children = false) noexcept {
        Table.Reset();
        HashBase = newBase;

        Index   _index;
        String *key;

        for (UNumber i = 0; i < Keys.Index; i++) {
            key            = &(Keys[Entries[i].KeyID]);
            _index.Hash    = String::Hash(key->Str, 0, key->Length);
            _index.EntryID = i;

            InsertIndex(_index, 0, Table);

            if (children && (Entries[i].Type == VType::DocumentT)) {
                Documents[Entries[i].ArrayID].Rehash(HashBase, true);
            }
        }
    }

    inline String ToJSON() const noexcept {
        StringStream ss;
        _toJSON(ss);
        return ss.Eject();
    }

    static void _makeOrderedList(Document &document, String const &content, Match &item) noexcept {
        UNumber offset = (item.Offset + 1); // the starting char [
        UNumber start  = offset;
        UNumber end    = (item.Length + item.Offset);

        UNumber nestID = 0;
        Match * subItem;

        UNumber limit;
        double  number;
        bool    pass = false;

        for (; offset < end; offset++) {
            switch (content[offset]) {
                case L'"': {
                    subItem = &item.NestMatch[nestID++];
                    document.Entries += Entry(document.Strings.Index, 0, VType::StringT);

                    if (subItem->NestMatch.Index == 0) {
                        document.Strings += String::Part(content.Str, (subItem->Offset + 1), (subItem->Length - 2));
                    } else {
                        document.Strings +=
                            Engine::Parse(content, subItem->NestMatch, (subItem->Offset + 1), (subItem->Length - 2));
                    }

                    offset = (subItem->Offset + subItem->Length) - 1;
                    pass   = true;
                    break;
                }
                case L'{': {
                    subItem = &item.NestMatch[nestID++];

                    document.Entries += Entry(document.Documents.Index, 0, VType::DocumentT);

                    Document doc;
                    _makeList(doc, content, subItem->NestMatch);
                    document.Documents += static_cast<Document &&>(doc);

                    offset = (subItem->Offset + subItem->Length) - 1;
                    pass   = true;
                    break;
                }
                case L'[': {
                    subItem = &item.NestMatch[nestID++];

                    document.Entries += Entry(document.Documents.Index, 0, VType::DocumentT);

                    Document doc;
                    doc.Ordered = true;
                    _makeOrderedList(doc, content, *subItem);
                    document.Documents += static_cast<Document &&>(doc);

                    offset = (subItem->Offset + subItem->Length) - 1;
                    pass   = true;
                    break;
                }
                case L',':
                case L']': {
                    if (!pass) {
                        // A Number, true/false or null
                        limit = (offset - start);
                        String::SoftTrim(content.Str, start, limit);

                        switch (content.Str[start]) {
                            case L't': {
                                // True
                                document.Entries += Entry(document.Numbers.Index, 0, VType::BooleanT);
                                document.Numbers += 1;
                                break;
                            }
                            case L'f': {
                                // False
                                document.Entries += Entry(document.Numbers.Index, 0, VType::BooleanT);
                                document.Numbers += 0;
                                break;
                            }
                            case L'n': {
                                // Null
                                document.Entries += Entry(0, 0, VType::NullT);
                                break;
                            }
                            default: {
                                // A number
                                if (String::ToNumber(content, number, start, limit)) {
                                    document.Entries += Entry(document.Numbers.Index, 0, VType::NumberT);
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
            }
        }
    }

    static void _makeList(Document &document, String const &content, Array<Match> &items) noexcept {
        Match * key;
        Match * data;
        UNumber start;
        UNumber j;
        bool    done;

        UNumber limit;
        double  number;

        String tmpString;

        for (UNumber i = 0; i < items.Index; i++) {
            key   = &(items[i]);
            start = 0;
            done  = false;
            j     = (key->Offset + key->Length);

            for (; j < content.Length; j++) {
                switch (content[j]) {
                    case L'"': {
                        ++i;

                        data = &(items[i]);
                        if (data->NestMatch.Index != 0) {
                            tmpString = Engine::Parse(content, data->NestMatch, (data->Offset + 1), (data->Length - 2));
                        } else {
                            tmpString = String::Part(content.Str, (data->Offset + 1), (data->Length - 2));
                        }

                        document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::StringT, &tmpString, true,
                                        false);

                        done = true;
                        break;
                    }
                    case L'{': {
                        ++i;

                        Document uno_document;
                        _makeList(uno_document, content, items[i].NestMatch);
                        document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::DocumentT, &uno_document,
                                        true, false);

                        done = true;
                        break;
                    }
                    case L'[': {
                        ++i;

                        Document o_document;
                        o_document.Ordered = true;

                        _makeOrderedList(o_document, content, items[i]);

                        document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::DocumentT, &o_document,
                                        true, false);

                        done = true;
                        break;
                    }
                    case L':': {
                        start = j + 1;
                        continue;
                    }
                    case L',':
                    case L'}': {
                        // A true, false, null or number value.
                        limit = j - start;
                        String::SoftTrim(content.Str, start, limit);

                        switch (content[start]) {
                            case L't': {
                                // True
                                number = 1;
                                document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::BooleanT, &number,
                                                false, false);
                                break;
                            }
                            case L'f': {
                                // False
                                number = 0;
                                document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::BooleanT, &number,
                                                false, false);
                                break;
                            }
                            case L'n': {
                                // Null
                                document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::NullT, nullptr,
                                                false, false);
                                break;
                            }
                            default: {
                                if (String::ToNumber(content, number, start, limit)) {
                                    // Number
                                    document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::NumberT,
                                                    &number, false, false);
                                }
                                break;
                            }
                        }

                        done = true;
                    } break;
                }

                if (done) {
                    break;
                }
            }
        }

        items.Reset();
    }

    static Document FromJSON(String const &content, bool comments = false) noexcept {
        Document     document;
        Array<Match> items;

        String n_content;

        static Expressions const &json_expres = _getJsonExpres();

        // C style comments
        if (!comments) {
            items = Engine::Search(content, json_expres, 0, content.Length);
        } else {
            static Expressions __comments;
            if (__comments.Index == 0) {
                static Expression comment1      = Expression();
                static Expression comment_next1 = Expression();
                comment1.Keyword                = L"/*";
                comment_next1.Keyword           = L"*/";
                comment_next1.Replace           = L'\n';
                comment1.Connected              = &comment_next1;

                static Expression comment2      = Expression();
                static Expression comment_next2 = Expression();
                comment2.Keyword                = L"//";
                comment_next2.Keyword           = L'\n';
                comment_next2.Replace           = L'\n';
                comment2.Connected              = &comment_next2;

                __comments = Expressions().Add(&comment1).Add(&comment2);
            }

            n_content =
                Engine::Parse(content, Engine::Search(content, __comments, 0, content.Length), 0, content.Length);
            items = Engine::Search(n_content, json_expres, 0, n_content.Length);
        }

        if (items.Index != 0) {
            Match *_item = &(items[0]);

            document.Ordered = (_item->Expr->ID == 2);
            if (document.Ordered) {
                _makeOrderedList(document, (n_content.Length == 0) ? content : n_content, *_item);
            } else if (_item->NestMatch.Index != 0) {
                _makeList(document, (n_content.Length == 0) ? content : n_content, _item->NestMatch);
            }
        }

        return document;
    }

    static bool ExtractID(UNumber &id, String const &key, UNumber offset, UNumber limit) noexcept {
        UNumber end = (offset + (limit - 1));

        while ((end > offset) && (key[--end] != L'[')) {
        }
        ++end;
        --limit;

        return String::ToNumber(key, id, end, (limit - (end - offset)));
    }

    // TODO: Rewrite
    // Key form can be: name, name[id1], name[id1][sub-id2], name[id1][sub-id2][sub-sub-idx]...
    Document *GetSource(Entry **_entry, String const &key, UNumber offset, UNumber limit) noexcept {
        if (limit == 0) {
            limit = key.Length - offset;
        }

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

            _id = String::Hash(key.Str, offset, end_offset);
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
            _id = String::Hash(key.Str, offset, end_offset);
        } else {
            _id = String::Hash(key.Str, offset, end_offset);
        }

        *_entry = Exist(_id, 0, Table);
        if (*_entry != nullptr) {
            if ((*_entry)->Type == VType::DocumentT) {
                limit -= (end_offset - offset);
                Document *doc = &(Documents[(*_entry)->ArrayID]);

                if (limit != 0) {
                    if (doc->Ordered) {
                        if (ExtractID(_id, key, end_offset, limit) && (doc->Entries.Index > _id)) {
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

    bool GetString(String &value, String const &key, UNumber offset = 0, UNumber limit = 0) noexcept {
        value = L"";

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

    bool GetNumber(UNumber &value, String const &key, UNumber offset = 0, UNumber limit = 0) noexcept {
        value = 0;

        Entry *         _entry;
        Document const *storage = GetSource(&_entry, key, offset, limit);

        if (storage != nullptr) {
            if (_entry->Type == VType::StringT) {
                return String::ToNumber(storage->Strings[_entry->ArrayID], value);
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

    bool GetDouble(double &value, String const &key, UNumber offset = 0, UNumber limit = 0) noexcept {
        value = 0.0;

        Entry *         _entry;
        Document const *storage = GetSource(&_entry, key, offset, limit);

        if (storage != nullptr) {
            if (_entry->Type == VType::StringT) {
                return String::ToNumber(storage->Strings[_entry->ArrayID], value);
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

    bool GetBool(bool &value, String const &key, UNumber offset = 0, UNumber limit = 0) noexcept {
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

    Document *GetDocument(String const &key, UNumber offset = 0, UNumber limit = 0) noexcept {
        Entry *   _entry;
        Document *storage = GetSource(&_entry, key, offset, limit);

        if ((storage != nullptr) && (_entry->Type == VType::DocumentT)) {
            return storage;
        }

        return nullptr;
    }

    void _toJSON(StringStream &ss) const noexcept {
        static Expressions const &to_json_expres = _getToJsonExpres();

        Entry *      _entry;
        Array<Match> tmpMatchs;
        String *     str_ptr;

        if (Ordered) {
            ss.Share(&JFX.fss4);

            for (UNumber i = 0; i < Entries.Index; i++) {
                _entry = &(Entries[i]);

                if (ss.Length != 1) {
                    ss.Share(&JFX.fss3);
                }

                switch (_entry->Type) {
                    case VType::StringT: {
                        ss.Share(&JFX.fss6);

                        str_ptr   = &(Strings[_entry->ArrayID]);
                        tmpMatchs = Engine::Search(*str_ptr, to_json_expres, 0, str_ptr->Length);
                        if (tmpMatchs.Index == 0) {
                            ss.Share(str_ptr);
                        } else {
                            ss += Engine::Parse(*str_ptr, tmpMatchs, 0, str_ptr->Length);
                        }

                        ss.Share(&JFX.fss6);
                    } break;
                    case VType::NumberT: {
                        ss += String::FromNumber(Numbers[_entry->ArrayID]);
                    } break;
                    case VType::DocumentT: {
                        ss += Documents[_entry->ArrayID].ToJSON();
                    } break;
                    case VType::NullT: {
                        ss.Share(&JFX.fNull);
                    } break;
                    case VType::BooleanT: {
                        ss.Share((Numbers[_entry->ArrayID] == 0) ? &JFX.fFalse : &JFX.fTrue);
                    } break;
                    default:
                        break;
                }
            }

            ss.Share(&JFX.fss5);
        } else {
            ss.Share(&JFX.fss1);

            for (UNumber i = 0; i < Entries.Index; i++) {
                _entry = &(Entries[i]);

                if (ss.Length != 1) {
                    ss.Share(&JFX.fss3);
                }

                switch (_entry->Type) {
                    case VType::StringT: {
                        ss.Share(&JFX.fss6);
                        ss += Keys[_entry->KeyID];
                        ss.Share(&JFX.fss6);
                        ss.Share(&JFX.fsc1);
                        ss.Share(&JFX.fss6);

                        str_ptr   = &(Strings[_entry->ArrayID]);
                        tmpMatchs = Engine::Search(*str_ptr, to_json_expres, 0, str_ptr->Length);
                        if (tmpMatchs.Index == 0) {
                            ss.Share(str_ptr);
                        } else {
                            ss += Engine::Parse(*str_ptr, tmpMatchs, 0, str_ptr->Length);
                        }

                        ss.Share(&JFX.fss6);
                    } break;
                    case VType::NumberT: {
                        ss.Share(&JFX.fss6);
                        ss += Keys[_entry->KeyID];
                        ss.Share(&JFX.fss6);
                        ss.Share(&JFX.fsc1);
                        ss += String::FromNumber(Numbers[_entry->ArrayID]);
                    } break;
                    case VType::DocumentT: {
                        ss.Share(&JFX.fss6);
                        ss += Keys[_entry->KeyID];
                        ss.Share(&JFX.fss6);
                        ss.Share(&JFX.fsc1);
                        ss += Documents[_entry->ArrayID].ToJSON();
                    } break;
                    case VType::NullT: {
                        ss.Share(&JFX.fss6);
                        ss += Keys[_entry->KeyID];
                        ss.Share(&JFX.fss6);
                        ss.Share(&JFX.fsc1);
                        ss.Share(&JFX.fNull);
                    } break;
                    case VType::BooleanT: {
                        ss.Share(&JFX.fss6);
                        ss += Keys[_entry->KeyID];
                        ss.Share(&JFX.fss6);
                        ss.Share(&JFX.fsc1);

                        ss.Share((Numbers[_entry->ArrayID] == 0) ? &JFX.fFalse : &JFX.fTrue);
                    } break;
                    default:
                        break;
                }
            }

            ss.Share(&JFX.fss2);
        }
    }

    static Expressions const &_getToJsonExpres() noexcept {
        static Expression  _JsonEsc;
        static Expression  _JsonQuot;
        static Expressions tags;

        if (tags.Index == 0) {
            _JsonEsc.Keyword = L'\\';
            _JsonEsc.MatchCB = ([](String const &content, UNumber &offset, UNumber const endOffset, Match &item,
                                   Array<Match> &items) noexcept->bool {
                UNumber _offset = offset;
                if ((content[++_offset] == L'\\') || (content[_offset] == L' ') || (_offset == endOffset)) {
                    // If there is a space after \ or \ or it's at the end, then it's a match.
                    items += static_cast<Match &&>(item);
                }

                return true;
            });
            _JsonEsc.Replace = L"\\\\";

            _JsonQuot.Keyword = L'"';
            _JsonQuot.Replace = L"\\\"";

            tags += &_JsonEsc;
            tags += &_JsonQuot;
        }

        return tags;
    }

    static Expressions const &_getJsonExpres() noexcept {
        static Expression esc_quotation = Expression();
        static Expression esc_esc       = Expression();

        static Expression quotation_start = Expression();
        static Expression quotation_end   = Expression();

        static Expression opened_square_bracket = Expression();
        static Expression closed_square_bracket = Expression();

        static Expression opened_curly_bracket = Expression();
        static Expression closed_curly_bracket = Expression();

        static Expressions tags;

        if (tags.Index == 0) {
            esc_esc.Keyword = L"\\\\";
            esc_esc.Replace = L"\\";

            esc_quotation.Keyword = L"\\\"";
            esc_quotation.Replace = L'"';

            quotation_start.Keyword   = L'"';
            quotation_end.Keyword     = L'"';
            quotation_end.ID          = 3;
            quotation_start.Connected = &quotation_end;
            quotation_end.NestExprs += &esc_esc;
            quotation_end.NestExprs += &esc_quotation;

            opened_curly_bracket.Keyword   = L'{';
            closed_curly_bracket.Keyword   = L'}';
            closed_curly_bracket.ID        = 1;
            opened_curly_bracket.Connected = &closed_curly_bracket;

            closed_curly_bracket.NestExprs =
                Expressions().Add(&quotation_start).Add(&opened_curly_bracket).Add(&opened_square_bracket);

            opened_square_bracket.Keyword   = L'[';
            closed_square_bracket.Keyword   = L']';
            closed_square_bracket.ID        = 2;
            opened_square_bracket.Connected = &closed_square_bracket;

            closed_square_bracket.NestExprs =
                Expressions().Add(&opened_square_bracket).Add(&opened_curly_bracket).Add(&quotation_start);

            tags = Expressions().Add(&opened_curly_bracket).Add(&opened_square_bracket);
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
            for (UNumber i = 0; i < doc.Entries.Index; i++) {
                _entry = &(doc.Entries[i]);

                switch (_entry->Type) {
                    case VType::StringT: {
                        Entries += Entry(Strings.Index, 0, VType::StringT);
                        Strings += doc.Strings[_entry->ArrayID];
                        break;
                    }
                    case VType::DocumentT: {
                        Entries += Entry(Documents.Index, 0, VType::DocumentT);
                        Documents += doc.Documents[_entry->ArrayID];
                        break;
                    }
                    case VType::NumberT: {
                        Entries += Entry(Numbers.Index, 0, VType::NumberT);
                        Numbers += doc.Numbers[_entry->ArrayID];
                        break;
                    }
                    case VType::BooleanT: {
                        Entries += Entry(Numbers.Index, 0, VType::BooleanT);
                        Numbers += doc.Numbers[_entry->ArrayID];
                        break;
                    }
                    case VType::NullT: {
                        Entries += Entry(0, 0, VType::NullT);
                        break;
                    }
                    default:
                        break;
                }
            }
        } else {
            String *_key;

            for (UNumber i = 0; i < doc.Entries.Index; i++) {
                _entry = &(doc.Entries[i]);
                _key   = &(doc.Keys[doc.Entries[i].KeyID]);

                switch (_entry->Type) {
                    case VType::StringT: {
                        Insert(*_key, 0, _key->Length, _entry->Type, &doc.Strings[_entry->ArrayID], false, true);
                        break;
                    }
                    case VType::DocumentT: {
                        Insert(*_key, 0, _key->Length, _entry->Type, &doc.Documents[_entry->ArrayID], false, true);
                        break;
                    }
                    case VType::NumberT: {
                        Insert(*_key, 0, _key->Length, _entry->Type, &doc.Numbers[_entry->ArrayID], false, true);
                        break;
                    }
                    case VType::BooleanT: {
                        Insert(*_key, 0, _key->Length, _entry->Type, &doc.Numbers[_entry->ArrayID], false, true);
                        break;
                    }
                    case VType::NullT: {
                        Insert(*_key, 0, _key->Length, _entry->Type, nullptr, false, true);
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
            for (UNumber i = 0; i < doc.Entries.Index; i++) {
                _entry = &(doc.Entries[i]);

                switch (_entry->Type) {
                    case VType::StringT: {
                        Entries += Entry(Strings.Index, 0, VType::StringT);
                        Strings += static_cast<String &&>(doc.Strings[_entry->ArrayID]);
                        break;
                    }
                    case VType::DocumentT: {
                        Entries += Entry(Documents.Index, 0, VType::DocumentT);
                        Documents += static_cast<Document &&>(doc.Documents[_entry->ArrayID]);
                        break;
                    }
                    case VType::NumberT: {
                        Entries += Entry(Numbers.Index, 0, VType::NumberT);
                        Numbers += doc.Numbers[_entry->ArrayID];
                        break;
                    }
                    case VType::BooleanT: {
                        Entries += Entry(Numbers.Index, 0, VType::BooleanT);
                        Numbers += doc.Numbers[_entry->ArrayID];
                        break;
                    }
                    case VType::NullT: {
                        Entries += Entry(0, 0, VType::NullT);
                        break;
                    }
                    default:
                        break;
                }
            }
        } else {
            String *_key;

            for (UNumber i = 0; i < doc.Entries.Index; i++) {
                _entry = &(doc.Entries[i]);
                _key   = &(doc.Keys[doc.Entries[i].KeyID]);

                switch (_entry->Type) {
                    case VType::StringT: {
                        Insert(*_key, 0, _key->Length, _entry->Type, &doc.Strings[_entry->ArrayID], true, true);
                        break;
                    }
                    case VType::DocumentT: {
                        Insert(*_key, 0, _key->Length, _entry->Type, &doc.Documents[_entry->ArrayID], true, true);
                        break;
                    }
                    case VType::NumberT: {
                        Insert(*_key, 0, _key->Length, _entry->Type, &doc.Numbers[_entry->ArrayID], false, true);
                        break;
                    }
                    case VType::BooleanT: {
                        Insert(*_key, 0, _key->Length, _entry->Type, &doc.Numbers[_entry->ArrayID], false, true);
                        break;
                    }
                    case VType::NullT: {
                        Insert(*_key, 0, _key->Length, _entry->Type, nullptr, false, true);
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }

    Document &operator=(Document &&doc) noexcept {
        if (lastKey.Length == 0) {
            HashBase  = doc.HashBase;
            Ordered   = doc.Ordered;
            Entries   = static_cast<Array<Entry> &&>(doc.Entries);
            Table     = static_cast<Array<Index> &&>(doc.Table);
            Numbers   = static_cast<Array<double> &&>(doc.Numbers);
            Strings   = static_cast<Array<String> &&>(doc.Strings);
            Keys      = static_cast<Array<String> &&>(doc.Keys);
            Documents = static_cast<Array<Document> &&>(doc.Documents);
            return *this;
        }

        if (Ordered) {
            // Unpacking
            UNumber id;
            if (String::ToNumber(lastKey, id)) {
                Documents[id] = static_cast<Document &&>(doc);
            }
        } else {
            Insert(lastKey, 0, lastKey.Length, VType::DocumentT, &doc, true, true);
        }

        Entry *   _entry;
        Document *storage = GetSource(&_entry, lastKey, 0, lastKey.Length);
        lastKey.Reset();
        return *storage;
    }

    Document &operator=(Document const &doc) noexcept {
        if (lastKey.Length == 0) {
            HashBase  = doc.HashBase;
            Ordered   = doc.Ordered;
            Entries   = doc.Entries;
            Table     = doc.Table;
            Numbers   = doc.Numbers;
            Strings   = doc.Strings;
            Keys      = doc.Keys;
            Documents = doc.Documents;
            return *this;
        }

        if (Ordered) {
            // Unpacking
            UNumber id;
            if (String::ToNumber(lastKey, id)) {
                Documents[id] = doc;
            }
        } else {
            Document in = doc;
            Insert(lastKey, 0, lastKey.Length, VType::DocumentT, &in, true, true);
        }

        Entry *   _entry;
        Document *storage = GetSource(&_entry, lastKey, 0, lastKey.Length);
        lastKey.Reset();
        return *storage;
    }

    Document &operator=(wchar_t const *value) noexcept {
        if (lastKey.Length == 0) {
            *this = Document::FromJSON(value);
            return *this;
        }

        if (Ordered) {
            // Unpacking
            UNumber id;
            if (String::ToNumber(lastKey, id)) {
                if (value != nullptr) {
                    Strings[id] = value;
                } else {
                    Entries[id].Type = VType::NullT;
                    Strings[id].Reset();
                }
            }
        } else {
            if (value != nullptr) {
                String str = value;
                Insert(lastKey, 0, lastKey.Length, VType::StringT, &str, true, true);
            } else {
                Insert(lastKey, 0, lastKey.Length, VType::NullT, nullptr, false, true);
            }
        }

        lastKey.Reset();
        return *this;
    }

    Document &operator=(String const &value) noexcept {
        if (lastKey.Length == 0) {
            *this = Document::FromJSON(value);
            return *this;
        }

        if (Ordered) {
            // Unpacking
            UNumber id;
            if (String::ToNumber(lastKey, id)) {
                Strings[id] = value;
            }
        } else {
            String str = value;
            Insert(lastKey, 0, lastKey.Length, VType::StringT, &str, true, true);
        }

        lastKey.Reset();
        return *this;
    }

    Document &operator=(String &&value) noexcept {
        if (lastKey.Length == 0) {
            *this = Document::FromJSON(value);
            return *this;
        }

        if (Ordered) {
            // Unpacking
            UNumber id;
            if (String::ToNumber(lastKey, id)) {
                Strings[id] = static_cast<String &&>(value);
            }
        } else {
            Insert(lastKey, 0, lastKey.Length, VType::StringT, &value, true, true);
        }

        lastKey.Reset();
        return *this;
    }

    Document &operator=(double value) noexcept {
        if (lastKey.Length != 0) {
            if (Ordered) {
                // Unpacking
                UNumber id;
                if (String::ToNumber(lastKey, id)) {
                    Numbers[id] = value;
                }
            } else {
                Insert(lastKey, 0, lastKey.Length, VType::NumberT, &value, false, true);
            }

            lastKey.Reset();
        }
        return *this;
    }

    Document &operator=(int value) noexcept {
        if (lastKey.Length != 0) {
            double num = static_cast<double>(value);

            if (Ordered) {
                // Unpacking
                UNumber id;
                if (String::ToNumber(lastKey, id)) {
                    Numbers[id] = num;
                }
            } else {
                Insert(lastKey, 0, lastKey.Length, VType::NumberT, &num, false, true);
            }

            lastKey.Reset();
        }
        return *this;
    }

    Document &operator=(long value) noexcept {
        if (lastKey.Length != 0) {
            double num = static_cast<double>(value);

            if (Ordered) {
                // Unpacking
                UNumber id;
                if (String::ToNumber(lastKey, id)) {
                    Numbers[id] = num;
                }
            } else {
                Insert(lastKey, 0, lastKey.Length, VType::NumberT, &num, false, true);
            }

            lastKey.Reset();
        }
        return *this;
    }

    Document &operator=(bool value) noexcept {
        if (lastKey.Length != 0) {
            double num = value ? 1.0 : 0.0;

            if (Ordered) {
                // Unpacking
                UNumber id;
                if (String::ToNumber(lastKey, id)) {
                    Numbers[id] = num;
                }
            } else {
                Insert(lastKey, 0, lastKey.Length, VType::BooleanT, &num, false, true);
            }

            lastKey.Reset();
        }
        return *this;
    }

    void operator+=(bool value) noexcept {
        if (Ordered) {
            Entries += Entry(Numbers.Index, 0, VType::BooleanT);
            Numbers += value ? 1.0 : 0.0;
        }
    }

    void operator+=(int num) noexcept {
        if (Ordered) {
            Entries += Entry(Numbers.Index, 0, VType::NumberT);
            Numbers += static_cast<double>(num);
        }
    }

    void operator+=(long num) noexcept {
        if (Ordered) {
            Entries += Entry(Numbers.Index, 0, VType::NumberT);
            Numbers += static_cast<double>(num);
        }
    }

    void operator+=(double num) noexcept {
        if (Ordered) {
            Entries += Entry(Numbers.Index, 0, VType::NumberT);
            Numbers += num;
        }
    }

    void operator+=(wchar_t const *string) noexcept {
        String str = string;

        Array<Match> items = Engine::Search(str, _getJsonExpres(), 0, str.Length);

        if (items.Index != 0) {
            Document doc;
            if ((doc.Ordered = (items[0].Expr->ID == 2))) {
                _makeOrderedList(doc, str, items[0]);
            } else if (items[0].NestMatch.Index != 0) {
                _makeList(doc, str, items[0].NestMatch);
            }
            *this += doc;
        } else if (Ordered) {
            Entries += Entry(Strings.Index, 0, VType::StringT);
            Strings += str;
        }
    }

    void operator+=(String const &string) noexcept {
        Array<Match> items = Engine::Search(string, _getJsonExpres(), 0, string.Length);

        if (items.Index != 0) {
            Document doc;
            if ((doc.Ordered = (items[0].Expr->ID == 2))) {
                _makeOrderedList(doc, string, items[0]);
            } else if (items[0].NestMatch.Index != 0) {
                _makeList(doc, string, items[0].NestMatch);
            }
            *this += doc;
        } else if (Ordered) {
            Entries += Entry(Strings.Index, 0, VType::StringT);
            Strings += string;
        }
    }

    void operator+=(String &&string) noexcept {
        Array<Match> items = Engine::Search(string, _getJsonExpres(), 0, string.Length);

        if (items.Index != 0) {
            Document doc;
            if ((doc.Ordered = (items[0].Expr->ID == 2))) {
                _makeOrderedList(doc, string, items[0]);
            } else if (items[0].NestMatch.Index != 0) {
                _makeList(doc, string, items[0].NestMatch);
            }
            *this += doc;
        } else if (Ordered) {
            Entries += Entry(Strings.Index, 0, VType::StringT);
            Strings += static_cast<String &&>(string);
        }
    }

    void operator+=(Array<double> const &_numbers) noexcept {
        if (Ordered) {
            UNumber id = Numbers.Index;
            for (UNumber i = 0; i < _numbers.Index; i++) {
                Entries += Entry(id++, 0, VType::NumberT);
            }

            Numbers += _numbers;
        }
    }

    void operator+=(Array<String> const &_strings) noexcept {
        if (Ordered) {
            UNumber id = Strings.Index;
            for (UNumber i = 0; i < _strings.Index; i++) {
                Entries += Entry(id++, 0, VType::StringT);
            }

            Strings += _strings;
        }
    }

    void operator+=(Array<Document> const &_documents) noexcept {
        if (Ordered) {
            UNumber id = Documents.Index;
            for (UNumber i = 0; i < _documents.Index; i++) {
                Entries += Entry(id++, 0, VType::DocumentT);
            }

            Documents += _documents;
        }
    }

    void operator+=(Array<double> &&_numbers) noexcept {
        if (Ordered) {
            UNumber id = Numbers.Index;
            for (UNumber i = 0; i < _numbers.Index; i++) {
                Entries += Entry(id++, 0, VType::NumberT);
            }

            Numbers += _numbers;
        }
    }

    void operator+=(Array<String> &&_strings) noexcept {
        if (Ordered) {
            UNumber id = Strings.Index;
            for (UNumber i = 0; i < _strings.Index; i++) {
                Entries += Entry(id++, 0, VType::StringT);
            }

            Strings += static_cast<Array<String> &&>(_strings);
        }
    }

    void operator+=(Array<Document> &&_documents) noexcept {
        if (Ordered) {
            UNumber id = Documents.Index;
            for (UNumber i = 0; i < _documents.Index; i++) {
                Entries += Entry(id++, 0, VType::DocumentT);
            }

            Documents += static_cast<Array<Document> &&>(_documents);
        }
    }

    Document &operator[](String const &key) noexcept {
        Entry *_entry;

        Document *src = GetSource(&_entry, key, 0, key.Length);
        if (src != nullptr) {
            src->lastKey = key;
            return *src;
        }

        lastKey = key;
        return *this;
    }

    Document &operator[](wchar_t key) noexcept {
        Entry *_entry;

        Document *src = GetSource(&_entry, key, 0, 1);
        if (src != nullptr) {
            src->lastKey = key;
            return *src;
        }

        lastKey = key;
        return *this;
    }

    Document &operator[](UNumber id) noexcept {
        if (!Ordered) {
            lastKey = Keys[Entries[id].KeyID];

            Entry *   _entry;
            Document *src = GetSource(&_entry, lastKey, 0, lastKey.Length);
            if (src != nullptr) {
                src->lastKey = lastKey;
                return *src;
            }
        } else {
            // I don't want to expand the size of Document by adding lastID , so this should do just find.
            lastKey = String::FromNumber(static_cast<double>(id), 1, 0, 0);
        }

        return *this;
    }

    Document &operator[](int id) noexcept {
        if (!Ordered) {
            lastKey = Keys[Entries[static_cast<UNumber>(id)].KeyID];

            Entry *   _entry;
            Document *src = GetSource(&_entry, lastKey, 0, lastKey.Length);
            if (src != nullptr) {
                src->lastKey = lastKey;
                return *src;
            }
        } else {
            // I don't want to expand the size of Document by adding lastID , so this should do just find.
            lastKey = String::FromNumber(static_cast<UNumber>(id), 1, 0, 0);
        }

        return *this;
    }
};
} // namespace Qentem
#endif
