
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

using Qentem::Engine::Expression;
using Qentem::Engine::Expressions;
using Qentem::Engine::Flags;
using Qentem::Engine::Match;

namespace Qentem {

enum VType { UndefinedT = 0, NullT, BooleanT, NumberT, StringT, DocumentT };

struct Index {
    UNumber      ID   = 0;
    UNumber      Hash = 0;
    Array<Index> Table;
};

struct Entry {
    UNumber ID    = 0;
    UNumber KeyID = 0;
    VType   Type  = VType::UndefinedT;

    Entry() = default;
    Entry(UNumber d, UNumber k, VType t) : ID(d), KeyID(k), Type(t) {
    }
};

struct _JsonFixedString {
    String const fss1  = L'{';
    String const fss2  = L'}';
    String const fss3  = L',';
    String const fss4  = L'[';
    String const fss5  = L']';
    String const fss6  = L'"';
    String const fss7  = L"\": null";
    String const fss8  = L"\": true";
    String const fss9  = L"\": false";
    String const fss10 = L"\": \"";
    String const fss11 = L"\": ";
} static JFX;

struct Document {
    UNumber HashBase = 17; // Or 97. Choose prime numbers only!
    bool    Ordered  = false;

    Array<Entry> Entries;
    Array<Index> Table;

    Array<double>   Numbers;
    Array<String>   Strings;
    Array<String>   Keys;
    Array<Document> Documents;

    String lastKey;
    Document()  = default;
    ~Document() = default;

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
        Ordered = true;
    }

    Document(Array<String> const &_strings) noexcept {
        Strings = _strings;
        Ordered = true;
    }

    Document(Array<Document> const &_documents) noexcept {
        Documents = _documents;
        Ordered   = true;
    }

    Document(Array<double> &&_numbers) noexcept {
        Numbers = static_cast<Array<double> &&>(_numbers);
        Ordered = true;
    }

    Document(Array<String> &&_strings) noexcept {
        Strings = static_cast<Array<String> &&>(_strings);
        Ordered = true;
    }

    Document(Array<Document> &&_documents) noexcept {
        Documents = static_cast<Array<Document> &&>(_documents);
        Ordered   = true;
    }

    Document(String const &json) noexcept {
        *this = FromJSON(json);
    }

    Document(wchar_t const *json) noexcept {
        *this = FromJSON(json);
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

        Insert(lastKey, 0, lastKey.Length, VType::DocumentT, &doc, true, true);
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

        Document in = doc;
        Entry *  _entry;
        Insert(lastKey, 0, lastKey.Length, VType::DocumentT, &in, true, true);
        Document *storage = GetSource(&_entry, lastKey, 0, lastKey.Length);
        lastKey.Reset();
        return *storage;
    }

    static void Drop(Entry &_entry, Document &storage) noexcept {
        _entry.Type = VType::UndefinedT;
        storage.Keys[_entry.KeyID].Reset();

        switch (_entry.Type) {
            // case VType::NumberT:
            //     storage.Numbers[_hash.ID] = 0; // Waste of time.
            //     break;
            case VType::StringT:
                storage.Strings[_entry.ID].Reset();
                break;
            case VType::DocumentT:
                storage.Documents[_entry.ID].Reset();
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

    Entry *Exist(UNumber hash, UNumber level, Array<Index> const &_table) const noexcept {
        UNumber id = ((hash + level) % HashBase);

        if ((_table.Index > id) && (_table[id].Hash != 0)) {
            if (_table[id].Hash == hash) {
                return &(Entries[_table[id].ID]);
            }

            return Exist(hash, (level + 2), _table[id].Table);
        }

        return nullptr;
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

        InsertIndex(_index, (level + 2), _table[id].Table);
    }

    void Rehash(UNumber newBase, bool const children = false) noexcept {
        Table.Reset();
        HashBase = newBase;

        Index   _index;
        String *key;

        for (UNumber i = 0; i < Entries.Index; i++) {
            key         = &(Keys[Entries[i].KeyID]);
            _index.Hash = String::Hash(key->Str, 0, key->Length);
            _index.ID   = i;

            InsertIndex(_index, 0, Table);

            if (children && (Entries[i].Type == VType::DocumentT)) {
                Documents[Entries[i].ID].Rehash(HashBase, true);
            }
        }
    }

    void _toJSON(StringStream &ss) const noexcept {
        static Expressions const &to_json_expres = _getToJsonExpres();

        Array<Match> tmpMatchs;

        if (!Ordered) {
            Entry *_entry;

            ss.Share(&JFX.fss1);

            for (UNumber i = 0; i < Entries.Index; i++) {
                _entry = &(Entries[i]);

                if (ss.Length != 1) {
                    ss.Share(&JFX.fss3);
                }

                switch (_entry->Type) {
                    case VType::NullT: {
                        ss.Share(&JFX.fss6);
                        ss += Keys[_entry->KeyID];
                        ss.Share(&JFX.fss7);
                    } break;
                    case VType::BooleanT: {
                        ss.Share(&JFX.fss6);
                        ss += Keys[_entry->KeyID];
                        ss.Share((Numbers[_entry->ID] != 0) ? &JFX.fss8 : &JFX.fss9);
                    } break;
                    case VType::StringT: {
                        ss.Share(&JFX.fss6);
                        ss += Keys[_entry->KeyID];
                        ss.Share(&JFX.fss10);

                        tmpMatchs = Engine::Search(Strings[_entry->ID], to_json_expres);
                        if (tmpMatchs.Index == 0) {
                            ss.Share(&Strings[_entry->ID]);
                        } else {
                            ss += Engine::Parse(Strings[_entry->ID], tmpMatchs);
                        }

                        ss.Share(&JFX.fss6);
                    } break;
                    case VType::NumberT: {
                        ss.Share(&JFX.fss6);
                        ss += Keys[_entry->KeyID];
                        ss.Share(&JFX.fss11);
                        ss += String::FromNumber(Numbers[_entry->ID]);
                    } break;
                    case VType::DocumentT: {
                        ss.Share(&JFX.fss6);
                        ss += Keys[_entry->KeyID];
                        ss.Share(&JFX.fss11);
                        ss += Documents[_entry->ID].ToJSON();
                    } break;
                    default:
                        break;
                }
            }

            ss.Share(&JFX.fss2);
            return;
        }

        ss.Share(&JFX.fss4);

        if (Strings.Index != 0) {
            for (UNumber i = 0; i < Strings.Index; i++) {
                if (ss.Length != 1) {
                    ss.Share(&JFX.fss3);
                }

                ss.Share(&JFX.fss6);

                tmpMatchs = Engine::Search(Strings[i], to_json_expres);
                if (tmpMatchs.Index == 0) {
                    ss.Share(&Strings[i]);
                } else {
                    ss += Engine::Parse(Strings[i], tmpMatchs);
                }

                ss.Share(&JFX.fss6);
            }
        } else if (Numbers.Index != 0) {
            for (UNumber i = 0; i < Numbers.Index; i++) {
                if (ss.Length != 1) {
                    ss.Share(&JFX.fss3);
                }

                ss += String::FromNumber(Numbers[i]);
            }
        } else if (Documents.Index != 0) {
            for (UNumber i = 0; i < Documents.Index; i++) {
                if (ss.Length != 1) {
                    ss.Share(&JFX.fss3);
                }

                ss += Documents[i].ToJSON();
            }
        }

        ss.Share(&JFX.fss5);
    }

    inline String ToJSON() const noexcept {
        StringStream ss;
        _toJSON(ss);
        return ss.Eject();
    }

    static void _makeListNumber(Array<double> &_numbers, String const &content, Match const &item) noexcept {
        double  tn;
        UNumber end;
        UNumber i;
        UNumber start = (item.Offset + 1);
        UNumber to    = (item.Offset + item.Length);

        for (i = start; i < to; i++) {
            if (content[i] == L',') {
                end = i;
                String::SoftTrim(content.Str, start, end);

                if (String::ToNumber(content, tn, start, ((end + 1) - start))) {
                    _numbers += tn;
                }

                start = end + 2;
                continue;
            }

            if (content[i] == L']') {
                end = i;
                String::SoftTrim(content.Str, start, end);

                if (String::ToNumber(content, tn, start, ((end + 1) - start))) {
                    _numbers += tn;
                }

                break;
            }
        }
    }

    void Insert(String const &key, UNumber offset, UNumber limit, VType const type, void *ptr, bool const move,
                bool const check = true) noexcept {
        UNumber       id    = 0;
        UNumber const _hash = String::Hash(key.Str, offset, (offset + limit));
        Entry *       _ent  = (!check) ? nullptr : Exist(_hash, 0, Table);

        if ((_ent == nullptr) || (_ent->Type != type)) {
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
            // Updating an existing item
            switch (type) {
                case VType::BooleanT:
                case VType::NumberT: {
                    Numbers[_ent->ID] = *(static_cast<double *>(ptr));
                } break;
                case VType::StringT: {
                    if (move) {
                        Strings[_ent->ID] = static_cast<String &&>(*(static_cast<String *>(ptr)));
                    } else {
                        Strings[_ent->ID] = *(static_cast<String *>(ptr));
                    }
                } break;
                case VType::DocumentT: {
                    if (move) {
                        Documents[_ent->ID] = static_cast<Document &&>(*(static_cast<Document *>(ptr)));
                    } else {
                        Documents[_ent->ID] = *(static_cast<Document *>(ptr));
                    }
                } break;
                default:
                    break;
            }
        }

        if (_ent != nullptr) {
            // If exists
            if (_ent->Type != type) {
                // Clearing existing value;
                switch (type) {
                    case VType::StringT: {
                        Strings[_ent->ID].Reset();
                    } break;
                    case VType::DocumentT: {
                        Documents[_ent->ID].Reset();
                    } break;
                    default:
                        break;
                }
                _ent->ID   = id;
                _ent->Type = type;
            }

            return;
        }

        Index _index;
        _index.Hash = _hash;
        _index.ID   = Entries.Index;
        Entries += Entry(id, Keys.Index, type);

        Keys += String::Part(key.Str, offset, limit);

        InsertIndex(_index, 0, Table);
    }

    static void _makeDocument(Document &document, String const &content, Array<Match> &items) noexcept {
        if (document.Ordered) {
            switch (items[0].Expr->ID) {
                case 3: {
                    document.Strings.SetCapacity(items.Index);

                    for (UNumber i = 0; i < items.Index; i++) {
                        if (items[i].NestMatch.Index == 0) {
                            document.Strings[i] =
                                String::Part(content.Str, (items[i].Offset + 1), (items[i].Length - 2));
                        } else {
                            document.Strings[i] = Engine::Parse(content, items[i].NestMatch, (items[i].Offset + 1),
                                                                ((items[i].Length + items[i].Offset) - 1));
                        }
                        ++document.Strings.Index;
                    }
                    break;
                }
                case 1: {
                    document.Documents.SetCapacity(items.Index);

                    for (UNumber i = 0; i < items.Index; i++) {
                        _makeDocument((document.Documents[document.Documents.Index++]), content, items[i].NestMatch);
                    }
                    break;
                }
                case 2: {
                    document.Documents.SetCapacity(items.Index);

                    for (UNumber i = 0; i < items.Index; i++) {
                        Document *te = &(document.Documents[document.Documents.Index]);
                        ++document.Documents.Index;
                        te->Ordered = true;

                        if (items[i].NestMatch.Index > 0) {
                            _makeDocument(*te, content, items[i].NestMatch);
                        } else {
                            _makeListNumber(te->Numbers, content, items[i]);
                        }
                    }
                    break;
                }
                default:
                    return;
            }

            return;
        }

        Match * key;
        Match * data;
        UNumber start;
        UNumber j;
        bool    done;

        for (UNumber i = 0; i < items.Index; i++) {
            key   = &(items[i]);
            start = 0;
            done  = false;
            j     = (key->Offset + key->Length);

            for (; j < content.Length; j++) {
                switch (content[j]) {
                    case L'"': {
                        ++i;

                        String ts;
                        data = &(items[i]);
                        if (data->NestMatch.Index != 0) {
                            ts = Engine::Parse(content, data->NestMatch, (data->Offset + 1),
                                               ((data->Offset + data->Length) - 1));
                        } else {
                            ts = String::Part(content.Str, (data->Offset + 1), (data->Length - 2));
                        }

                        document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::StringT, &ts, true, false);

                        done = true;
                    } break;
                    case L'{': {
                        ++i;

                        Document uno_document;
                        _makeDocument(uno_document, content, items[i].NestMatch);
                        document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::DocumentT, &uno_document,
                                        true, false);

                        done = true;
                    } break;
                    case L'[': {
                        ++i;

                        Document o_document;
                        o_document.Ordered = true;

                        if (items[i].NestMatch.Index > 0) {
                            _makeDocument(o_document, content, items[i].NestMatch);
                        } else {
                            _makeListNumber(o_document.Numbers, content, items[i]);
                        }

                        document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::DocumentT, &o_document,
                                        true, false);

                        done = true;
                    } break;
                    case L':': {
                        start = j + 1;
                        continue;
                    }
                    case L',':
                    case L'}':
                    case L']': {
                        // true, false, null or a number
                        double  tn;
                        UNumber end = j;
                        String::SoftTrim(content.Str, start, end);

                        switch (content[start]) {
                            case L't': {
                                // True
                                tn = 1;
                                document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::BooleanT, &tn,
                                                false, false);
                            } break;
                            case L'f': {
                                // False
                                tn = 0;
                                document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::BooleanT, &tn,
                                                false, false);
                            } break;
                            case L'n': {
                                // Null
                                document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::NullT, nullptr,
                                                false, false);
                            } break;
                            default: {
                                if (String::ToNumber(content, tn, start, ((end + 1) - start))) {
                                    // Number
                                    document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::NumberT, &tn,
                                                    false, false);
                                }
                            } break;
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

    void Reset() noexcept {
        Entries.Reset();
        Table.Reset();
        Numbers.Reset();
        Strings.Reset();
        Keys.Reset();
        Documents.Reset();
    }

    static Document FromJSON(String const &content, bool comments = false) noexcept {
        Document     document;
        Array<Match> items;

        String n_content;

        static Expressions const &json_expres = _getJsonExpres();

        // C style comments
        if (!comments) {
            items = Engine::Search(content, json_expres);
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

            n_content = Engine::Parse(content, Engine::Search(content, __comments));
            items     = Engine::Search(n_content, json_expres);
        }

        if (items.Index != 0) {
            Match *_item = &(items[0]);

            document.Ordered = (_item->Expr->Keyword.Str[0] == L']');

            if (n_content.Length == 0) {
                if (_item->NestMatch.Index != 0) {
                    _makeDocument(document, content, _item->NestMatch);
                } else if (document.Ordered) {
                    _makeListNumber(document.Numbers, content, *_item);
                }
            } else {
                if (_item->NestMatch.Index != 0) {
                    _makeDocument(document, n_content, _item->NestMatch);
                } else if (document.Ordered) {
                    _makeListNumber(document.Numbers, n_content, *_item);
                }
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

    // Key form can be: name, name[id1], name[id1][sub-id2], name[id1][sub-id2][sub-sub-idx]...
    Document *GetSource(Entry **_entry, String const &key, UNumber offset, UNumber limit) noexcept {
        if (limit == 0) {
            limit = key.Length - offset;
        }

        UNumber end_offset = (offset + limit);
        UNumber _hash;

        if (key[offset] == L'[') {
            ++offset;
            end_offset = offset;

            while (key[end_offset] != L']') {
                ++end_offset;

                if (end_offset > (offset + limit)) {
                    return nullptr;
                }
            }

            _hash = String::Hash(key.Str, offset, end_offset);
            ++end_offset;
            --offset;
        } else if (key[(end_offset - 1)] == L']') {
            end_offset = offset;

            while (key[end_offset] != L'[') {
                ++end_offset;

                if (end_offset > (offset + limit)) {
                    return nullptr;
                }
            }
            _hash = String::Hash(key.Str, offset, end_offset);
        } else {
            _hash = String::Hash(key.Str, offset, end_offset);
        }

        *_entry = Exist(_hash, 0, Table);
        if (*_entry != nullptr) {
            if ((*_entry)->Type == VType::DocumentT) {
                limit -= (end_offset - offset);
                Document *tmp = &(Documents[(*_entry)->ID]);

                if ((limit != 0) && !tmp->Ordered) {
                    return tmp->GetSource(_entry, key, end_offset, limit);
                }

                return tmp;
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
            if (!storage->Ordered) {
                if (_entry->Type == VType::StringT) {
                    value = storage->Strings[_entry->ID];
                    return true;
                }

                if (_entry->Type == VType::BooleanT) {
                    value = ((storage->Numbers[_entry->ID] == 1.0) ? L"true" : L"false");
                    return true;
                }

                if (_entry->Type == VType::NumberT) {
                    value = String::FromNumber(storage->Numbers[_entry->ID], 1, 0, 3);
                    return true;
                }
            } else {
                UNumber id = 0;
                Document::ExtractID(id, key, offset, limit);

                if (storage->Strings.Index > id) {
                    value = storage->Strings[id];
                    return true;
                }

                if (storage->Numbers.Index > id) {
                    value = String::FromNumber(storage->Numbers[id], 1, 0, 3);
                    return true;
                }
            }
        }

        return false;
    }

    bool GetNumber(UNumber &value, String const &key, UNumber offset = 0, UNumber limit = 0) noexcept {
        value = 0;

        Entry *         _entry;
        Document const *storage = GetSource(&_entry, key, offset, limit);

        if (storage != nullptr) {
            if (!storage->Ordered) {
                if (_entry->Type == VType::StringT) {
                    return String::ToNumber(storage->Strings[_entry->ID], value);
                }

                if (_entry->Type == VType::BooleanT) {
                    value = ((storage->Numbers[_entry->ID] == 1.0) ? 1 : 0);
                    return true;
                }

                if (_entry->Type == VType::NumberT) {
                    value = static_cast<UNumber>(storage->Numbers[_entry->ID]);
                    return true;
                }
            } else {
                UNumber id = 0;
                Document::ExtractID(id, key, offset, limit);

                if (storage->Numbers.Index > id) {
                    value = static_cast<UNumber>(storage->Numbers[id]);
                    return true;
                }

                if (storage->Strings.Index > id) {
                    return String::ToNumber(storage->Strings[id], value);
                }
            }
        }

        return false;
    }

    bool GetDouble(double &value, String const &key, UNumber offset = 0, UNumber limit = 0) noexcept {
        value = 0.0;

        Entry *         _entry;
        Document const *storage = GetSource(&_entry, key, offset, limit);

        if (storage != nullptr) {
            if (!storage->Ordered) {
                if (_entry->Type == VType::StringT) {
                    return String::ToNumber(storage->Strings[_entry->ID], value);
                }

                if (_entry->Type == VType::BooleanT) {
                    value = ((storage->Numbers[_entry->ID] == 1.0) ? 1.0 : 0.0);
                    return true;
                }

                if (_entry->Type == VType::NumberT) {
                    value = storage->Numbers[_entry->ID];
                    return true;
                }
            } else {
                UNumber id = 0;
                Document::ExtractID(id, key, offset, limit);

                if (storage->Numbers.Index > id) {
                    value = storage->Numbers[id];
                    return true;
                }

                if (storage->Strings.Index > id) {
                    return String::ToNumber(storage->Strings[id], value);
                }
            }
        }

        return false;
    }

    bool GetBool(bool &value, String const &key, UNumber offset = 0, UNumber limit = 0) noexcept {
        Entry *         _entry;
        Document const *storage = GetSource(&_entry, key, offset, limit);

        if (storage != nullptr) {
            if (!storage->Ordered) {
                if ((_entry->Type == VType::BooleanT) || (_entry->Type == VType::NumberT)) {
                    value = (storage->Numbers[_entry->ID] == 1);
                    return true;
                }

                if (_entry->Type == VType::StringT) {
                    value = (storage->Strings[_entry->ID] == L"true");
                    return true;
                }
            } else {
                UNumber id = 0;
                Document::ExtractID(id, key, offset, limit);

                if (storage->Strings.Index > id) {
                    value = (storage->Strings[id] == L"true");
                    return true;
                }

                if (storage->Numbers.Index > id) {
                    value = (storage->Numbers[id] == 1);
                    return true;
                }
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

    static Expressions const &_getToJsonExpres() noexcept {
        static Expression  _JsonEsc;
        static Expression  _JsonQuot;
        static Expressions tags;

        if (tags.Index == 0) {
            _JsonEsc.Keyword = L'\\';
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
            return;
        }

        if (!Ordered) {
            Entry * _entry;
            String *_key;

            for (UNumber i = 0; i < doc.Entries.Index; i++) {
                _entry = &(doc.Entries[i]);
                _key   = &(doc.Keys[doc.Entries[i].KeyID]);

                switch (_entry->Type) {
                    case VType::NullT: {
                        Insert(*_key, 0, _key->Length, _entry->Type, nullptr, false, true);
                    } break;
                    case VType::BooleanT: {
                        Insert(*_key, 0, _key->Length, _entry->Type, &doc.Numbers[_entry->ID], false, true);
                    } break;
                    case VType::StringT: {
                        Insert(*_key, 0, _key->Length, _entry->Type, &doc.Strings[_entry->ID], false, true);
                    } break;
                    case VType::NumberT: {
                        Insert(*_key, 0, _key->Length, _entry->Type, &doc.Numbers[_entry->ID], false, true);
                    } break;
                    case VType::DocumentT: {
                        Insert(*_key, 0, _key->Length, _entry->Type, &doc.Documents[_entry->ID], false, true);
                    } break;
                    default:
                        break;
                }
            }

            return;
        }

        if (doc.Numbers.Index != 0) {
            Numbers += doc.Numbers;
        } else if (doc.Strings.Index != 0) {
            Strings += doc.Strings;
        } else if (doc.Documents.Index != 0) {
            Documents += doc.Documents;
        }
    }

    void operator+=(Document &&doc) noexcept {
        if (Ordered != doc.Ordered) {
            return;
        }

        if (!Ordered) {
            Entry * _entry;
            String *_key;

            for (UNumber i = 0; i < doc.Entries.Index; i++) {
                _entry = &(doc.Entries[i]);
                _key   = &(doc.Keys[doc.Entries[i].KeyID]);

                switch (_entry->Type) {
                    case VType::NullT: {
                        Insert(*_key, 0, _key->Length, _entry->Type, nullptr, true, true);
                    } break;
                    case VType::BooleanT: {
                        Insert(*_key, 0, _key->Length, _entry->Type, &doc.Numbers[_entry->ID], true, true);
                    } break;
                    case VType::StringT: {
                        Insert(*_key, 0, _key->Length, _entry->Type, &doc.Strings[_entry->ID], true, true);
                    } break;
                    case VType::NumberT: {
                        Insert(*_key, 0, _key->Length, _entry->Type, &doc.Numbers[_entry->ID], true, true);
                    } break;
                    case VType::DocumentT: {
                        Insert(*_key, 0, _key->Length, _entry->Type, &doc.Documents[_entry->ID], true, true);
                    } break;
                    default:
                        break;
                }
            }

            return;
        }

        if (doc.Numbers.Index != 0) {
            Numbers += static_cast<Array<double> &&>(doc.Numbers);
        } else if (doc.Strings.Index != 0) {
            Strings += static_cast<Array<String> &&>(doc.Strings);
        } else if (doc.Documents.Index != 0) {
            Documents += static_cast<Array<Document> &&>(doc.Documents);
        }
    }

    Document &operator=(wchar_t const *value) noexcept {
        if (lastKey.Length == 0) {
            *this = Document::FromJSON(value);
            return *this;
        }

        if (value != nullptr) {
            String str = value;
            Insert(lastKey, 0, lastKey.Length, VType::StringT, &str, true, true);
        } else {
            Insert(lastKey, 0, lastKey.Length, VType::NullT, nullptr, false, true);
        }

        lastKey.Reset();
        return *this;
    }

    Document &operator=(String const &value) noexcept {
        if (lastKey.Length == 0) {
            *this = Document::FromJSON(value);
            return *this;
        }

        String str = value;
        Insert(lastKey, 0, lastKey.Length, VType::StringT, &str, true, true);
        lastKey.Reset();
        return *this;
    }

    Document &operator=(String &&value) noexcept {
        if (lastKey.Length == 0) {
            *this = Document::FromJSON(value);
            return *this;
        }

        Insert(lastKey, 0, lastKey.Length, VType::StringT, &value, true, true);
        lastKey.Reset();
        return *this;
    }

    Document &operator=(double value) noexcept {
        if (lastKey.Length != 0) {
            Insert(lastKey, 0, lastKey.Length, VType::NumberT, &value, false, true);
            lastKey.Reset();
        }
        return *this;
    }

    Document &operator=(int value) noexcept {
        if (lastKey.Length != 0) {
            double num = static_cast<double>(value);
            Insert(lastKey, 0, lastKey.Length, VType::NumberT, &num, false, true);
            lastKey.Reset();
        }
        return *this;
    }

    Document &operator=(long value) noexcept {
        if (lastKey.Length != 0) {
            double num = static_cast<double>(value);
            Insert(lastKey, 0, lastKey.Length, VType::NumberT, &num, false, true);
            lastKey.Reset();
        }
        return *this;
    }

    Document &operator=(bool value) noexcept {
        if (lastKey.Length != 0) {
            double num = value ? 1.0 : 0.0;
            Insert(lastKey, 0, lastKey.Length, VType::BooleanT, &num, false, true);
            lastKey.Reset();
        }
        return *this;
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

    Document &operator[](wchar_t const &key) noexcept {
        Entry *_entry;

        Document *src = GetSource(&_entry, key, 0, 1);
        if (src != nullptr) {
            src->lastKey = key;
            return *src;
        }

        lastKey = key;
        return *this;
    }

    Document &operator[](UNumber const &id) noexcept {
        lastKey = Keys[Entries[id].KeyID];
        Entry *_entry;

        Document *src = GetSource(&_entry, lastKey, 0, lastKey.Length);
        if (src != nullptr) {
            src->lastKey = lastKey;
            return *src;
        }

        return *this;
    }

    void operator+=(Array<double> const &_numbers) noexcept {
        Numbers += _numbers;
    }

    void operator+=(Array<String> const &_strings) noexcept {
        Strings += _strings;
    }

    void operator+=(Array<Document> const &_documents) noexcept {
        Documents += _documents;
    }

    void operator+=(Array<double> &&_numbers) noexcept {
        Numbers += static_cast<Array<double> &&>(_numbers);
    }

    void operator+=(Array<String> &&_strings) noexcept {
        Strings += static_cast<Array<String> &&>(_strings);
    }

    void operator+=(Array<Document> &&_documents) noexcept {
        Documents += static_cast<Array<Document> &&>(_documents);
    }
};
} // namespace Qentem
#endif
