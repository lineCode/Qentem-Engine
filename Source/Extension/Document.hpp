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

enum VType { UndefinedT = 0, NumberT = 1, StringT = 2, DocumentT = 3, FalseT = 4, TrueT = 5, NullT = 6 };

struct Index {
    UNumber      Hash{0};
    UNumber      EntryID{0};
    Array<Index> Table;

    Index() = default;

    Index(UNumber hash, UNumber e_id) : Hash(hash), EntryID(e_id) {
    }
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

    UNumber       HashBase{17}; // Or 97; a prime number only!
    Array<String> Keys;
    Array<Index>  Table;
    Array<Entry>  Entries;

    Array<double>   Numbers;
    Array<String>   Strings;
    Array<Document> Documents;

    UNumber        LastKeyLen{0};
    wchar_t const *LastKey{nullptr};

    Document()                    = default;
    virtual ~Document()           = default;
    Document(Document &&doc)      = default;
    Document(Document const &doc) = default;

    Document(Array<double> const &numbers) noexcept {
        Numbers = numbers;
        Entries.SetCapacity(numbers.Size);

        for (UNumber i = 0; i < numbers.Size; i++) {
            Entries += Entry({VType::NumberT, 0, i});
        }

        Ordered = true;
    }

    Document(Array<String> const &strings) noexcept {
        Strings = strings;
        Entries.SetCapacity(strings.Size);

        for (UNumber i = 0; i < strings.Size; i++) {
            Entries += Entry({VType::StringT, 0, i});
        }

        Ordered = true;
    }

    Document(Array<Document> const &documents) noexcept {
        Documents = documents;
        Entries.SetCapacity(documents.Size);

        for (UNumber i = 0; i < documents.Size; i++) {
            Entries += Entry({VType::DocumentT, 0, i});
        }

        Ordered = true;
    }

    Document(Array<double> &&numbers) noexcept {
        Numbers = static_cast<Array<double> &&>(numbers);
        Entries.SetCapacity(numbers.Size);

        for (UNumber i = 0; i < numbers.Size; i++) {
            Entries += Entry({VType::NumberT, 0, i});
        }

        Ordered = true;
    }

    Document(Array<String> &&strings) noexcept {
        Strings = static_cast<Array<String> &&>(strings);
        Entries.SetCapacity(strings.Size);

        for (UNumber i = 0; i < strings.Size; i++) {
            Entries += Entry({VType::StringT, 0, i});
        }

        Ordered = true;
    }

    Document(Array<Document> &&documents) noexcept {
        Documents = static_cast<Array<Document> &&>(documents);
        Entries.SetCapacity(documents.Size);

        for (UNumber i = 0; i < documents.Size; i++) {
            Entries += Entry({VType::DocumentT, 0, i});
        }

        Ordered = true;
    }

    Document(wchar_t const *value) noexcept {
        if (value != nullptr) {
            Array<Match> const items(Engine::Search(value, getJsonExpres(), 0, String::Count(value)));

            if (items.Size != 0) {
                *this = makeList(value, items[0]);
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
        Array<Match> const items(Engine::Search(value.Str, getJsonExpres(), 0, value.Length));

        if (items.Size != 0) {
            *this = makeList(value.Str, items[0]);
        } else {
            Ordered = true;
            Strings += value;
            Entries += Entry({VType::StringT, 0, 0});
        }
    }

    Document(String &&value) noexcept {
        Array<Match> const items(Engine::Search(value.Str, getJsonExpres(), 0, value.Length));

        if (items.Size != 0) {
            *this = makeList(value.Str, items[0]);
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

    static void Drop(Entry &entry, Document &storage) noexcept {
        entry.Type = VType::UndefinedT;

        if (!storage.Ordered) {
            storage.Keys[entry.KeyID].Reset();
        }

        switch (entry.Type) {
            // case VType::NumberT:
            //     storage.Numbers[entry.ArrayID] = 0; // Waste of time.
            //     break;
            case VType::StringT:
                storage.Strings[entry.ArrayID].Reset();
                break;
            case VType::DocumentT:
                storage.Documents[entry.ArrayID].Reset();
                break;
            default:
                break;
        }
    }

    void Drop(wchar_t const *key) noexcept {
        Entry *   entry;
        Document *storage = GetSource(&entry, key, 0, String::Count(key));

        if (entry != nullptr) {
            Drop(*entry, *storage);
        }
    }

    static void InsertIndex(Index const &index, UNumber const hashBase, UNumber const level, Array<Index> &table) noexcept {
        UNumber const id = ((index.Hash + level) % hashBase);

        if (table.Capacity <= id) {
            table.Resize(id + 1);
            table.Size = table.Capacity;
        }

        if (table[id].Hash == 0) {
            table[id] = index;
            return;
        }

        InsertIndex(index, hashBase, (level + id + 2), table[id].Table);
    }

    Entry *Exist(UNumber hash, UNumber const level, Array<Index> const &table) const noexcept {
        UNumber const id = ((hash + level) % HashBase);

        if ((table.Size > id) && (table[id].Hash != 0)) {
            if (table[id].Hash == hash) {
                return &(Entries[table[id].EntryID]);
            }

            return Exist(hash, (level + id + 2), table[id].Table);
        }

        return nullptr;
    }

    void Rehash(UNumber const newBase, bool const children = false) noexcept {
        Table.Reset();
        HashBase = newBase;

        Index   index;
        String *key;

        for (UNumber i = 0; i < Keys.Size; i++) {
            key           = &(Keys[Entries[i].KeyID]);
            index.Hash    = String::Hash(key->Str, 0, key->Length);
            index.EntryID = i;

            InsertIndex(index, HashBase, 0, Table);

            if (children && (Entries[i].Type == VType::DocumentT)) {
                Documents[Entries[i].ArrayID].Rehash(HashBase, true);
            }
        }
    }

    void Insert(UNumber entryID, VType const type, void *ptr, bool const move) noexcept {
        if (entryID < Entries.Size) {
            Entry & entry = Entries[entryID];
            UNumber id    = 0;

            if (entry.Type != type) {
                // New item.
                switch (type) {
                    case VType::NumberT: {
                        id = Numbers.Size;
                        Numbers += *(static_cast<double *>(ptr));
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
                    case VType::DocumentT: {
                        id = Documents.Size;
                        if (move) {
                            Documents += static_cast<Document &&>(*(static_cast<Document *>(ptr)));
                        } else {
                            Documents += *(static_cast<Document *>(ptr));
                        }
                        break;
                    }
                    default:
                        break;
                }

                // Clearing any existing value.
                switch (entry.Type) {
                    case VType::StringT: {
                        Strings[entry.ArrayID].Reset();
                    } break;
                    case VType::DocumentT: {
                        Documents[entry.ArrayID].Reset();
                    } break;
                    default:
                        break;
                }

                entry.ArrayID = id;
                entry.Type    = type;
            } else {
                // Updating existing item.
                switch (type) {
                    case VType::NumberT: {
                        Numbers[entry.ArrayID] = *(static_cast<double *>(ptr));
                        break;
                    }
                    case VType::StringT: {
                        if (move) {
                            Strings[entry.ArrayID] = static_cast<String &&>(*(static_cast<String *>(ptr)));
                        } else {
                            Strings[entry.ArrayID] = *(static_cast<String *>(ptr));
                        }
                        break;
                    }
                    case VType::DocumentT: {
                        if (move) {
                            Documents[entry.ArrayID] = static_cast<Document &&>(*(static_cast<Document *>(ptr)));
                        } else {
                            Documents[entry.ArrayID] = *(static_cast<Document *>(ptr));
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }

    void InsertHash(UNumber id, wchar_t const *key, UNumber const offset, UNumber const limit, VType const type) noexcept {
        InsertIndex({String::Hash(key, offset, limit), Entries.Size}, HashBase, 0, Table);
        Entries += Entry({type, Keys.Size, id});
        Keys += String::Part(key, offset, limit);
    }

    void Insert(wchar_t const *key, UNumber const offset, UNumber const limit, VType const type, void *ptr, bool const move) noexcept {
        UNumber       id    = 0;
        UNumber const hash  = String::Hash(key, offset, limit);
        Entry *       entry = Exist(hash, 0, Table);

        if ((entry == nullptr) || (entry->Type != type)) {
            // New item.
            switch (type) {
                case VType::NumberT: {
                    id = Numbers.Size;
                    Numbers += *(static_cast<double *>(ptr));
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
                case VType::DocumentT: {
                    id = Documents.Size;
                    if (move) {
                        Documents += static_cast<Document &&>(*(static_cast<Document *>(ptr)));
                    } else {
                        Documents += *(static_cast<Document *>(ptr));
                    }
                    break;
                }
                default:
                    break;
            }
        } else {
            // Updating existing item.
            switch (type) {
                case VType::NumberT: {
                    Numbers[entry->ArrayID] = *(static_cast<double *>(ptr));
                    break;
                }
                case VType::StringT: {
                    if (move) {
                        Strings[entry->ArrayID] = static_cast<String &&>(*(static_cast<String *>(ptr)));
                    } else {
                        Strings[entry->ArrayID] = *(static_cast<String *>(ptr));
                    }
                    break;
                }
                case VType::DocumentT: {
                    if (move) {
                        Documents[entry->ArrayID] = static_cast<Document &&>(*(static_cast<Document *>(ptr)));
                    } else {
                        Documents[entry->ArrayID] = *(static_cast<Document *>(ptr));
                    }
                    break;
                }
                default:
                    break;
            }
        }

        if (entry != nullptr) {
            // If exists ...
            if (entry->Type != type) {
                // Clearing any existing value.
                switch (entry->Type) {
                    case VType::StringT: {
                        Strings[entry->ArrayID].Reset();
                    } break;
                    case VType::DocumentT: {
                        Documents[entry->ArrayID].Reset();
                    } break;
                    default:
                        break;
                }

                entry->ArrayID = id;
                entry->Type    = type;
            }

            return;
        }

        InsertIndex({hash, Entries.Size}, HashBase, 0, Table);
        Entries += Entry({type, Keys.Size, id});
        Keys += String::Part(key, offset, limit);
    }

    static Document makeList(wchar_t const *content, Match const &match) noexcept {
        Document            document;
        Array<Match> const &items = match.NestMatch;

        Match *item;

        bool          done    = false;
        UNumber       item_id = 0;
        UNumber       offset  = (match.Offset + 1); // the starting char [
        UNumber       start   = offset;
        UNumber const length  = (match.Length + match.Offset);

        if (match.Expr->ID == 1) {
            UNumber j;

            for (UNumber i = 0; i < items.Size; i++) {
                item = &(items[i]);
                j    = (item->Offset + item->Length);

                for (; j < length; j++) {
                    while (content[j] == L' ') {
                        ++j;
                    }

                    switch (content[j]) {
                        case L',':
                        case L'}': {
                            // A true, false, null or number value.
                            UNumber limit = j - start;
                            String::SoftTrim(content, start, limit);

                            switch (content[start]) {
                                case L'f': {
                                    // False
                                    document.InsertHash(0, content, (item->Offset + 1), (item->Length - 2), VType::FalseT);
                                    break;
                                }
                                case L't': {
                                    // True

                                    document.InsertHash(0, content, (item->Offset + 1), (item->Length - 2), VType::TrueT);
                                    break;
                                }
                                case L'n': {
                                    // Null
                                    document.InsertHash(0, content, (item->Offset + 1), (item->Length - 2), VType::NullT);
                                    break;
                                }
                                default: {
                                    double number;
                                    if (String::ToNumber(number, content, start, limit)) {
                                        // Number
                                        document.InsertHash(document.Numbers.Size, content, (item->Offset + 1), (item->Length - 2),
                                                            VType::NumberT);
                                        document.Numbers += number;
                                    }
                                    break;
                                }
                            }

                            done = true;
                            break;
                        }
                        case L'"': {
                            ++i;

                            document.InsertHash(document.Strings.Size, content, (item->Offset + 1), (item->Length - 2), VType::StringT);

                            if (items[i].NestMatch.Size == 0) {
                                document.Strings += String::Part(content, (items[i].Offset + 1), (items[i].Length - 2));
                            } else {
                                document.Strings +=
                                    Engine::Parse(content, items[i].NestMatch, (items[i].Offset + 1), (items[i].Length - 2));
                            }

                            done = true;
                            break;
                        }
                        case L':': {
                            start = j + 1;
                            continue;
                        }
                        case L'{':
                        case L'[': {
                            ++i;

                            document.InsertHash(document.Documents.Size, content, (item->Offset + 1), (item->Length - 2), VType::DocumentT);
                            document.Documents += makeList(content, items[i]);

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
        } else {
            document.Ordered = true;

            for (; offset < length; offset++) {
                while (content[offset] == L' ') {
                    ++offset;
                }

                switch (content[offset]) {
                    case L',':
                    case L']': {
                        if (!done) {
                            // A Number, true/false or null
                            UNumber limit = (offset - start);
                            String::SoftTrim(content, start, limit);

                            switch (content[start]) {

                                case L'f': {
                                    // False
                                    document.Entries += Entry({VType::FalseT, 0, 0});
                                    break;
                                }
                                case L't': {
                                    // True
                                    document.Entries += Entry({VType::TrueT, 0, 0});
                                    break;
                                }
                                case L'n': {
                                    // Null
                                    document.Entries += Entry({VType::NullT, 0, 0});
                                    break;
                                }
                                default: {
                                    // A number
                                    double number;
                                    if (String::ToNumber(number, content, start, limit)) {
                                        document.Entries += Entry({VType::NumberT, 0, document.Numbers.Size});
                                        document.Numbers += number;
                                    }
                                    break;
                                }
                            }
                        }

                        start = (offset + 1);
                        done  = false;
                        break;
                    }
                    case L'"': {
                        item = &(items[item_id++]);

                        document.Entries += Entry({VType::StringT, 0, document.Strings.Size});

                        if (item->NestMatch.Size == 0) {
                            document.Strings += String::Part(content, (item->Offset + 1), (item->Length - 2));
                        } else {
                            document.Strings += Engine::Parse(content, item->NestMatch, (item->Offset + 1), (item->Length - 2));
                        }

                        offset = (item->Offset + item->Length) - 1;
                        done   = true;
                        break;
                    }
                    case L'{':
                    case L'[': {
                        item = &(items[item_id++]);

                        document.Entries += Entry({VType::DocumentT, 0, document.Documents.Size});

                        document.Documents += makeList(content, *item);

                        offset = (item->Offset + item->Length) - 1;
                        done   = true;
                        break;
                    }
                }
            }
        }

        return document;
    }

    static Document FromJSON(wchar_t const *content, UNumber const offset, UNumber const limit, bool const comments = false) noexcept {
        Document document;
        if (content == nullptr) {
            return document;
        }

        Array<Match> items;

        String n_content;

        static Expressions const &json_expres = getJsonExpres();

        // C style comments
        if (!comments) {
            items = Engine::Search(content, json_expres, offset, limit);
        } else {
            static Expressions comments_exprs;
            if (comments_exprs.Size == 0) {
                static Expression comment1;
                static Expression comment1_end;
                comment1.SetKeyword(L"/*");
                comment1_end.SetKeyword(L"*/");
                comment1_end.SetReplace(L"\n");
                comment1.Connected = &comment1_end;

                static Expression comment2;
                static Expression comment2_end;
                comment2.SetKeyword(L"//");
                comment2_end.SetKeyword(L"\n");
                comment2_end.SetReplace(L"\n");
                comment2.Connected = &comment2_end;

                comments_exprs.Add(&comment1).Add(&comment2);
            }

            n_content = Engine::Parse(content, Engine::Search(content, comments_exprs, offset, limit), offset, limit);
            items     = Engine::Search(n_content.Str, json_expres, 0, n_content.Length);
        }

        if (items.Size != 0) {
            document = makeList((n_content.Length == 0) ? content : n_content.Str, items[0]);
        }

        return document;
    }

    static Document FromJSON(String const &content, bool const comments = false) noexcept {
        return FromJSON(content.Str, 0, content.Length, comments);
    }

    // Key can be: name/is, [name/id], name/id[name/id], name[name/id][sub-name/id], name/id[name/id][sub-name/id][sub-sub-name/id]...
    // "name": a string a stored value in "Keys" array. "id" is the number of array index that starts with 0: Entries[id]
    Document *GetSource(Entry **entry, wchar_t const *key, UNumber const offset, UNumber limit) noexcept {
        Document *doc = this;

        UNumber start = offset;
        UNumber end   = (offset + limit);

        while ((start < end) && (key[++start] != L'[')) {
        }

        if (end != start) {
            // Starting with a string followed by [...]
            end = start;
            --limit;
        }

        start = offset;

        UNumber const end_offset = (offset + limit);

        while (true) {
            if (doc->Ordered) {
                UNumber ent_id;
                if (!String::ToNumber(ent_id, key, start, (end - start)) || (doc->Entries.Size <= ent_id)) {
                    return nullptr;
                }

                *entry = &(doc->Entries[ent_id]);
            } else if ((*entry = doc->Exist(String::Hash(key, start, (end - start)), 0, doc->Table)) == nullptr) {
                return nullptr;
            }

            if ((*entry)->Type == VType::DocumentT) {
                doc = &(doc->Documents[(*entry)->ArrayID]);
            }

            if (end == end_offset) {
                return doc;
            }

            // Next part
            while ((++start < end_offset) && (key[start] != L'[')) {
            }

            end = ++start;

            while ((end < end_offset) && (key[++end] != L']')) {
            }
        }

        return nullptr;
    }

    bool GetString(String &value, UNumber entryID) const noexcept {
        value.Reset();

        if (entryID < Entries.Size) {
            Entry const &entry = Entries[entryID];

            switch (entry.Type) {
                case VType::NumberT: {
                    value = String::FromNumber(Numbers[entry.ArrayID], 1, 0, 3);
                    return true;
                }
                case VType::StringT: {
                    value = Strings[entry.ArrayID];
                    return true;
                }
                case VType::FalseT: {
                    value = L"false";
                    return true;
                }
                case VType::TrueT: {
                    value = L"true";
                    return true;
                }
                case VType::NullT: {
                    value = L"null";
                    return true;
                }
                default: {
                    return false;
                }
            }
        }

        return false;
    }

    bool GetString(String &value, wchar_t const *key, UNumber const offset, UNumber const limit) noexcept {
        value.Reset();

        Entry *         entry;
        Document const *storage = GetSource(&entry, key, offset, limit);

        if (storage != nullptr) {
            switch (entry->Type) {
                case VType::NumberT: {
                    value = String::FromNumber(storage->Numbers[entry->ArrayID], 1, 0, 3);
                    return true;
                }
                case VType::StringT: {
                    value = storage->Strings[entry->ArrayID];
                    return true;
                }
                case VType::FalseT: {
                    value = L"false";
                    return true;
                }
                case VType::TrueT: {
                    value = L"true";
                    return true;
                }
                case VType::NullT: {
                    value = L"null";
                    return true;
                }
                default: {
                    return false;
                }
            }
        }

        return false;
    }

    bool GetString(String &value, wchar_t const *key) noexcept {
        return GetString(value, key, 0, String::Count(key));
    }

    bool GetNumber(UNumber &value, wchar_t const *key, UNumber const offset, UNumber const limit) noexcept {
        value = 0;

        Entry *         entry;
        Document const *storage = GetSource(&entry, key, offset, limit);

        if (storage != nullptr) {
            switch (entry->Type) {
                case VType::NumberT: {
                    value = static_cast<UNumber>(storage->Numbers[entry->ArrayID]);
                    return true;
                }
                case VType::StringT: {
                    String const &st = storage->Strings[entry->ArrayID];
                    return String::ToNumber(value, st.Str, 0, st.Length);
                    return true;
                }
                case VType::FalseT:
                case VType::NullT: {
                    value = 0;
                    return true;
                }
                case VType::TrueT: {
                    value = 1;
                    return true;
                }
                default: {
                    return false;
                }
            }
        }

        return false;
    }

    bool GetDouble(double &value, wchar_t const *key, UNumber const offset, UNumber const limit) noexcept {
        value = 0.0;

        Entry *         entry;
        Document const *storage = GetSource(&entry, key, offset, limit);

        if (storage != nullptr) {
            switch (entry->Type) {
                case VType::NumberT: {
                    value = storage->Numbers[entry->ArrayID];
                    return true;
                }
                case VType::StringT: {
                    String const &st = storage->Strings[entry->ArrayID];
                    return String::ToNumber(value, st.Str, 0, st.Length);
                    return true;
                }
                case VType::FalseT:
                case VType::NullT: {
                    value = 0;
                    return true;
                }
                case VType::TrueT: {
                    value = 1;
                    return true;
                }
                default: {
                    return false;
                }
            }
        }

        return false;
    }

    bool GetBool(bool &value, wchar_t const *key, UNumber const offset, UNumber const limit) noexcept {
        Entry *         entry;
        Document const *storage = GetSource(&entry, key, offset, limit);

        if (storage != nullptr) {
            switch (entry->Type) {
                case VType::NumberT: {
                    value = (storage->Numbers[entry->ArrayID] > 0.0);
                    return true;
                }
                case VType::StringT: {
                    value = (storage->Strings[entry->ArrayID] == L"true");
                    return true;
                }
                case VType::FalseT:
                case VType::NullT: {
                    value = false;
                    return true;
                }
                case VType::TrueT: {
                    value = true;
                    return true;
                }
                default: {
                    return false;
                }
            }
        }

        return false;
    }

    Document *GetDocument(wchar_t const *key, UNumber const offset, UNumber const limit) noexcept {
        Entry *   entry;
        Document *storage = GetSource(&entry, key, offset, limit);

        if ((storage != nullptr) && (entry->Type == VType::DocumentT)) {
            return storage;
        }

        return nullptr;
    }

    String ToJSON() const noexcept {
        static Expressions const &to_json_expres = getToJsonExpres();

        StringStream ss;
        Entry *      entry;

        if (Ordered) {
            ss.Bits.SetCapacity(2 + (Entries.Size * 2) + (Strings.Size * 2));

            ss += JFX.fss4;

            for (UNumber i = 0; i < Entries.Size; i++) {
                entry = &(Entries[i]);

                if (ss.Length != 1) {
                    ss += JFX.fss3;
                }

                switch (entry->Type) {
                    case VType::NumberT: {
                        ss += String::FromNumber(Numbers[entry->ArrayID]);
                        break;
                    }
                    case VType::StringT: {
                        ss += JFX.fss6;

                        String const &es = Strings[entry->ArrayID];

                        Array<Match> const tmpMatchs(Engine::Search(es.Str, to_json_expres, 0, es.Length));
                        if (tmpMatchs.Size == 0) {
                            ss += es;
                        } else {
                            ss += Engine::Parse(es.Str, tmpMatchs, 0, es.Length);
                        }

                        ss += JFX.fss6;
                        break;
                    }
                    case VType::DocumentT: {
                        ss += Documents[entry->ArrayID].ToJSON();
                        break;
                    }
                    case VType::FalseT: {
                        ss += JFX.fFalse;
                        break;
                    }
                    case VType::TrueT: {
                        ss += JFX.fTrue;
                        break;
                    }
                    default: {
                        ss += JFX.fNull;
                        break;
                    }
                }
            }

            ss += JFX.fss5;
        } else {
            ss.Bits.SetCapacity(2 + (Entries.Size * 6) + (Strings.Size * 2));

            ss += JFX.fss1;

            for (UNumber i = 0; i < Entries.Size; i++) {
                entry = &(Entries[i]);

                if (ss.Length != 1) {
                    ss += JFX.fss3;
                }

                switch (entry->Type) {
                    case VType::NumberT: {
                        ss += JFX.fss6;
                        ss += Keys[entry->KeyID];
                        ss += JFX.fss6;
                        ss += JFX.fsc1;
                        ss += String::FromNumber(Numbers[entry->ArrayID]);
                        break;
                    }
                    case VType::StringT: {
                        ss += JFX.fss6;
                        ss += Keys[entry->KeyID];
                        ss += JFX.fss6;
                        ss += JFX.fsc1;
                        ss += JFX.fss6;

                        String const &es = Strings[entry->ArrayID];

                        Array<Match> const tmpMatchs(Engine::Search(es.Str, to_json_expres, 0, es.Length));
                        if (tmpMatchs.Size == 0) {
                            ss += es;
                        } else {
                            ss += Engine::Parse(es.Str, tmpMatchs, 0, es.Length);
                        }

                        ss += JFX.fss6;
                        break;
                    }
                    case VType::DocumentT: {
                        ss += JFX.fss6;
                        ss += Keys[entry->KeyID];
                        ss += JFX.fss6;
                        ss += JFX.fsc1;
                        ss += Documents[entry->ArrayID].ToJSON();
                        break;
                    }
                    case VType::FalseT: {
                        ss += JFX.fss6;
                        ss += Keys[entry->KeyID];
                        ss += JFX.fss6;
                        ss += JFX.fsc1;
                        ss += JFX.fFalse;
                        break;
                    }
                    case VType::TrueT: {
                        ss += JFX.fss6;
                        ss += Keys[entry->KeyID];
                        ss += JFX.fss6;
                        ss += JFX.fsc1;
                        ss += JFX.fTrue;
                        break;
                    }
                    default: {
                        ss += JFX.fss6;
                        ss += Keys[entry->KeyID];
                        ss += JFX.fss6;
                        ss += JFX.fsc1;
                        ss += JFX.fNull;
                        break;
                    }
                }
            }

            ss += JFX.fss2;
        }

        return ss.Eject();
    }

    static Expressions const &getToJsonExpres() noexcept {
        static Expression  esc;
        static Expression  quot;
        static Expressions tags;

        if (tags.Size == 0) {
            esc.SetKeyword(L"\\");
            esc.MatchCB =
                ([](wchar_t const *content, UNumber &offset, UNumber const endOffset, Match &item, Array<Match> &items) noexcept -> void {
                    if ((content[offset] == L'\\') || (content[offset] == L' ') || (offset == endOffset)) {
                        // If there is a space after \ or \ or it's at the end, then it's a match.
                        items += static_cast<Match &&>(item);
                    }
                });
            esc.SetReplace(L"\\\\");

            quot.SetKeyword(L"\"");
            quot.SetReplace(L"\\\"");

            tags += &esc;
            tags += &quot;
        }

        return tags;
    }

    static Expressions const &getJsonExpres() noexcept {
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

            closed_curly_bracket.NestExprs.SetCapacity(3);
            closed_curly_bracket.NestExprs.Add(&opened_curly_bracket).Add(&quotation_start).Add(&opened_square_bracket);

            opened_square_bracket.SetKeyword(L"[");
            closed_square_bracket.SetKeyword(L"]");
            closed_square_bracket.ID        = 2;
            opened_square_bracket.Connected = &closed_square_bracket;

            closed_square_bracket.NestExprs.SetCapacity(3);
            closed_square_bracket.NestExprs.Add(&opened_square_bracket).Add(&quotation_start).Add(&opened_curly_bracket);

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

        Entry *entry;

        if (Ordered) {
            for (UNumber i = 0; i < doc.Entries.Size; i++) {
                entry = &(doc.Entries[i]);

                switch (entry->Type) {
                    case VType::NumberT: {
                        Entries += Entry({entry->Type, 0, Numbers.Size});
                        Numbers += doc.Numbers[entry->ArrayID];
                        break;
                    }
                    case VType::StringT: {
                        Entries += Entry({entry->Type, 0, Strings.Size});
                        Strings += doc.Strings[entry->ArrayID];
                        break;
                    }
                    case VType::DocumentT: {
                        Entries += Entry({entry->Type, 0, Documents.Size});
                        Documents += doc.Documents[entry->ArrayID];
                        break;
                    }
                    default: {
                        Entries += Entry({entry->Type, 0, 0});
                        break;
                    }
                }
            }
        } else {
            String *key;

            for (UNumber i = 0; i < doc.Entries.Size; i++) {
                entry = &(doc.Entries[i]);
                key   = &(doc.Keys[doc.Entries[i].KeyID]);

                switch (entry->Type) {
                    case VType::NumberT: {
                        Insert(key->Str, 0, key->Length, entry->Type, &doc.Numbers[entry->ArrayID], false);
                        break;
                    }
                    case VType::StringT: {
                        Insert(key->Str, 0, key->Length, entry->Type, &doc.Strings[entry->ArrayID], false);
                        break;
                    }
                    case VType::DocumentT: {
                        Insert(key->Str, 0, key->Length, entry->Type, &doc.Documents[entry->ArrayID], false);
                        break;
                    }
                    default: {
                        Insert(key->Str, 0, key->Length, entry->Type, nullptr, false);
                        break;
                    }
                }
            }
        }
    }

    void operator+=(Document &&doc) noexcept {
        if (Ordered != doc.Ordered) {
            // Not the same type!
            return;
        }

        Entry *entry;

        if (Ordered) {
            for (UNumber i = 0; i < doc.Entries.Size; i++) {
                entry = &(doc.Entries[i]);

                switch (entry->Type) {
                    case VType::NumberT: {
                        Entries += Entry({entry->Type, 0, Numbers.Size});
                        Numbers += doc.Numbers[entry->ArrayID];
                        break;
                    }
                    case VType::StringT: {
                        Entries += Entry({entry->Type, 0, Strings.Size});
                        Strings += static_cast<String &&>(doc.Strings[entry->ArrayID]);
                        break;
                    }
                    case VType::DocumentT: {
                        Entries += Entry({entry->Type, 0, Documents.Size});
                        Documents += static_cast<Document &&>(doc.Documents[entry->ArrayID]);
                        break;
                    }
                    default: {
                        Entries += Entry({entry->Type, 0, 0});
                        break;
                    }
                }
            }
        } else {
            String *key;

            for (UNumber i = 0; i < doc.Entries.Size; i++) {
                entry = &(doc.Entries[i]);
                key   = &(doc.Keys[doc.Entries[i].KeyID]);

                switch (entry->Type) {
                    case VType::NumberT: {
                        Insert(key->Str, 0, key->Length, entry->Type, &doc.Numbers[entry->ArrayID], false);
                        break;
                    }
                    case VType::StringT: {
                        Insert(key->Str, 0, key->Length, entry->Type, &doc.Strings[entry->ArrayID], true);
                        break;
                    }
                    case VType::DocumentT: {
                        Insert(key->Str, 0, key->Length, entry->Type, &doc.Documents[entry->ArrayID], true);
                        break;
                    }
                    default: {
                        Insert(key->Str, 0, key->Length, entry->Type, nullptr, false);
                        break;
                    }
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
            Insert(LastKey, 0, LastKeyLen, VType::DocumentT, &doc, true);
            Entry *entry;
            storage = GetSource(&entry, LastKey, 0, LastKeyLen);
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
            Insert(LastKey, 0, LastKeyLen, VType::DocumentT, &doc_copy, true);

            Entry *entry;
            storage = GetSource(&entry, LastKey, 0, LastKeyLen);
        }

        LastKey    = nullptr;
        LastKeyLen = 0;

        return *storage;
    }

    Document &operator=(wchar_t const *str) noexcept {
        if (LastKeyLen == 0) {
            this->Reset();
            this->Ordered = true;
            LastKeyLen    = 1;
        }

        if (Ordered) {
            --LastKeyLen;

            if (str != nullptr) {
                String string = str;
                Insert(LastKeyLen, VType::StringT, &string, true);
            } else {
                Insert(LastKeyLen, VType::NullT, nullptr, false);
            }
        } else {
            if (str != nullptr) {
                String string = str;
                Insert(LastKey, 0, LastKeyLen, VType::StringT, &string, true);
            } else {
                Insert(LastKey, 0, LastKeyLen, VType::NullT, nullptr, false);
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
                Insert(LastKey, 0, LastKeyLen, VType::NumberT, &number, false);
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
                Insert(LastKey, 0, LastKeyLen, VType::NumberT, &number, false);
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
                Insert(LastKey, 0, LastKeyLen, VType::NumberT, &number, false);
            }

            LastKey    = nullptr;
            LastKeyLen = 0;
        }

        return *this;
    }

    Document &operator=(bool const value) noexcept {
        if (LastKeyLen != 0) {
            if (Ordered) {
                Insert(--LastKeyLen, (value ? VType::TrueT : VType::FalseT), nullptr, false);
            } else {
                Insert(LastKey, 0, LastKeyLen, (value ? VType::TrueT : VType::FalseT), nullptr, false);
            }

            LastKey    = nullptr;
            LastKeyLen = 0;
        }

        return *this;
    }

    void operator+=(bool const value) noexcept {
        if (Ordered) {
            Entries += Entry({(value ? VType::TrueT : VType::FalseT), 0, 0});
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
        if (Ordered) {
            if (str != nullptr) {
                Entries += Entry({VType::StringT, 0, Strings.Size});
                Strings += str;
            } else {
                Entries += Entry({VType::NullT, 0, 0});
            }
        }
    }

    void operator+=(String const &string) noexcept {
        if (Ordered) {
            Entries += Entry({VType::StringT, 0, Strings.Size});
            Strings += string;
        }
    }

    void operator+=(String &&string) noexcept {
        if (Ordered) {
            Entries += Entry({VType::StringT, 0, Strings.Size});
            Strings += static_cast<String &&>(string);
        }
    }

    void operator+=(Array<double> const &numbers) noexcept {
        if (Ordered) {
            UNumber id = Numbers.Size;

            for (UNumber i = 0; i < numbers.Size; i++) {
                Entries += Entry({VType::NumberT, 0, id++});
            }

            Numbers += numbers;
        }
    }

    void operator+=(Array<String> const &strings) noexcept {
        if (Ordered) {
            UNumber id = Strings.Size;

            for (UNumber i = 0; i < strings.Size; i++) {
                Entries += Entry({VType::StringT, 0, id++});
            }

            Strings += strings;
        }
    }

    void operator+=(Array<Document> const &documents) noexcept {
        if (Ordered) {
            UNumber id = Documents.Size;

            for (UNumber i = 0; i < documents.Size; i++) {
                Entries += Entry({VType::DocumentT, 0, id++});
            }

            Documents += documents;
        }
    }

    void operator+=(Array<double> &&numbers) noexcept {
        if (Ordered) {
            UNumber id = Numbers.Size;

            for (UNumber i = 0; i < numbers.Size; i++) {
                Entries += Entry({VType::NumberT, 0, id++});
            }

            Numbers += numbers;
        }
    }

    void operator+=(Array<String> &&strings) noexcept {
        if (Ordered) {
            UNumber id = Strings.Size;

            for (UNumber i = 0; i < strings.Size; i++) {
                Entries += Entry({VType::StringT, 0, id++});
            }

            Strings += static_cast<Array<String> &&>(strings);
        }
    }

    void operator+=(Array<Document> &&documents) noexcept {
        if (Ordered) {
            UNumber id = Documents.Size;

            for (UNumber i = 0; i < documents.Size; i++) {
                Entries += Entry({VType::DocumentT, 0, id++});
            }

            Documents += static_cast<Array<Document> &&>(documents);
        }
    }

    Document &operator[](wchar_t const *key) noexcept {
        UNumber const str_len = String::Count(key);

        if (Ordered) {
            UNumber id;
            if (String::ToNumber(id, key, 0, str_len)) {
                if (Entries[id].Type == VType::DocumentT) {
                    return Documents[Entries[id].ArrayID];
                }

                LastKeyLen = (id + 1);
            }
        } else {
            Entry const *entry = Exist(String::Hash(key, 0, str_len), 0, Table);

            if ((entry != nullptr) && (entry->Type == VType::DocumentT)) {
                return Documents[entry->ArrayID];
            }

            LastKey    = key;
            LastKeyLen = str_len;
        }

        return *this;
    }

    Document &operator[](String const &key) noexcept {
        if (Ordered) {
            UNumber id;
            if (String::ToNumber(id, key.Str, 0, key.Length)) {
                if (Entries[id].Type == VType::DocumentT) {
                    return Documents[Entries[id].ArrayID];
                }

                LastKeyLen = (id + 1);
            }
        } else {
            Entry const *entry = Exist(String::Hash(key.Str, 0, key.Length), 0, Table);

            if ((entry != nullptr) && (entry->Type == VType::DocumentT)) {
                return Documents[entry->ArrayID];
            }

            LastKey    = key.Str;
            LastKeyLen = key.Length;
        }

        return *this;
    }

    Document &operator[](UNumber const id) noexcept {
        Entry const &entry = Entries[id];

        if (entry.Type == VType::DocumentT) {
            return Documents[entry.ArrayID];
        }

        if (Ordered) {
            LastKeyLen = (id + 1);
        } else {
            LastKey    = Keys[entry.KeyID].Str;
            LastKeyLen = Keys[entry.KeyID].Length;
        }

        return *this;
    }

    Document &operator[](int const id) noexcept {
        UNumber const ID    = static_cast<UNumber>(id);
        Entry const & entry = Entries[ID];

        if (entry.Type == VType::DocumentT) {
            return Documents[entry.ArrayID];
        }

        if (Ordered) {
            LastKeyLen = (ID + 1);
        } else {
            LastKey    = Keys[entry.KeyID].Str;
            LastKeyLen = Keys[entry.KeyID].Length;
        }

        return *this;
    }
};

} // namespace Qentem

#endif
