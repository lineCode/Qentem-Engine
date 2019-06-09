
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

struct Document {
    UNumber HashBase = 7; // Or 97. Choose prime numbers only!
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

    static void Drop(Entry &_entry, Document &storage) noexcept;
    void        Drop(const String &key, UNumber offset, UNumber limit) noexcept;

    static bool ExtractID(UNumber &id, const String &key, UNumber offset, UNumber limit) noexcept;

    Document *GetSource(Entry **_entry, const String &key, UNumber offset = 0, UNumber limit = 0) noexcept;
    bool      GetString(String &value, const String &key, UNumber offset = 0, UNumber limit = 0) noexcept;
    bool      GetNumber(UNumber &value, const String &key, UNumber offset = 0, UNumber limit = 0) noexcept;
    bool      GetDouble(double &value, const String &key, UNumber offset = 0, UNumber limit = 0) noexcept;
    bool      GetBool(bool &value, const String &key, UNumber offset = 0, UNumber limit = 0) noexcept;
    Document *GetDocument(const String &key, UNumber offset = 0, UNumber limit = 0) noexcept;

    // void InsertHash(const Hash &_hash) noexcept;
    Entry *Exist(const UNumber hash, const UNumber level, const Array<Index> &_table) const noexcept;
    void   InsertIndex(const Index &_index, const UNumber level, Array<Index> &_table) noexcept;
    void Insert(const String &key, UNumber offset, UNumber limit, const VType type, void *ptr, const bool move) noexcept;

    String ToJSON() const noexcept;

    static Expressions &GetJsonExpres() noexcept;

    static void     _makeListNumber(Document &document, const String &content, const Match &item) noexcept;
    static void     _makeDocument(Document &document, const String &content, Array<Match> &items) noexcept;
    static Document FromJSON(const String &content, bool comments = false) noexcept;

    void Clear() noexcept {
        Entries.Clear();
        Table.Clear();
        Numbers.Clear();
        Strings.Clear();
        Keys.Clear();
        Documents.Clear();
    }
};

Document Document::FromJSON(const String &content, bool comments) noexcept {
    Document     document;
    Array<Match> items;

    String n_content;

    static Expressions __comments;

    // C style comments
    if (comments) {
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
        items     = Engine::Search(n_content, GetJsonExpres());
    } else {
        items = Engine::Search(content, GetJsonExpres());
    }

    // return document; // TODO: remove

    if (items.Size != 0) {
        Match *_item = &(items.Storage[0]);

        if (_item->Expr->Keyword == L']') {
            document.Ordered = true;
        }

        if (n_content.Length == 0) {
            if (_item->NestMatch.Size != 0) {
                Document::_makeDocument(document, content, _item->NestMatch);
            } else if (document.Ordered) {
                Document::_makeListNumber(document, content, *_item);
            }
        } else {
            if (_item->NestMatch.Size != 0) {
                Document::_makeDocument(document, n_content, _item->NestMatch);
            } else if (document.Ordered) {
                Document::_makeListNumber(document, n_content, *_item);
            }
        }
    }

    return document;
}

void Document::Drop(Entry &_entry, Document &storage) noexcept {
    _entry.Type = VType::UndefinedT;
    storage.Keys.Storage[_entry.KeyID].Clear();

    switch (_entry.Type) {
        // case VType::NumberT:
        //     storage.Numbers[_hash.ID] = 0; // Waste of time
        //     break;
        case VType::StringT:
            storage.Strings.Storage[_entry.ID].Clear();
            break;
        case VType::DocumentT:
            storage.Documents.Storage[_entry.ID].Clear();
            break;
        default:
            break;
    }
}

void Document::Drop(const String &key, UNumber offset, UNumber limit) noexcept {
    Entry *   _entry;
    Document *storage = GetSource(&_entry, key, offset, limit);

    if (_entry != nullptr) {
        Document::Drop(*_entry, *storage);
    }
}

bool Document::ExtractID(UNumber &id, const String &key, UNumber offset, UNumber limit) noexcept {
    UNumber end = (offset + (limit - 1));

    while ((end > offset) && (key.Str[--end] != L'[')) {
    }
    ++end;
    --limit;

    return String::ToNumber(key, id, end, (limit - (end - offset)));
}

// Key form can be: name, name[id1], name[id1][sub-id2], name[id1][sub-id2][sub-sub-idx]...
Document *Document::GetSource(Entry **_entry, const String &key, UNumber offset, UNumber limit) noexcept {
    if (limit == 0) {
        limit = key.Length - offset;
    }

    UNumber end_offset = (offset + limit);
    UNumber _hash;

    if (key.Str[offset] == L'[') {
        ++offset;
        end_offset = offset;

        while (key.Str[end_offset] != L']') {
            ++end_offset;

            if (end_offset > (offset + limit)) {
                return nullptr;
            }
        }

        _hash = String::Hash(key, offset, end_offset);
        ++end_offset;
        offset--;
    } else if (key.Str[(end_offset - 1)] == L']') {
        end_offset = offset;

        while (key.Str[end_offset] != L'[') {
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
            Document *tmp = &(this->Documents.Storage[(*_entry)->ID]);

            if ((limit != 0) && !tmp->Ordered) {
                return tmp->GetSource(_entry, key, end_offset, limit);
            }

            return tmp;
        }

        return this;
    }

    return nullptr;
}

bool Document::GetString(String &value, const String &key, UNumber offset, UNumber limit) noexcept {
    value.Clear();

    Entry *         _entry;
    const Document *storage = GetSource(&_entry, key, offset, limit);

    if (storage != nullptr) {
        if (!storage->Ordered) {
            if (_entry->Type == VType::StringT) {
                value = storage->Strings.Storage[_entry->ID];
                return true;
            }

            if (_entry->Type == VType::BooleanT) {
                value = ((storage->Numbers.Storage[_entry->ID] == 1.0) ? L"true" : L"false");
                return true;
            }

            if (_entry->Type == VType::NumberT) {
                value = String::FromNumber(storage->Numbers.Storage[_entry->ID]);
                return true;
            }
        } else {
            UNumber id = 0;
            storage->ExtractID(id, key, offset, limit);

            if (storage->Strings.Size > id) {
                value = storage->Strings.Storage[id];
                return true;
            } else if (storage->Numbers.Size > id) {
                value = String::FromNumber(storage->Numbers.Storage[id]);
                return true;
            }
        }
    }

    return false;
}

bool Document::GetNumber(UNumber &value, const String &key, UNumber offset, UNumber limit) noexcept {
    value = 0;

    Entry *         _entry;
    const Document *storage = GetSource(&_entry, key, offset, limit);

    if (storage != nullptr) {
        if (!storage->Ordered) {
            if (_entry->Type == VType::StringT) {
                return String::ToNumber(storage->Strings.Storage[_entry->ID], value);
            }

            if (_entry->Type == VType::BooleanT) {
                value = ((storage->Numbers.Storage[_entry->ID] == 1.0) ? 1 : 0);
                return true;
            }

            if (_entry->Type == VType::NumberT) {
                value = static_cast<UNumber>(storage->Numbers.Storage[_entry->ID]);
                return true;
            }
        } else {
            UNumber id = 0;
            storage->ExtractID(id, key, offset, limit);

            if (storage->Numbers.Size > id) {
                value = static_cast<UNumber>(storage->Numbers.Storage[id]);
                return true;
            } else if (storage->Strings.Size > id) {
                return String::ToNumber(storage->Strings.Storage[id], value);
            }
        }
    }

    return false;
}

bool Document::GetDouble(double &value, const String &key, UNumber offset, UNumber limit) noexcept {
    value = 0.0;

    Entry *         _entry;
    const Document *storage = GetSource(&_entry, key, offset, limit);

    if (storage != nullptr) {
        if (!storage->Ordered) {
            if (_entry->Type == VType::StringT) {
                return String::ToNumber(storage->Strings.Storage[_entry->ID], value);
            }

            if (_entry->Type == VType::BooleanT) {
                value = ((storage->Numbers.Storage[_entry->ID] == 1.0) ? 1.0 : 0.0);
                return true;
            }

            if (_entry->Type == VType::NumberT) {
                value = storage->Numbers.Storage[_entry->ID];
                return true;
            }
        } else {
            UNumber id = 0;
            storage->ExtractID(id, key, offset, limit);

            if (storage->Numbers.Size > id) {
                value = storage->Numbers.Storage[id];
                return true;
            } else if (storage->Strings.Size > id) {
                return String::ToNumber(storage->Strings.Storage[id], value);
            }
        }
    }

    return false;
}

bool Document::GetBool(bool &value, const String &key, UNumber offset, UNumber limit) noexcept {
    Entry *         _entry;
    const Document *storage = GetSource(&_entry, key, offset, limit);

    if (storage != nullptr) {
        if (!storage->Ordered) {
            if ((_entry->Type == VType::BooleanT) || (_entry->Type == VType::NumberT)) {
                value = (storage->Numbers.Storage[_entry->ID] == 1);
                return true;
            }

            if (_entry->Type == VType::StringT) {
                value = (storage->Strings.Storage[_entry->ID] == L"true");
                return true;
            }
        } else {
            UNumber id = 0;
            storage->ExtractID(id, key, offset, limit);

            if (storage->Strings.Size > id) {
                value = (storage->Strings.Storage[id] == L"true");
                return true;
            } else if (storage->Numbers.Size > id) {
                value = (storage->Numbers.Storage[id] == 1);
                return true;
            }
        }
    }

    return false;
}

Document *Document::GetDocument(const String &key, UNumber offset, UNumber limit) noexcept {
    Entry *   _entry;
    Document *storage = GetSource(&_entry, key, offset, limit);

    if ((storage != nullptr) && (_entry->Type == VType::DocumentT)) {
        return storage;
    }

    return nullptr;
}

Entry *Document::Exist(const UNumber hash, const UNumber level, const Array<Index> &_table) const noexcept {
    UNumber id = ((hash + level) % HashBase);

    if ((_table.Size > id) && (_table.Storage[id].Hash != 0)) {
        if (_table.Storage[id].Hash == hash) {
            return &(Entries.Storage[_table.Storage[id].ID]);
        } else {
            return Exist(hash, (level + 3), _table.Storage[id].Table);
        }
    }

    return nullptr;
}

void Document::InsertIndex(const Index &_index, const UNumber level, Array<Index> &_table) noexcept {
    UNumber id = ((_index.Hash + level) % HashBase);

    if (_table.Size <= id) {
        UNumber s = id + 1;
        _table.ExpandTo(s);
        _table.Size = s;
    }

    if (_table.Storage[id].Hash == 0) {
        _table.Storage[id] = _index;
    } else {
        InsertIndex(_index, (level + 3), _table.Storage[id].Table);
    }
}

void Document::Insert(const String &key, UNumber offset, UNumber limit, const VType type, void *ptr,
                      const bool move) noexcept {
    UNumber id    = 0;
    UNumber _hash = String::Hash(key, offset, (offset + limit));
    Entry * _ent  = Exist(_hash, 0, Table);

    if ((_ent == nullptr) || (_ent->Type != type)) {
        switch (type) {
            case VType::StringT: {
                id = Strings.Size;

                if (Strings.Size == Strings.Capacity) {
                    Strings.ExpandTo((Strings.Size == 0 ? 1 : (Strings.Size * 2)));
                }

                if (move) {
                    Strings.Storage[Strings.Size] = static_cast<String &&>(*(static_cast<String *>(ptr)));
                } else {
                    Strings.Storage[Strings.Size] = *(static_cast<String *>(ptr));
                }

                ++Strings.Size;
            } break;
            case VType::NumberT:
            case VType::BooleanT: {
                id = Numbers.Size;
                Numbers.Add(*(static_cast<double *>(ptr)));
            } break;
            case VType::DocumentT: {
                id = Documents.Size;

                if (Documents.Size == Documents.Capacity) {
                    Documents.ExpandTo((Documents.Size == 0 ? 1 : (Documents.Size * 2)));
                }

                if (move) {
                    Documents.Storage[Documents.Size] = static_cast<Document &&>(*(static_cast<Document *>(ptr)));
                } else {
                    Documents.Storage[Documents.Size] = *(static_cast<Document *>(ptr));
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
                Numbers.Storage[_ent->ID] = *(static_cast<double *>(ptr));
            } break;
            case VType::StringT: {
                if (move) {
                    Strings.Storage[_ent->ID] = static_cast<String &&>(*(static_cast<String *>(ptr)));
                } else {
                    Strings.Storage[_ent->ID] = *(static_cast<String *>(ptr));
                }
            } break;
            case VType::DocumentT: {
                if (move) {
                    Documents.Storage[_ent->ID] = static_cast<Document &&>(*(static_cast<Document *>(ptr)));
                } else {
                    Documents.Storage[_ent->ID] = *(static_cast<Document *>(ptr));
                }
            } break;
            default:
                break;
        }
    }

    if (_ent != nullptr) {
        // Does exist
        if (_ent->Type != type) {
            // Clearing existing value;
            switch (type) {
                case VType::StringT: {
                    Strings.Storage[_ent->ID].Clear();
                } break;
                case VType::DocumentT: {
                    Documents.Storage[_ent->ID].Clear();
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

        if (Keys.Size == Keys.Capacity) {
            Keys.ExpandTo((Keys.Size == 0 ? 1 : (Keys.Size * 2)));
        }

        Keys.Storage[Keys.Size] = String::Part(key, offset, limit);
        ++Keys.Size;

        Entries.Add(_entry);

        InsertIndex(_index, 0, Table);
    }
}

void Document::_makeListNumber(Document &document, const String &content, const Match &item) noexcept {
    double  tn;
    UNumber end;
    UNumber start = (item.Offset + 1);
    UNumber to    = (item.Offset + item.Length);

    for (UNumber i = start; i < to; i++) {
        if ((content.Str[i] == L',') || (content.Str[i] == L']')) {
            end = i;
            String::SoftTrim(content, start, end);

            if (String::ToNumber(content, tn, start, ((end + 1) - start))) {
                document.Numbers.Add(tn);
            }

            start = end + 2;
        }
    }
}

void Document::_makeDocument(Document &document, const String &content, Array<Match> &items) noexcept {
    static Array<Expression *> JsonDeQuot;

    if (JsonDeQuot.Size == 0) {
        static Expression _JsonDeQuot;
        _JsonDeQuot.Keyword = L"\\\"";
        _JsonDeQuot.Replace = L"\"";
        JsonDeQuot.Add(&_JsonDeQuot);
    }

    if (!document.Ordered) {
        Match * key;
        UNumber start;
        UNumber j;
        bool    done;

        for (UNumber i = 0; i < items.Size; i++) {
            key   = &(items.Storage[i]);
            start = 0;
            done  = false;
            j     = (key->Offset + key->Length);

            for (; j < content.Length; j++) {
                // TODO: Check for comments.
                switch (content.Str[j]) {
                    case L'"': {
                        ++i;

                        Match *data = &(items.Storage[i]);
                        String ts;
                        if (data->Length != 0) {
                            ts = String::Part(content, (data->Offset + 1), (data->Length - 2));
                        }

                        if (data->NestMatch.Size != 0) { // TODO: Use local replace
                            ts = Engine::Parse(ts, Engine::Search(ts, JsonDeQuot));
                        }

                        document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::StringT, &ts, true);

                        done = true;
                    } break;
                    case L'{': {
                        ++i;

                        Document uno_document = Document();
                        Document::_makeDocument(uno_document, content, items.Storage[i].NestMatch);
                        document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::DocumentT, &uno_document,
                                        true);

                        done = true;
                    } break;
                    case L'[': {
                        ++i;

                        Document o_document = Document();
                        o_document.Ordered  = true;
                        Match *data         = &(items.Storage[i]);

                        if (items.Storage[i].NestMatch.Size > 0) {
                            Document::_makeDocument(o_document, content, data->NestMatch);
                        } else {
                            Document::_makeListNumber(o_document, content, *data);
                        }

                        document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::DocumentT, &o_document,
                                        true);

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

                        if (content.Str[start] == L't') {
                            // True
                            tn = 1;
                            document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::BooleanT, &tn, false);
                        } else if (content.Str[start] == L'f') {
                            // False
                            tn = 0;
                            document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::BooleanT, &tn, false);
                        } else if (content.Str[start] == L'n') {
                            // Nullcontent.Str[i]
                            document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::NullT, nullptr, false);
                        } else if (String::ToNumber(content, tn, start, ((end + 1) - start))) {
                            // Number
                            document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::NumberT, &tn, false);
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
            if (items.Storage[0].Expr->ID == 3) { // " Strings
                document.Strings.SetCapacity(items.Size);

                Match *data;
                for (UNumber i = 0; i < items.Size; i++) {
                    data = &(items.Storage[i]);
                    if (data->NestMatch.Size == 0) {
                        document.Strings.Storage[i] = String::Part(content, (data->Offset + 1), (data->Length - 2));
                        ++document.Strings.Size;
                    } else {
                        String rs                   = String::Part(content, (data->Offset + 1), (data->Length - 2));
                        document.Strings.Storage[i] = Engine::Parse(rs, Engine::Search(rs, JsonDeQuot));
                        ++document.Strings.Size;
                    }
                }
            } else if (items.Storage[0].Expr->ID == 1) { // } Unordered arrays
                document.Documents.SetCapacity(items.Size);

                for (UNumber i = 0; i < items.Size; i++) {
                    Document *te = &(document.Documents.Storage[document.Documents.Size]);
                    ++document.Documents.Size;

                    _makeDocument(*te, content, items.Storage[i].NestMatch);
                }
            } else if (items.Storage[0].Expr->ID == 2) { // ] Ordered arrays
                Match *data;
                document.Documents.SetCapacity(items.Size);

                for (UNumber i = 0; i < items.Size; i++) {
                    Document *te = &(document.Documents.Storage[document.Documents.Size]);
                    ++document.Documents.Size;
                    te->Ordered = true;

                    data = &(items.Storage[i]);
                    if (data->NestMatch.Size > 0) {
                        Document::_makeDocument(*te, content, data->NestMatch);
                    } else {
                        Document::_makeListNumber(*te, content, *data);
                    }
                }
            }
        } else {
            Document::_makeListNumber(document, content, items.Storage[0]);
        }
    }

    items.Clear();
}

String Document::ToJSON() const noexcept {
    static Array<Expression *> JsonQuot;

    if (JsonQuot.Size == 0) {
        static Expression _JsonQuot;
        _JsonQuot.Keyword = L"\"";
        _JsonQuot.Replace = L"\\\"";
        JsonQuot.Add(&_JsonQuot);
    }

    StringStream ss;

    if (!Ordered) {
        Entry *_ptr;

        ss += L'{';

        for (UNumber i = 0; i < Entries.Size; i++) {
            _ptr = &(Entries.Storage[i]);

            if (ss.Length != 1) {
                ss += L',';
            }

            switch (_ptr->Type) {
                case VType::NullT: {
                    ss += L'"';
                    ss += Keys.Storage[_ptr->KeyID];
                    ss += L"\":null"; // TODO: implement share(); to have a pointer to this inseat of copying it.
                } break;
                case VType::BooleanT: {
                    ss += L'"';
                    ss += Keys.Storage[_ptr->KeyID];

                    if (Numbers.Storage[_ptr->ID] != 0) {
                        ss += L"\":true";
                    } else {
                        ss += L"\":false";
                    }
                } break;
                case VType::StringT: {
                    ss += L'"';
                    ss += Keys.Storage[_ptr->KeyID];
                    ss += L"\":\"";
                    ss += Engine::Parse(Strings.Storage[_ptr->ID], Engine::Search(Strings.Storage[_ptr->ID], JsonQuot));
                    ss += L'"';
                } break;
                case VType::NumberT: {
                    ss += L'"';
                    ss += Keys.Storage[_ptr->KeyID];
                    ss += L"\":";
                    ss += String::FromNumber(Numbers.Storage[_ptr->ID]);
                } break;
                case VType::DocumentT: {
                    ss += L'"';
                    ss += Keys.Storage[_ptr->KeyID];
                    ss += L"\":";
                    ss += Documents.Storage[_ptr->ID].ToJSON();
                } break;
                default:
                    break;
            }
        }

        ss += L'}';
    } else {
        ss += L'[';

        if (Strings.Size != 0) {
            for (UNumber i = 0; i < Strings.Size; i++) {
                if (ss.Length != 1) {
                    ss += L',';
                }

                ss += L'"';
                ss += Engine::Parse(Strings.Storage[i], Engine::Search(Strings.Storage[i], JsonQuot));
                ss += L'"';
            }
        } else if (Numbers.Size != 0) {
            for (UNumber i = 0; i < Numbers.Size; i++) {
                if (ss.Length != 1) {
                    ss += L',';
                }

                ss += String::FromNumber(Numbers.Storage[i]);
            }
        } else if (Documents.Size != 0) {
            for (UNumber i = 0; i < Documents.Size; i++) {
                if (ss.Length != 1) {
                    ss += L',';
                }

                ss += Documents.Storage[i].ToJSON();
            }
        }

        ss += L']';
    }

    return ss.Eject();
}

Expressions &Document::GetJsonExpres() noexcept {
    static Expressions json_expres;

    if (json_expres.Size != 0) {
        return json_expres;
    }

    static Expression esc_quotation = Expression();
    esc_quotation.Keyword           = L"\\\"";

    static Expression quotation_start = Expression();
    static Expression quotation_end   = Expression();
    quotation_start.Keyword           = L'"';
    quotation_end.Keyword             = L'"';
    quotation_end.ID                  = 3; // For faster comparing.
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
    closed_curly_bracket.NestExprs.Add(&quotation_start).Add(&opened_square_bracket).Add(&opened_curly_bracket);

    opened_square_bracket.Keyword   = L'[';
    closed_square_bracket.Keyword   = L']';
    closed_square_bracket.ID        = 2;
    opened_square_bracket.Connected = &closed_square_bracket;
    closed_square_bracket.NestExprs.Add(&quotation_start).Add(&opened_square_bracket).Add(&opened_curly_bracket);

    json_expres.Add(&opened_curly_bracket).Add(&opened_square_bracket);

    return json_expres;
}

//////////// Fields' Operators
Field &Field::operator=(bool value) noexcept {
    if (this->Storage != nullptr) {
        double num = value ? 1.0 : 0.0;
        this->Storage->Insert(this->Key, 0, this->Key.Length, VType::BooleanT, &num, false);
    }
    return *this;
}

Field &Field::operator=(double value) noexcept {
    if (this->Storage != nullptr) {
        this->Storage->Insert(this->Key, 0, this->Key.Length, VType::NumberT, &value, false);
    }
    return *this;
}

Field &Field::operator=(const wchar_t *value) noexcept {
    if (this->Storage != nullptr) {
        if (value != nullptr) {
            String _s = value;
            this->Storage->Insert(this->Key, 0, this->Key.Length, VType::StringT, &_s, true);
        } else {
            this->Storage->Insert(this->Key, 0, this->Key.Length, VType::NullT, nullptr, false);
        }
    }
    return *this;
}

Field &Field::operator=(String &value) noexcept {
    if (this->Storage != nullptr) {
        this->Storage->Insert(this->Key, 0, this->Key.Length, VType::StringT, &value, false);
    }
    return *this;
}

Field &Field::operator=(Document &value) noexcept {
    if (this->Storage != nullptr) {
        this->Storage->Insert(this->Key, 0, this->Key.Length, VType::DocumentT, &value, false);
    }
    return *this;
}

Field &Field::operator=(Document &&value) noexcept {
    if (this->Storage != nullptr) {
        this->Storage->Insert(this->Key, 0, this->Key.Length, VType::DocumentT, &value, true);
    }
    return *this;
}

Field &Field::operator=(Array<double> &value) noexcept {
    if (this->Storage != nullptr) {
        Document tmp;
        tmp.Ordered = true;
        tmp.Numbers = value;
        this->Storage->Insert(this->Key, 0, this->Key.Length, VType::DocumentT, &tmp, true);
    }
    return *this;
}

Field &Field::operator=(Array<String> &value) noexcept {
    if (this->Storage != nullptr) {
        Document tmp;
        tmp.Ordered = true;
        tmp.Strings = value;
        this->Storage->Insert(this->Key, 0, this->Key.Length, VType::DocumentT, &tmp, true);
    }
    return *this;
}

Field &Field::operator=(Array<Document> &value) noexcept {
    if (this->Storage != nullptr) {
        Document tmp;
        tmp.Ordered   = true;
        tmp.Documents = value;
        this->Storage->Insert(this->Key, 0, this->Key.Length, VType::DocumentT, &tmp, true);
    }
    return *this;
}

Field Field::operator[](const String &key) noexcept {
    if (this->Storage != nullptr) {
        Document *document = this->Storage->GetDocument(this->Key, 0, this->Key.Length);

        if (document != nullptr) {
            return (*document)[key];
        }
    }

    return Field();
}

} // namespace Qentem
#endif
