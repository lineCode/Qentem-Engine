
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

#include "Engine.hpp"

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
};

struct JsonFixedString {
    String const fss1  = L'{';
    String const fss2  = L'}';
    String const fss3  = L',';
    String const fss4  = L'[';
    String const fss5  = L']';
    String const fss6  = L'"';
    String const fss7  = L"\":null";
    String const fss8  = L"\":true";
    String const fss9  = L"\":false";
    String const fss10 = L"\":\"";
    String const fss11 = L"\":";
};

struct Document;

struct Field {
    String    Key     = L"";
    Document *Storage = nullptr;

    Field &operator=(UNumber value) noexcept;
    Field &operator=(double value) noexcept;
    Field &operator=(wchar_t const *value) noexcept;
    Field &operator=(String &value) noexcept;
    Field &operator=(Document &value) noexcept;
    Field &operator=(Document &&value) noexcept;
    Field &operator=(Array<double> &value) noexcept;
    Field &operator=(Array<String> &value) noexcept;
    Field &operator=(Array<Document> &value) noexcept;
    Field &operator=(bool value) noexcept;

    // TODO: Add String(); to get string and Number to get a number

    Field operator[](String const &key) noexcept;
};

struct Document {
    UNumber HashBase = 17; // Or 97. Choose prime numbers only!
    bool    Ordered  = false;

    Array<Entry> Entries;
    Array<Index> Table;

    Array<double>   Numbers;
    Array<String>   Strings;
    Array<String>   Keys;
    Array<Document> Documents;

    static Expressions const json_expres;
    static Expressions const to_json_expres;

    static JsonFixedString const fxs;

    Document() = default;

    Field operator[](String const &key) noexcept {
        Field _field;
        _field.Key     = key;
        _field.Storage = this;
        return _field;
    }

    Field operator[](UNumber const &id) noexcept {
        Field _field;

        if (id < Keys.Size) {
            _field.Key = Keys[id];
        }

        _field.Storage = this;
        return _field;
    }

    static void Drop(Entry &_entry, Document &storage) noexcept {
        _entry.Type = VType::UndefinedT;
        storage.Keys[_entry.KeyID].Reset();

        switch (_entry.Type) {
            // case VType::NumberT:
            //     storage.Numbers[_hash.ID] = 0; // Waste of time
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

    Entry *Exist(UNumber const hash, UNumber const level, Array<Index> const &_table) const noexcept {
        UNumber id = ((hash + level) % HashBase);

        if ((_table.Size > id) && (_table[id].Hash != 0)) {
            if (_table[id].Hash == hash) {
                return &(Entries[_table[id].ID]);
            }

            return Exist(hash, (level + 3), _table[id].Table);
        }

        return nullptr;
    }

    void InsertIndex(Index const &_index, UNumber const level, Array<Index> &_table) noexcept {
        UNumber id = ((_index.Hash + level) % HashBase);

        if (_table.Size <= id) {
            _table.Capacity = id + 1;
            _table.Resize(_table.Capacity);
            _table.Size = _table.Capacity;
        }

        if (_table[id].Hash == 0) {
            _table[id] = _index;
            return;
        }

        InsertIndex(_index, (level + 3), _table[id].Table);
    }

    void _toJSON(StringStream &ss) const noexcept {
        Array<Match> tmpMatchs;

        if (!Ordered) {
            Entry *_ptr;

            ss.Share(&fxs.fss1);

            for (UNumber i = 0; i < Entries.Size; i++) {
                _ptr = &(Entries[i]);

                if (ss.Length != 1) {
                    ss.Share(&fxs.fss3);
                }

                switch (_ptr->Type) {
                    case VType::NullT: {
                        ss.Share(&fxs.fss6);
                        ss += Keys[_ptr->KeyID];
                        ss.Share(&fxs.fss7);
                    } break;
                    case VType::BooleanT: {
                        ss.Share(&fxs.fss6);
                        ss += Keys[_ptr->KeyID];
                        ss.Share((Numbers[_ptr->ID] != 0) ? &fxs.fss8 : &fxs.fss9);
                    } break;
                    case VType::StringT: {
                        ss.Share(&fxs.fss6);
                        ss += Keys[_ptr->KeyID];
                        ss.Share(&fxs.fss10);

                        tmpMatchs = Engine::Search(Strings[_ptr->ID], to_json_expres);
                        if (tmpMatchs.Size == 0) {
                            ss.Share(&Strings[_ptr->ID]);
                        } else {
                            ss += Engine::Parse(Strings[_ptr->ID], tmpMatchs);
                        }

                        ss.Share(&fxs.fss6);
                    } break;
                    case VType::NumberT: {
                        ss.Share(&fxs.fss6);
                        ss += Keys[_ptr->KeyID];
                        ss.Share(&fxs.fss11);
                        ss += String::FromNumber(Numbers[_ptr->ID]);
                    } break;
                    case VType::DocumentT: {
                        ss.Share(&fxs.fss6);
                        ss += Keys[_ptr->KeyID];
                        ss.Share(&fxs.fss11);
                        ss += Documents[_ptr->ID].ToJSON();
                    } break;
                    default:
                        break;
                }
            }

            ss.Share(&fxs.fss2);
            return;
        }

        ss.Share(&fxs.fss4);

        if (Strings.Size != 0) {
            for (UNumber i = 0; i < Strings.Size; i++) {
                if (ss.Length != 1) {
                    ss.Share(&fxs.fss3);
                }

                ss.Share(&fxs.fss6);

                tmpMatchs = Engine::Search(Strings[i], to_json_expres);
                if (tmpMatchs.Size == 0) {
                    ss.Share(&Strings[i]);
                } else {
                    ss += Engine::Parse(Strings[i], tmpMatchs);
                }

                ss.Share(&fxs.fss6);
            }
        } else if (Numbers.Size != 0) {
            for (UNumber i = 0; i < Numbers.Size; i++) {
                if (ss.Length != 1) {
                    ss.Share(&fxs.fss3);
                }

                ss += String::FromNumber(Numbers[i]);
            }
        } else if (Documents.Size != 0) {
            for (UNumber i = 0; i < Documents.Size; i++) {
                if (ss.Length != 1) {
                    ss.Share(&fxs.fss3);
                }

                ss += Documents[i].ToJSON();
            }
        }

        ss.Share(&fxs.fss5);
    }

    static Expressions _getToJsonExpres() noexcept {
        static Expression _JsonQuot;
        _JsonQuot.Keyword = L'"';
        _JsonQuot.Replace = L"\\\"";

        static Expression *jq[1];
        jq[0] = &_JsonQuot;

        static Expressions qs;
        qs.Share(&jq[0], 1);

        return qs;
    }

    static Expressions _getJsonExpres() noexcept {
        static Expression esc_quotation = Expression();
        esc_quotation.Keyword           = L"\\\"";
        esc_quotation.Replace           = L'"';

        static Expression quotation_start = Expression();
        static Expression quotation_end   = Expression();
        quotation_start.Keyword           = L'"';
        quotation_end.Keyword             = L'"';
        quotation_end.ID                  = 3;
        quotation_start.Connected         = &quotation_end;
        quotation_end.NestExprs.Add(&esc_quotation);

        static Expression opened_square_bracket = Expression();
        static Expression closed_square_bracket = Expression();

        static Expression opened_curly_bracket = Expression();
        static Expression closed_curly_bracket = Expression();

        opened_curly_bracket.Keyword   = L'{';
        closed_curly_bracket.Keyword   = L'}';
        closed_curly_bracket.ID        = 1;
        opened_curly_bracket.Connected = &closed_curly_bracket;

        // Memory locality
        static Expression *_curly[3];
        _curly[0] = &quotation_start;
        _curly[1] = &opened_curly_bracket;
        _curly[2] = &opened_square_bracket;

        closed_curly_bracket.NestExprs.Share(&_curly[0], 3);

        opened_square_bracket.Keyword   = L'[';
        closed_square_bracket.Keyword   = L']';
        closed_square_bracket.ID        = 2;
        opened_square_bracket.Connected = &closed_square_bracket;

        static Expression *_bracket[3];
        _bracket[0] = &quotation_start;
        _bracket[1] = &opened_square_bracket;
        _bracket[2] = &opened_curly_bracket;

        closed_square_bracket.NestExprs.Share(&_bracket[0], 3);

        static Expression *_all[2];
        _all[0] = &opened_curly_bracket;
        _all[1] = &opened_square_bracket;

        static Expressions js;
        js.Share(&_all[0], 2);

        return js;
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
                String::SoftTrim(content, start, end);

                if (String::ToNumber(content, tn, start, ((end + 1) - start))) {
                    _numbers.Add(tn);
                }

                start = end + 2;
                continue;
            }

            if (content[i] == L']') {
                end = i;
                String::SoftTrim(content, start, end);

                if (String::ToNumber(content, tn, start, ((end + 1) - start))) {
                    _numbers.Add(tn);
                }

                break;
            }
        }
    }

    void Insert(String const &key, UNumber offset, UNumber limit, VType const type, void *ptr, bool const move,
                bool const check = true) noexcept {
        UNumber       id    = 0;
        UNumber const _hash = String::Hash(key, offset, (offset + limit));
        Entry *       _ent  = (!check) ? nullptr : Exist(_hash, 0, Table);

        if ((_ent == nullptr) || (_ent->Type != type)) {
            switch (type) {
                case VType::StringT: {
                    id = Strings.Size;

                    if (Strings.Size == Strings.Capacity) {
                        Strings.Resize((Strings.Size + 1) * 4);
                    }

                    if (move) {
                        Strings[Strings.Size] = static_cast<String &&>(*(static_cast<String *>(ptr)));
                    } else {
                        Strings[Strings.Size] = *(static_cast<String *>(ptr));
                    }

                    ++Strings.Size;
                } break;
                case VType::NumberT:
                case VType::BooleanT: {
                    id = Numbers.Size;
                    if (Numbers.Size == Numbers.Capacity) {
                        Numbers.Resize((Numbers.Size + 1) * 4);
                    }

                    Numbers[Numbers.Size] = *(static_cast<double *>(ptr));
                    ++Numbers.Size;
                } break;
                case VType::DocumentT: {
                    id = Documents.Size;

                    if (Documents.Size == Documents.Capacity) {
                        Documents.Resize((Documents.Size + 1) * 4);
                    }

                    if (move) {
                        Documents[Documents.Size] = static_cast<Document &&>(*(static_cast<Document *>(ptr)));
                    } else {
                        Documents[Documents.Size] = *(static_cast<Document *>(ptr));
                    }

                    ++Documents.Size;
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
        } else {
            Index _index;
            _index.Hash = _hash;
            _index.ID   = Entries.Size;

            Entry _entry;
            _entry.Type  = type;
            _entry.ID    = id;
            _entry.KeyID = Keys.Size;

            Keys.Add(String::Part(key, offset, limit));
            Entries.Add(_entry);

            InsertIndex(_index, 0, Table);
        }
    }

    static void _makeDocument(Document &document, String const &content, Array<Match> &items) noexcept {
        if (!document.Ordered) {
            Match *  key;
            Match *  data;
            UNumber  start;
            UNumber  j;
            bool     done;
            String   ts;
            Document o_document;
            Document uno_document;

            for (UNumber i = 0; i < items.Size; i++) {
                key   = &(items[i]);
                start = 0;
                done  = false;
                j     = (key->Offset + key->Length);

                for (; j < content.Length; j++) {
                    switch (content[j]) {
                        case L'"': {
                            ++i;

                            data = &(items[i]);
                            if (data->NestMatch.Size != 0) {
                                ts = Engine::Parse(content, data->NestMatch, (data->Offset + 1),
                                                   ((data->Offset + data->Length) - 1));
                            } else {
                                ts = String::Part(content, (data->Offset + 1), (data->Length - 2));
                            }

                            document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::StringT, &ts, true,
                                            false);

                            done = true;
                        } break;
                        case L'{': {
                            ++i;

                            uno_document = Document();
                            _makeDocument(uno_document, content, items[i].NestMatch);
                            document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::DocumentT,
                                            &uno_document, true, false);

                            done = true;
                        } break;
                        case L'[': {
                            ++i;

                            o_document         = Document();
                            o_document.Ordered = true;

                            if (items[i].NestMatch.Size > 0) {
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
                            String::SoftTrim(content, start, end);

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
                                        document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::NumberT,
                                                        &tn, false, false);
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
        } else {
            if (items.Size > 0) {
                if (items[0].Expr->ID == 3) { // " Strings
                    document.Strings.SetCapacity(items.Size);

                    for (UNumber i = 0; i < items.Size; i++) {
                        if (items[i].NestMatch.Size == 0) {
                            document.Strings[i] = String::Part(content, (items[i].Offset + 1), (items[i].Length - 2));
                        } else {
                            document.Strings[i] = Engine::Parse(content, items[i].NestMatch, (items[i].Offset + 1),
                                                                ((items[i].Length + items[i].Offset) - 1));
                        }
                        ++document.Strings.Size;
                    }
                } else if (items[0].Expr->ID == 1) { // } Unordered arrays
                    document.Documents.SetCapacity(items.Size);

                    for (UNumber i = 0; i < items.Size; i++) {
                        _makeDocument((document.Documents[document.Documents.Size++]), content, items[i].NestMatch);
                    }
                } else if (items[0].Expr->ID == 2) { // ] Ordered arrays
                    document.Documents.SetCapacity(items.Size);

                    for (UNumber i = 0; i < items.Size; i++) {
                        Document *te = &(document.Documents[document.Documents.Size]);
                        ++document.Documents.Size;
                        te->Ordered = true;

                        if (items[i].NestMatch.Size > 0) {
                            _makeDocument(*te, content, items[i].NestMatch);
                        } else {
                            _makeListNumber(te->Numbers, content, items[i]);
                        }
                    }
                }
            } else {
                _makeListNumber(document.Numbers, content, items[0]);
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

        static Expressions __comments;

        // C style comments
        if (!comments) {
            items = Engine::Search(content, json_expres);
        } else {
            if (__comments.Size == 0) {
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

                __comments.Add(&comment1);
                __comments.Add(&comment2);
            }

            n_content = Engine::Parse(content, Engine::Search(content, __comments));
            items     = Engine::Search(n_content, json_expres);
        }

        if (items.Size != 0) {
            Match *_item = &(items[0]);

            document.Ordered = (_item->Expr->Keyword == L']');

            if (n_content.Length == 0) {
                if (_item->NestMatch.Size != 0) {
                    _makeDocument(document, content, _item->NestMatch);
                } else if (document.Ordered) {
                    _makeListNumber(document.Numbers, content, *_item);
                }
            } else {
                if (_item->NestMatch.Size != 0) {
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

            _hash = String::Hash(key, offset, end_offset);
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
            _hash = String::Hash(key, offset, end_offset);
        } else {
            _hash = String::Hash(key, offset, end_offset);
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

                if (storage->Strings.Size > id) {
                    value = storage->Strings[id];
                    return true;
                }

                if (storage->Numbers.Size > id) {
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

                if (storage->Numbers.Size > id) {
                    value = static_cast<UNumber>(storage->Numbers[id]);
                    return true;
                }

                if (storage->Strings.Size > id) {
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

                if (storage->Numbers.Size > id) {
                    value = storage->Numbers[id];
                    return true;
                }

                if (storage->Strings.Size > id) {
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

                if (storage->Strings.Size > id) {
                    value = (storage->Strings[id] == L"true");
                    return true;
                }

                if (storage->Numbers.Size > id) {
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
};

Expressions const     Document::json_expres    = Document::_getJsonExpres();
Expressions const     Document::to_json_expres = Document::_getToJsonExpres();
JsonFixedString const Document::fxs            = JsonFixedString();
//////////// Fields' Operators
Field &Field::operator=(UNumber value) noexcept {
    if (Storage != nullptr) {
        Storage->Insert(Key, 0, Key.Length, VType::NumberT, &value, false);
    }
    return *this;
}

Field &Field::operator=(double value) noexcept {
    if (Storage != nullptr) {
        Storage->Insert(Key, 0, Key.Length, VType::NumberT, &value, false);
    }
    return *this;
}

Field &Field::operator=(wchar_t const *value) noexcept {
    if (Storage != nullptr) {
        if (value != nullptr) {
            String _s = value;
            Storage->Insert(Key, 0, Key.Length, VType::StringT, &_s, true);
        } else {
            Storage->Insert(Key, 0, Key.Length, VType::NullT, nullptr, false);
        }
    }
    return *this;
}

Field &Field::operator=(String &value) noexcept {
    if (Storage != nullptr) {
        Storage->Insert(Key, 0, Key.Length, VType::StringT, &value, false);
    }
    return *this;
}

Field &Field::operator=(Document &value) noexcept {
    if (Storage != nullptr) {
        Storage->Insert(Key, 0, Key.Length, VType::DocumentT, &value, false);
    }
    return *this;
}

Field &Field::operator=(Document &&value) noexcept {
    if (Storage != nullptr) {
        Storage->Insert(Key, 0, Key.Length, VType::DocumentT, &value, true);
    }
    return *this;
}

Field &Field::operator=(Array<double> &value) noexcept {
    if (Storage != nullptr) {
        Document tmp;
        tmp.Ordered = true;
        tmp.Numbers = value;
        Storage->Insert(Key, 0, Key.Length, VType::DocumentT, &tmp, true);
    }
    return *this;
}

Field &Field::operator=(Array<String> &value) noexcept {
    if (Storage != nullptr) {
        Document tmp;
        tmp.Ordered = true;
        tmp.Strings = value;
        Storage->Insert(Key, 0, Key.Length, VType::DocumentT, &tmp, true);
    }
    return *this;
}

Field &Field::operator=(Array<Document> &value) noexcept {
    if (Storage != nullptr) {
        Document tmp;
        tmp.Ordered   = true;
        tmp.Documents = value;
        Storage->Insert(Key, 0, Key.Length, VType::DocumentT, &tmp, true);
    }
    return *this;
}

Field &Field::operator=(bool value) noexcept {
    if (Storage != nullptr) {
        double num = value ? 1.0 : 0.0;
        Storage->Insert(Key, 0, Key.Length, VType::BooleanT, &num, false);
    }
    return *this;
}

Field Field::operator[](String const &key) noexcept {
    if (Storage != nullptr) {
        Document *document = Storage->GetDocument(Key, 0, Key.Length);

        if (document != nullptr) {
            return (*document)[key];
        }
    }

    return Field();
}

} // namespace Qentem
#endif
