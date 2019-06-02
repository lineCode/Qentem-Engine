
/**
 * Qentem Document
 *
 * @brief     Ordered/Unordered array with hasing capability and JSON build-in
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Extintion/Document.hpp"

using Qentem::Array;
using Qentem::String;
using Qentem::StringStream;
using Qentem::Engine::Expression;
using Qentem::Engine::Expressions;
using Qentem::Engine::Flags;

//////////// Fields' Operators
Qentem::Field &Qentem::Field::operator=(bool value) noexcept {
    if (this->Storage != nullptr) {
        double num = value ? 1.0 : 0.0;
        this->Storage->Insert(this->Key, 0, this->Key.Length, VType::BooleanT, &num, false);
    }
    return *this;
}

Qentem::Field &Qentem::Field::operator=(double value) noexcept {
    if (this->Storage != nullptr) {
        this->Storage->Insert(this->Key, 0, this->Key.Length, VType::NumberT, &value, false);
    }
    return *this;
}

Qentem::Field &Qentem::Field::operator=(const wchar_t *value) noexcept {
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

Qentem::Field &Qentem::Field::operator=(String &value) noexcept {
    if (this->Storage != nullptr) {
        this->Storage->Insert(this->Key, 0, this->Key.Length, VType::StringT, &value, false);
    }
    return *this;
}

Qentem::Field &Qentem::Field::operator=(Document &value) noexcept {
    if (this->Storage != nullptr) {
        this->Storage->Insert(this->Key, 0, this->Key.Length, VType::DocumentT, &value, false);
    }
    return *this;
}

Qentem::Field &Qentem::Field::operator=(Document &&value) noexcept {
    if (this->Storage != nullptr) {
        this->Storage->Insert(this->Key, 0, this->Key.Length, VType::DocumentT, &value, true);
    }
    return *this;
}

Qentem::Field &Qentem::Field::operator=(Array<double> &value) noexcept {
    if (this->Storage != nullptr) {
        Document tmp;
        tmp.Ordered = true;
        tmp.Numbers = value;
        this->Storage->Insert(this->Key, 0, this->Key.Length, VType::DocumentT, &tmp, true);
    }
    return *this;
}

Qentem::Field &Qentem::Field::operator=(Array<String> &value) noexcept {
    if (this->Storage != nullptr) {
        Document tmp;
        tmp.Ordered = true;
        tmp.Strings = value;
        this->Storage->Insert(this->Key, 0, this->Key.Length, VType::DocumentT, &tmp, true);
    }
    return *this;
}

Qentem::Field &Qentem::Field::operator=(Array<Document> &value) noexcept {
    if (this->Storage != nullptr) {
        Document tmp;
        tmp.Ordered   = true;
        tmp.Documents = value;
        this->Storage->Insert(this->Key, 0, this->Key.Length, VType::DocumentT, &tmp, true);
    }
    return *this;
}

Qentem::Field Qentem::Field::operator[](const String &key) noexcept {
    if (this->Storage != nullptr) {
        Document *document = this->Storage->GetDocument(this->Key, 0, this->Key.Length);

        if (document != nullptr) {
            return (*document)[key];
        }
    }

    return Field();
}
////////////////

void Qentem::Document::Drop(Entry &_entry, Document &storage) noexcept {
    _entry.Type = VType::UndefinedT;
    storage.Keys[_entry.KeyID].Clear();

    switch (_entry.Type) {
        // case VType::NumberT:
        //     storage.Numbers[_hash.ID] = 0; // Waste of time
        //     break;
        case VType::StringT:
            storage.Strings[_entry.ID].Clear();
            break;
        case VType::DocumentT:
            storage.Documents[_entry.ID] = Document(); // TODO: Add Clear()
            break;
        default:
            break;
    }
}

void Qentem::Document::Drop(const String &key, UNumber offset, UNumber limit) noexcept {
    Entry *   _entry;
    Document *storage = GetSource(&_entry, key, offset, limit);

    if (_entry != nullptr) {
        Document::Drop(*_entry, *storage);
    }
}

bool Qentem::Document::ExtractID(UNumber &id, const String &key, UNumber offset, UNumber limit) noexcept {
    UNumber end = (offset + (limit - 1));

    while ((end > offset) && (key.Str[--end] != L'[')) {
    }
    end++;
    limit--;

    return String::ToNumber(key, id, end, (limit - (end - offset)));
}

// Key form can be: name, name[id1], name[id1][sub-id2], name[id1][sub-id2][sub-sub-idx]...
Qentem::Document *Qentem::Document::GetSource(Entry **_entry, const String &key, UNumber offset,
                                              UNumber limit) noexcept {
    if (limit == 0) {
        limit = key.Length - offset;
    }

    UNumber end_offset = (offset + limit);
    UNumber _hash;

    if (key.Str[offset] == L'[') {
        offset++;
        end_offset = offset;

        while (key.Str[end_offset] != L']') {
            end_offset++;

            if (end_offset > (offset + limit)) {
                return nullptr;
            }
        }

        _hash = String::Hash(key, offset, end_offset);
        end_offset++;
        offset--;
    } else if (key.Str[(end_offset - 1)] == L']') {
        end_offset = offset;

        while (key.Str[end_offset] != L'[') {
            end_offset++;

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
            Document *tmp = &(this->Documents[(*_entry)->ID]);

            if ((limit != 0) && !tmp->Ordered) {
                return tmp->GetSource(_entry, key, end_offset, limit);
            }

            return tmp;
        }

        return this;
    }

    return nullptr;
}

bool Qentem::Document::GetString(String &value, const String &key, UNumber offset, UNumber limit) noexcept {
    value.Clear();

    Entry *         _entry;
    const Document *storage = GetSource(&_entry, key, offset, limit);

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
                value = String::FromNumber(storage->Numbers[_entry->ID]);
                return true;
            }
        } else {
            UNumber id = 0;
            storage->ExtractID(id, key, offset, limit);

            if (storage->Strings.Size > id) {
                value = storage->Strings[id];
                return true;
            } else if (storage->Numbers.Size > id) {
                value = String::FromNumber(storage->Numbers[id]);
                return true;
            }
        }
    }

    return false;
}

bool Qentem::Document::GetNumber(UNumber &value, const String &key, UNumber offset, UNumber limit) noexcept {
    value = 0;

    Entry *         _entry;
    const Document *storage = GetSource(&_entry, key, offset, limit);

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
            storage->ExtractID(id, key, offset, limit);

            if (storage->Numbers.Size > id) {
                value = static_cast<UNumber>(storage->Numbers[id]);
                return true;
            } else if (storage->Strings.Size > id) {
                return String::ToNumber(storage->Strings[id], value);
            }
        }
    }

    return false;
}

bool Qentem::Document::GetDouble(double &value, const String &key, UNumber offset, UNumber limit) noexcept {
    value = 0.0;

    Entry *         _entry;
    const Document *storage = GetSource(&_entry, key, offset, limit);

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
            storage->ExtractID(id, key, offset, limit);

            if (storage->Numbers.Size > id) {
                value = storage->Numbers[id];
                return true;
            } else if (storage->Strings.Size > id) {
                return String::ToNumber(storage->Strings[id], value);
            }
        }
    }

    return false;
}

bool Qentem::Document::GetBool(bool &value, const String &key, UNumber offset, UNumber limit) noexcept {
    Entry *         _entry;
    const Document *storage = GetSource(&_entry, key, offset, limit);

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
            storage->ExtractID(id, key, offset, limit);

            if (storage->Strings.Size > id) {
                value = (storage->Strings[id] == L"true");
                return true;
            } else if (storage->Numbers.Size > id) {
                value = (storage->Numbers[id] == 1);
                return true;
            }
        }
    }

    return false;
}

Qentem::Document *Qentem::Document::GetDocument(const String &key, UNumber offset, UNumber limit) noexcept {
    Entry *   _entry;
    Document *storage = GetSource(&_entry, key, offset, limit);

    if ((storage != nullptr) && (_entry->Type == VType::DocumentT)) {
        return storage;
    }

    return nullptr;
}

Qentem::Entry *Qentem::Document::Exist(const UNumber hash, const UNumber level, const Array<Qentem::Index> &_table) const
    noexcept {
    UNumber id = ((hash + level) % HashBase);

    if ((_table.Size > id) && (_table[id].Hash != 0)) {
        if (_table[id].Hash == hash) {
            return &(Entries[_table[id].ID]);
        } else {
            return Exist(hash, (level + 3), _table[id].Table);
        }
    }

    return nullptr;
}

void Qentem::Document::InsertIndex(const Qentem::Index _index, const UNumber level,
                                   Array<Qentem::Index> &_table) noexcept {
    UNumber id = ((_index.Hash + level) % HashBase);

    if (!(_table.Size > id)) {
        _table.ExpandTo((id + 1));
        _table.Size = id;
    }

    if (_table[id].Hash == 0) {
        _table[id] = _index;
    } else {
        InsertIndex(_index, (level + 3), _table[id].Table);
    }
}

void Qentem::Document::Insert(const String &key, UNumber offset, UNumber limit, const VType type, void *ptr,
                              const bool move) noexcept {

    UNumber id    = 0;
    UNumber _hash = String::Hash(key, offset, (offset + limit));
    Entry * _ent  = Exist(_hash, 0, Table);

    if ((_ent == nullptr) || (_ent->Type != type)) {
        switch (type) {
            case VType::StringT: {
                id = Strings.Size;
                if (move) {
                    Strings.Add(static_cast<String &&>(*(static_cast<String *>(ptr))));
                } else {
                    Strings.Add(*(static_cast<String *>(ptr)));
                }
            } break;
            case VType::NumberT:
            case VType::BooleanT: {
                id = Numbers.Size;
                Numbers.Add(*(static_cast<double *>(ptr)));
            } break;
            case VType::DocumentT: {
                id = Documents.Size;
                if (move) {
                    Documents.Add(static_cast<Document &&>(*(static_cast<Document *>(ptr))));
                } else {
                    Documents.Add(*(static_cast<Document *>(ptr)));
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
        // Does exist
        if (_ent->Type != type) {
            // Clearing existing value;
            switch (type) {
                case VType::StringT: {
                    Strings[_ent->ID].Clear();
                } break;
                case VType::DocumentT: {
                    Documents[_ent->ID] = Document();
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

void Qentem::Document::_makeNumberedDocument(Document &document, const String &content, const Match &item) noexcept {
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

void Qentem::Document::_makeDocument(Document &document, const String &content, const Array<Match> &items) noexcept {
    if (!document.Ordered) {
        Match * key;
        UNumber start;
        UNumber j;
        bool    done;

        for (UNumber i = 0; i < items.Size; i++) {
            key   = &(items[i]);
            start = 0;
            done  = false;

            for (j = (key->Offset + key->Length); j < content.Length; j++) {
                switch (content.Str[j]) {
                    case L'"': {
                        i++;

                        Match *data = &(items[i]);
                        String ts   = String::Part(content, (data->Offset + 1), (data->Length - 2));

                        if (data->NestMatch.Size != 0) {
                            ts = Engine::Parse(ts, Engine::Search(ts, JsonDeQuot));
                        }

                        document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::StringT, &ts, true);

                        done = true;
                    } break;
                    case L'{': {
                        i++;

                        Document uno_document = Document();
                        Document::_makeDocument(uno_document, content, items[i].NestMatch);
                        document.Insert(content, (key->Offset + 1), (key->Length - 2), VType::DocumentT, &uno_document,
                                        true);

                        done = true;
                    } break;
                    case L'[': {
                        i++;

                        Document o_document = Document();
                        o_document.Ordered  = true;
                        Match *data         = &(items[i]);

                        if (items[i].NestMatch.Size > 0) {
                            Document::_makeDocument(o_document, content, data->NestMatch);
                        } else {
                            Document::_makeNumberedDocument(o_document, content, *data);
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
                        } else {
                            // Error converting a number.
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
            if (items[0].Expr->Keyword == L'"') { // Strings
                document.Strings.SetCapacity(items.Size);

                Match *data;
                for (UNumber i = 0; i < items.Size; i++) {
                    data = &(items[i]);
                    if (data->NestMatch.Size == 0) {
                        document.Strings.Add(String::Part(content, (data->Offset + 1), (data->Length - 2)));
                    } else {
                        // TODO: Do cleanup this part.
                        String *rs = &(document.Strings[document.Strings.Size]);
                        document.Strings.Size++;

                        *rs = String::Part(content, (data->Offset + 1), (data->Length - 2));
                        // TODO2: Use local replace.
                        Engine::Parse(*rs, Engine::Search(*rs, JsonDeQuot));
                    }
                }
            } else if (items[0].Expr->Keyword == L'}') { // Unordered arrays
                document.Documents.SetCapacity(items.Size);

                for (UNumber i = 0; i < items.Size; i++) {
                    Document *te = &(document.Documents[document.Documents.Size]);
                    document.Documents.Size++;

                    _makeDocument(*te, content, items[i].NestMatch);
                }
            } else if (items[0].Expr->Keyword == L']') { // Ordered arrays
                Match *data;
                document.Documents.SetCapacity(items.Size);

                for (UNumber i = 0; i < items.Size; i++) {
                    Document *te = &(document.Documents[document.Documents.Size]);
                    document.Documents.Size++;
                    te->Ordered = true;

                    data = &(items[i]);
                    if (data->NestMatch.Size > 0) {
                        Document::_makeDocument(*te, content, data->NestMatch);
                    } else {
                        Document::_makeNumberedDocument(*te, content, *data);
                    }
                }
            }
        } else {
            Document::_makeNumberedDocument(document, content, items[0]);
        }
    }
}

void Qentem::Document::MakeDocument(Document &document, const String &content, const Array<Match> &items) noexcept {
    if (items.Size != 0) {
        Match *_item = &(items[0]);
        if (_item->Expr->Keyword == L']') {
            document.Ordered = true;
        }

        if (_item->NestMatch.Size != 0) {
            Document::_makeDocument(document, content, _item->NestMatch);
        } else if (document.Ordered) {
            // Might be: true, false, null or a number.
            // [1,2,3]
            Document::_makeNumberedDocument(document, content, *_item);
        }
    }
}

String Qentem::Document::ToJSON() const noexcept {
    Document::SetJsonQuot();

    return Document::_ToJSON().Eject();
}

StringStream Qentem::Document::_ToJSON() const noexcept {
    StringStream ss;

    if (!Ordered) {
        Entry *_ptr;

        ss += L'{';
        for (UNumber i = 0; i < Entries.Size; i++) {
            _ptr = &(Entries[i]);

            if (ss.Length != 1) {
                ss += L',';
            }

            switch (_ptr->Type) {
                case VType::NullT: {
                    ss += L'"';
                    ss += Keys[_ptr->KeyID];
                    ss += L"\":null"; // TODO: implement share(); to have a pointer to this inseat of copying it.
                } break;
                case VType::BooleanT: {
                    ss += L'"';
                    ss += Keys[_ptr->KeyID];

                    if (Numbers[_ptr->ID] != 0) {
                        ss += L"\":true";
                    } else {
                        ss += L"\":false";
                    }
                } break;
                case VType::StringT: {
                    ss += L'"';
                    ss += Keys[_ptr->KeyID];
                    ss += L"\":\"";
                    ss += Engine::Parse(Strings[_ptr->ID], Engine::Search(Strings[_ptr->ID], JsonQuot));
                    ss += L'"';
                } break;
                case VType::NumberT: {
                    ss += L'"';
                    ss += Keys[_ptr->KeyID];
                    ss += L"\":";
                    ss += String::FromNumber(Numbers[_ptr->ID]);
                } break;
                case VType::DocumentT: {
                    ss += L'"';
                    ss += Keys[_ptr->KeyID];
                    ss += L"\":";
                    ss += Documents[_ptr->ID]._ToJSON().Eject();
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
                ss += Engine::Parse(Strings[i], Engine::Search(Strings[i], JsonQuot));
                ss += L'"';
            }
        } else if (Numbers.Size != 0) {
            for (UNumber i = 0; i < Numbers.Size; i++) {
                if (ss.Length != 1) {
                    ss += L',';
                }

                ss += String::FromNumber(Numbers[i]);
            }
        } else if (Documents.Size != 0) {
            for (UNumber i = 0; i < Documents.Size; i++) {
                if (ss.Length != 1) {
                    ss += L',';
                }

                ss += Documents[i]._ToJSON().Eject();
            }
        }

        ss += L']';
    }

    return ss;
}

Expressions Qentem::Document::GetJsonExpres() noexcept {
    static Expressions json_expres;

    if (json_expres.Size != 0) {
        return json_expres;
    }

    SetJsonQuot();

    static Expression opened_square_bracket = Expression();
    static Expression closed_square_bracket = Expression();

    static Expression quotation_start = Expression();
    static Expression quotation_end   = Expression();

    static Expression opened_curly_bracket = Expression();
    static Expression closed_curly_bracket = Expression();

    static Expression esc_quotation = Expression();
    esc_quotation.Keyword           = L"\\\"";

    static Expression comment1      = Expression();
    static Expression comment_next1 = Expression();
    comment1.Keyword                = L"/*";
    comment_next1.Keyword           = L"*/";
    comment1.Connected              = &comment_next1;

    static Expression comment2      = Expression();
    static Expression comment_next2 = Expression();
    comment2.Keyword                = L"//";
    comment_next2.Keyword           = L"\n";
    comment2.Connected              = &comment_next2;

    quotation_start.Keyword   = L'"';
    quotation_end.Keyword     = L'"';
    quotation_start.Connected = &quotation_end;
    quotation_end.NestExprs.Add(&esc_quotation);

    opened_curly_bracket.Keyword   = L'{';
    closed_curly_bracket.Keyword   = L'}';
    opened_curly_bracket.Connected = &closed_curly_bracket;
    closed_curly_bracket.NestExprs.Add(&quotation_start).Add(&opened_square_bracket).Add(&opened_curly_bracket);

    opened_square_bracket.Keyword   = L'[';
    closed_square_bracket.Keyword   = L']';
    opened_square_bracket.Connected = &closed_square_bracket;
    closed_square_bracket.NestExprs.Add(&quotation_start).Add(&opened_square_bracket).Add(&opened_curly_bracket);

    json_expres.Add(&opened_curly_bracket).Add(&opened_square_bracket).Add(&comment1).Add(&comment2);

    return json_expres;
}

Qentem::Document Qentem::Document::FromJSON(const String &content) noexcept {
    Document document;
    // Engine::Search(content, GetJsonExpres());
    Document::MakeDocument(document, content, Engine::Search(content, GetJsonExpres()));
    return document;
}

Expressions Qentem::Document::JsonQuot   = Expressions(); // See SetJsonQuot()
Expressions Qentem::Document::JsonDeQuot = Expressions(); // TODO: move it inside function
