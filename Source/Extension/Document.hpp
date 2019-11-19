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
using Engine::MatchBit;

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
    const char *fss1   = "{";
    const char *fss2   = "}";
    const char *fss3   = ",";
    const char *fss4   = "[";
    const char *fss5   = "]";
    const char *fss6   = "\"";
    const char *fsc1   = ":";
    const char *fNull  = "null";
    const char *fTrue  = "true";
    const char *fFalse = "false";
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

    UNumber     LastKeyLen{0};
    const char *LastKey{nullptr};

    static constexpr const char *char_list = "{[]}\\\\\"";

    Document()                    = default;
    virtual ~Document()           = default;
    Document(Document &&doc)      = default;
    Document(const Document &doc) = default;

    Document(const Array<double> &numbers) noexcept {
        Numbers = numbers;
        Entries.SetCapacity(numbers.Size);

        for (UNumber i = 0; i < numbers.Size; i++) {
            Entries += Entry({VType::NumberT, 0, i});
        }

        Ordered = true;
    }

    Document(const Array<String> &strings) noexcept {
        Strings = strings;
        Entries.SetCapacity(strings.Size);

        for (UNumber i = 0; i < strings.Size; i++) {
            Entries += Entry({VType::StringT, 0, i});
        }

        Ordered = true;
    }

    Document(const Array<Document> &documents) noexcept {
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

    Document(const char *value) noexcept {
        if (value != nullptr) {
            const Array<MatchBit> items(Engine::Match(getJsonExpres(), value, 0, String::Count(value)));

            if (items.Size != 0) {
                *this = makeList(items[0].NestMatch, value, items[0].Offset, items[0].Length);
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

    Document(const String &value) noexcept {
        const Array<MatchBit> items(Engine::Match(getJsonExpres(), value.Str, 0, value.Length));

        if (items.Size != 0) {
            *this = makeList(items[0].NestMatch, value.Str, items[0].Offset, items[0].Length);
        } else {
            Ordered = true;
            Strings += value;
            Entries += Entry({VType::StringT, 0, 0});
        }
    }

    Document(String &&value) noexcept {
        const Array<MatchBit> items(Engine::Match(getJsonExpres(), value.Str, 0, value.Length));

        if (items.Size != 0) {
            *this = makeList(items[0].NestMatch, value.Str, items[0].Offset, items[0].Length);
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

    static void Delete(Entry &entry, const Document &storage) noexcept {
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

    void Delete(const UNumber id) noexcept {
        if (id < Entries.Size) {
            Delete(Entries[id], *this);
        }
    }

    void Delete(const char *key) noexcept {
        Entry *         entry;
        const Document *storage = GetSource(&entry, key, 0, String::Count(key));

        if (storage != nullptr) {
            Delete(*entry, *storage);
        }
    }

    static void InsertIndex(const Index &index, const UNumber hashBase, const UNumber level, Array<Index> &table) noexcept {
        const UNumber id = ((index.Hash + level) % hashBase);

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

    Entry *Exist(UNumber hash, const UNumber level, const Array<Index> &table) const noexcept {
        const UNumber id = ((hash + level) % HashBase);

        if ((table.Size > id) && (table[id].Hash != 0)) {
            if (table[id].Hash == hash) {
                return &(Entries[table[id].EntryID]);
            }

            return Exist(hash, (level + id + 2), table[id].Table);
        }

        return nullptr;
    }

    void Rehash(const UNumber newBase, const bool children = false) noexcept {
        if (!Ordered) {
            Table.Reset();
            HashBase = newBase;

            Index         index;
            const String *key;
            const Entry * entry;

            for (UNumber i = 0; i < Entries.Size; i++) {
                entry = &(Entries[i]);

                if (entry->Type != VType::UndefinedT) {
                    key           = &(Keys[entry->KeyID]);
                    index.Hash    = String::Hash(key->Str, 0, key->Length);
                    index.EntryID = i;

                    InsertIndex(index, HashBase, 0, Table);

                    if (children && (entry->Type == VType::DocumentT)) {
                        Documents[entry->ArrayID].Rehash(HashBase, true);
                    }
                }
            }
        }
    }

    void Insert(UNumber entryID, const VType type, void *ptr, const bool move) noexcept {
        if (entryID >= Entries.Size) {
            // Filling the gap with the same value.

            UNumber nid = Entries.Size;

            Entries.Resize(entryID + 1);
            Entries.Size = Entries.Capacity;

            while (nid < entryID) {
                Insert(nid++, type, ptr, false);
            }
        }

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

    void InsertHash(UNumber id, const char *key, const UNumber offset, const UNumber limit, const VType type) noexcept {
        InsertIndex({String::Hash(key, offset, limit), Entries.Size}, HashBase, 0, Table);
        Entries += Entry({type, Keys.Size, id});
        Keys += String::Part(key, offset, limit);
    }

    UNumber Insert(const char *key, const UNumber offset, const UNumber limit, const VType type, void *ptr, const bool move) noexcept {
        UNumber       id    = 0;
        const UNumber hash  = String::Hash(key, offset, limit);
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
            id = entry->ArrayID;
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

            return id;
        }

        InsertIndex({hash, Entries.Size}, HashBase, 0, Table);
        Entries += Entry({type, Keys.Size, id});
        Keys += String::Part(key, offset, limit);

        return id;
    }

    static Document makeList(Array<MatchBit> &items, const char *content, const UNumber offset, const UNumber length) noexcept {
        Document  document;
        MatchBit *item;
        UNumber   x;

        bool          done           = false;
        UNumber       item_id        = 0;
        UNumber       current_offset = (offset + 1);
        const UNumber end            = (length + offset);

        if (content[offset] == '{') {
            while (item_id < items.Size) {
                item = &(items[item_id++]);
                x    = (item->Offset + item->Length);

                for (; x < end; x++) {
                    while (content[x] == ' ') {
                        ++x;
                    }

                    switch (content[x]) {
                        case ',':
                        case '}': {
                            // A true, false, null or number value.
                            UNumber limit = x - current_offset;
                            String::SoftTrim(content, current_offset, limit);

                            switch (content[current_offset]) {
                                case 'f': {
                                    // False
                                    document.InsertHash(0, content, (item->Offset + 1), (item->Length - 2), VType::FalseT);
                                    break;
                                }
                                case 't': {
                                    // True
                                    document.InsertHash(0, content, (item->Offset + 1), (item->Length - 2), VType::TrueT);
                                    break;
                                }
                                case 'n': {
                                    // Null
                                    document.InsertHash(0, content, (item->Offset + 1), (item->Length - 2), VType::NullT);
                                    break;
                                }
                                default: {
                                    double number;
                                    if (String::ToNumber(number, content, current_offset, limit)) {
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
                        case '"': {
                            document.InsertHash(document.Strings.Size, content, (item->Offset + 1), (item->Length - 2), VType::StringT);

                            if (items[item_id].NestMatch.Size == 0) {
                                document.Strings += String::Part(content, (items[item_id].Offset + 1), (items[item_id].Length - 2));
                            } else {
                                document.Strings += Engine::Parse(items[item_id].NestMatch, content, (items[item_id].Offset + 1),
                                                                  (items[item_id].Length - 2));
                            }

                            ++item_id;
                            done = true;
                            break;
                        }
                        case ':': {
                            current_offset = x + 1;
                            continue;
                        }
                        case '{':
                        case '[': {

                            document.InsertHash(document.Documents.Size, content, (item->Offset + 1), (item->Length - 2), VType::DocumentT);
                            document.Documents += makeList(items[item_id].NestMatch, content, items[item_id].Offset, items[item_id].Length);

                            items[item_id].NestMatch.Reset();
                            ++item_id;
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

            for (x = current_offset; x < end; x++) {
                while (content[x] == ' ') {
                    ++x;
                }

                switch (content[x]) {
                    case ',':
                    case ']': {
                        if (!done) {
                            // A Number, true/false or null
                            UNumber limit = (x - current_offset);
                            String::SoftTrim(content, current_offset, limit);

                            switch (content[current_offset]) {

                                case 'f': {
                                    // False
                                    document.Entries += Entry({VType::FalseT, 0, 0});
                                    break;
                                }
                                case 't': {
                                    // True
                                    document.Entries += Entry({VType::TrueT, 0, 0});
                                    break;
                                }
                                case 'n': {
                                    // Null
                                    document.Entries += Entry({VType::NullT, 0, 0});
                                    break;
                                }
                                default: {
                                    // A number
                                    double number;
                                    if (String::ToNumber(number, content, current_offset, limit)) {
                                        document.Entries += Entry({VType::NumberT, 0, document.Numbers.Size});
                                        document.Numbers += number;
                                    }
                                    break;
                                }
                            }
                        }

                        current_offset = (x + 1);
                        done           = false;
                        break;
                    }
                    case '"': {
                        item = &(items[item_id++]);

                        document.Entries += Entry({VType::StringT, 0, document.Strings.Size});

                        if (item->NestMatch.Size == 0) {
                            document.Strings += String::Part(content, (item->Offset + 1), (item->Length - 2));
                        } else {
                            document.Strings += Engine::Parse(item->NestMatch, content, (item->Offset + 1), (item->Length - 2));
                        }

                        x    = ((item->Offset + item->Length) - 1);
                        done = true;
                        break;
                    }
                    case '{':
                    case '[': {
                        item = &(items[item_id++]);

                        document.Entries += Entry({VType::DocumentT, 0, document.Documents.Size});

                        document.Documents += makeList(item->NestMatch, content, item->Offset, item->Length);

                        item->NestMatch.Reset();

                        x    = ((item->Offset + item->Length) - 1);
                        done = true;
                        break;
                    }
                }
            }
        }

        items.Reset();

        return document;
    }

    static Document FromJSON(const char *content, const UNumber offset, const UNumber limit, const bool comments = false) noexcept {
        if (content == nullptr) {
            return Document();
        }

        Array<MatchBit> items;

        // C style comments
        if (!comments) {
            items = Engine::Match(getJsonExpres(), content, offset, limit);
            if (items.Size != 0) {
                return makeList(items[0].NestMatch, content, items[0].Offset, items[0].Length);
            }
        } else {
            static Expressions comments_expres;
            if (comments_expres.Size == 0) {
                static Expression comment1_end;
                comment1_end.SetKeyword("*/");
                comment1_end.SetReplace("\n");

                static Expression comment1;
                comment1.SetKeyword("/*");
                comment1.Connected = &comment1_end;

                static Expression comment2_end;
                comment2_end.SetKeyword("\n");
                comment2_end.SetReplace("\n");

                static Expression comment2;
                comment2.SetKeyword("//");
                comment2.Connected = &comment2_end;

                comments_expres.Add(&comment1).Add(&comment2);
            }

            String n_content(Engine::Parse(Engine::Match(comments_expres, content, offset, limit), content, offset, limit));
            items = Engine::Match(getJsonExpres(), n_content.Str, 0, n_content.Length);

            if (items.Size != 0) {
                return makeList(items[0].NestMatch, n_content.Str, items[0].Offset, items[0].Length);
            }
        }

        return Document();
    }

    static Document FromJSON(const String &content, const bool comments = false) noexcept {
        return FromJSON(content.Str, 0, content.Length, comments);
    }

    // Key can be: name/id, name/id[name/id], name/id[name/id][sub-name/id], name/id[name/id][sub-name/id][sub-sub-name/id]...
    // "name": a string that's stored in "Keys". "id" is the number of array index that starts with 0: Entries[id]
    const Document *GetSource(Entry **entry, const char *key, const UNumber offset, UNumber limit) const noexcept {
        if ((key == nullptr) || (limit == 0)) {
            return nullptr;
        }

        const Document *doc = this;

        UNumber curent_offset = offset;
        UNumber end           = (offset + limit);

        if (key[end - 1] == ']') {
            // Starting with a string followed by [...]
            while ((curent_offset < end) && (key[++curent_offset] != '[')) {
            }

            end = curent_offset;
            --limit;
        }

        curent_offset = offset;

        const UNumber end_offset = (offset + limit);

        while (true) {
            if (doc->Ordered) {
                UNumber entry_id;
                if (!String::ToNumber(entry_id, key, curent_offset, (end - curent_offset)) || (doc->Entries.Size <= entry_id)) {
                    return nullptr;
                }

                *entry = &(doc->Entries[entry_id]);
            } else if ((*entry = doc->Exist(String::Hash(key, curent_offset, (end - curent_offset)), 0, doc->Table)) == nullptr) {
                return nullptr;
            }

            if ((*entry)->Type == VType::DocumentT) {
                doc = &(doc->Documents[(*entry)->ArrayID]);
            }

            if (end == end_offset) {
                return doc;
            }

            // Next part
            while ((++curent_offset < end_offset) && (key[curent_offset] != '[')) {
            }

            end = ++curent_offset;

            while ((end < end_offset) && (key[++end] != ']')) {
            }
        }

        return nullptr;
    }

    static bool GetString(String &value, const Entry &entry, const Document &parent) noexcept {
        switch (entry.Type) {
            case VType::NumberT: {
                value = String::FromNumber(parent.Numbers[entry.ArrayID], 1, 0, 3);
                return true;
            }
            case VType::StringT: {
                value = parent.Strings[entry.ArrayID];
                return true;
            }
            case VType::FalseT: {
                value = "false";
                return true;
            }
            case VType::TrueT: {
                value = "true";
                return true;
            }
            case VType::NullT: {
                value = "null";
                return true;
            }
            default: {
                return false;
            }
        }
    }

    bool GetString(String &value, const UNumber entryID) const noexcept {
        value.Reset();

        if (entryID < Entries.Size) {
            return GetString(value, Entries[entryID], *this);
        }

        return false;
    }

    bool GetString(String &value, const char *key, const UNumber offset, const UNumber limit) const noexcept {
        value.Reset();

        Entry *         entry;
        const Document *storage = GetSource(&entry, key, offset, limit);

        if (storage != nullptr) {
            return GetString(value, *entry, *storage);
        }

        return false;
    }

    bool GetString(String &value, const char *key) noexcept {
        return GetString(value, key, 0, String::Count(key));
    }

    bool GetNumber(UNumber &value, const char *key, const UNumber offset, const UNumber limit) noexcept {
        value = 0;

        Entry *         entry;
        const Document *storage = GetSource(&entry, key, offset, limit);

        if (storage != nullptr) {
            switch (entry->Type) {
                case VType::NumberT: {
                    value = static_cast<UNumber>(storage->Numbers[entry->ArrayID]);
                    return true;
                }
                case VType::StringT: {
                    const String &st = storage->Strings[entry->ArrayID];
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

    bool GetNumber(double &value, const char *key, const UNumber offset, const UNumber limit) noexcept {
        value = 0.0;

        Entry *         entry;
        const Document *storage = GetSource(&entry, key, offset, limit);

        if (storage != nullptr) {
            switch (entry->Type) {
                case VType::NumberT: {
                    value = storage->Numbers[entry->ArrayID];
                    return true;
                }
                case VType::StringT: {
                    const String &st = storage->Strings[entry->ArrayID];
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

    bool GetBool(bool &value, const char *key, const UNumber offset, const UNumber limit) noexcept {
        Entry *         entry;
        const Document *storage = GetSource(&entry, key, offset, limit);

        if (storage != nullptr) {
            switch (entry->Type) {
                case VType::NumberT: {
                    value = (storage->Numbers[entry->ArrayID] > 0.0);
                    return true;
                }
                case VType::StringT: {
                    value = (storage->Strings[entry->ArrayID] == "true");
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

    const Document *GetDocument(const char *key, const UNumber offset, const UNumber limit) noexcept {
        Entry *         entry;
        const Document *storage = GetSource(&entry, key, offset, limit);

        if ((storage != nullptr) && (entry->Type == VType::DocumentT)) {
            return storage;
        }

        return nullptr;
    }

    String ToJSON() const noexcept {
        StringStream ss;
        const Entry *entry;
        UNumber      counter;

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

                        const String &es = Strings[entry->ArrayID];

                        for (counter = 0; counter < es.Length; counter++) {
                            if (es[counter] == '\\' || es[counter] == '"') {
                                counter = 0;
                                break;
                            }
                        }

                        if (counter == es.Length) {
                            ss += es;
                        } else {
                            ss += Engine::Parse(Engine::Match(getToJsonExpres(), es.Str, 0, es.Length), es.Str, 0, es.Length);
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
                    case VType::NullT: {
                        ss += JFX.fNull;
                        break;
                    }
                    default: {
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

                        const String &es = Strings[entry->ArrayID];

                        for (counter = 0; counter < es.Length; counter++) {
                            if (es[counter] == '\\' || es[counter] == '"') {
                                counter = 0;
                                break;
                            }
                        }

                        if (counter == es.Length) {
                            ss += es;
                        } else {
                            ss += Engine::Parse(Engine::Match(getToJsonExpres(), es.Str, 0, es.Length), es.Str, 0, es.Length);
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
                    case VType::NullT: {
                        ss += JFX.fss6;
                        ss += Keys[entry->KeyID];
                        ss += JFX.fss6;
                        ss += JFX.fsc1;
                        ss += JFX.fNull;
                        break;
                    }
                    default: {
                        break;
                    }
                }
            }

            ss += JFX.fss2;
        }

        return ss.ToString();
    }

    static const Expressions &getToJsonExpres() noexcept {
        static Expressions expres;

        if (expres.Size == 0) {
            static Expression esc;
            // esc.SetKeyword("\\");
            esc.Keyword = &(char_list[4]);
            esc.Length  = 1;
            esc.MatchCB = ([](const char *content, UNumber &offset, const UNumber endOffset, MatchBit &item,
                              Array<MatchBit> &items) noexcept -> void {
                if ((content[offset] == '\\') || (content[offset] == ' ') || (offset == endOffset)) {
                    // If there is a space after \ or \ or it's at the end, then it's a match.
                    items += static_cast<MatchBit &&>(item);
                }
            });
            // esc.SetReplace("\\\\");
            esc.ReplaceWith = &(char_list[4]);
            esc.RLength     = 2;

            static Expression quot;
            // quot.SetKeyword("\"");
            quot.Keyword = &(char_list[6]);
            quot.Length  = 1;
            // quot.SetReplace("\\\"");
            quot.ReplaceWith = &(char_list[5]);
            quot.RLength     = 2;

            expres.Add(&esc).Add(&quot);
        }

        return expres;
    }

    static const Expressions &getJsonExpres() noexcept {
        static Expressions expres;

        if (expres.Size == 0) {

            static Expression esc_esc;
            // esc_esc.SetKeyword("\\\\");
            esc_esc.Keyword = &(char_list[4]);
            esc_esc.Length  = 2;
            // esc_esc.SetReplace("\\");
            esc_esc.ReplaceWith = &(char_list[4]);
            esc_esc.RLength     = 1;

            static Expression esc_quotation;
            // esc_quotation.SetKeyword("\\\"");
            esc_quotation.Keyword = &(char_list[5]);
            esc_quotation.Length  = 2;
            // esc_quotation.SetReplace("\"");
            esc_quotation.ReplaceWith = &(char_list[6]);
            esc_quotation.RLength     = 1;

            static Expression quotation_end;
            // quotation_end.SetKeyword("\"");
            quotation_end.Keyword = &(char_list[6]);
            quotation_end.Length  = 1;
            quotation_end.NestExpres += &esc_esc;
            quotation_end.NestExpres += &esc_quotation;

            static Expression quotation_start;
            // quotation_start.SetKeyword("\"");
            quotation_start.Keyword   = &(char_list[6]);
            quotation_start.Length    = 1;
            quotation_start.Connected = &quotation_end;

            static Expression closed_curly_bracket;
            // closed_curly_bracket.SetKeyword("}");
            closed_curly_bracket.Keyword = &(char_list[3]);
            closed_curly_bracket.Length  = 1;

            static Expression opened_curly_bracket;
            // opened_curly_bracket.SetKeyword("{");
            opened_curly_bracket.Keyword   = &(char_list[0]);
            opened_curly_bracket.Length    = 1;
            opened_curly_bracket.Connected = &closed_curly_bracket;

            static Expression closed_square_bracket;
            // closed_square_bracket.SetKeyword("]");
            closed_square_bracket.Keyword = &(char_list[2]);
            closed_square_bracket.Length  = 1;

            static Expression opened_square_bracket;
            // opened_square_bracket.SetKeyword("[");
            opened_square_bracket.Keyword   = &(char_list[1]);
            opened_square_bracket.Length    = 1;
            opened_square_bracket.Connected = &closed_square_bracket;

            closed_square_bracket.NestExpres.SetCapacity(3);
            closed_square_bracket.NestExpres.Add(&opened_square_bracket).Add(&quotation_start).Add(&opened_curly_bracket);

            closed_curly_bracket.NestExpres.SetCapacity(3);
            closed_curly_bracket.NestExpres.Add(&opened_curly_bracket).Add(&quotation_start).Add(&opened_square_bracket);

            expres.Add(&opened_curly_bracket).Add(&opened_square_bracket);
        }

        return expres;
    }

    void operator+=(const Document &doc) noexcept {
        if (Ordered != doc.Ordered) {
            if (Entries.Size != 0) {
                // Not the same type!
                return;
            }
            Ordered = doc.Ordered;
        }

        const Entry *entry;

        if (Ordered) {
            for (UNumber i = 0; i < doc.Entries.Size; i++) {
                entry = &(doc.Entries[i]);

                switch (entry->Type) {
                    case VType::UndefinedT: {
                        break;
                    }
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
                    case VType::UndefinedT: {
                        break;
                    }
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
            if (Entries.Size != 0) {
                // Not the same type!
                return;
            }
            Ordered = doc.Ordered;
        }

        const Entry *entry;

        if (Ordered) {
            for (UNumber i = 0; i < doc.Entries.Size; i++) {
                entry = &(doc.Entries[i]);

                switch (entry->Type) {
                    case VType::UndefinedT: {
                        break;
                    }
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
                    case VType::UndefinedT: {
                        break;
                    }
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
            Insert(--LastKeyLen, VType::DocumentT, &doc, true);
            storage = &(Documents[Entries[LastKeyLen].ArrayID]);
        } else {
            if (LastKey != nullptr) {
                storage = &(Documents[Insert(LastKey, 0, LastKeyLen, VType::DocumentT, &doc, true)]);
            } else {
                String key_ = String::FromNumber(--LastKeyLen);
                storage     = &(Documents[Insert(key_.Str, 0, key_.Length, VType::DocumentT, &doc, true)]);
            }
        }

        LastKey    = nullptr;
        LastKeyLen = 0;

        return *storage;
    }

    Document &operator=(const Document &doc) noexcept {
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
            Insert(--LastKeyLen, VType::DocumentT, &doc_copy, true);
            storage = &(Documents[Entries[LastKeyLen].ArrayID]);
        } else {
            if (LastKey != nullptr) {
                storage = &(Documents[Insert(LastKey, 0, LastKeyLen, VType::DocumentT, &doc_copy, true)]);
            } else {
                String key_ = String::FromNumber(--LastKeyLen);
                storage     = &(Documents[Insert(key_.Str, 0, key_.Length, VType::DocumentT, &doc_copy, true)]);
            }
        }

        LastKey    = nullptr;
        LastKeyLen = 0;

        return *storage;
    }

    Document &operator=(const char *str) noexcept {
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
            if (LastKey != nullptr) {
                if (str != nullptr) {
                    String string = str;
                    Insert(LastKey, 0, LastKeyLen, VType::StringT, &string, true);
                } else {
                    Insert(LastKey, 0, LastKeyLen, VType::NullT, nullptr, false);
                }
            } else {
                String key_ = String::FromNumber(--LastKeyLen);
                if (str != nullptr) {
                    String string = str;
                    Insert(key_.Str, 0, key_.Length, VType::StringT, &string, true);
                } else {
                    Insert(key_.Str, 0, key_.Length, VType::NullT, nullptr, false);
                }
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
                if (LastKey != nullptr) {
                    Insert(LastKey, 0, LastKeyLen, VType::NumberT, &number, false);
                } else {
                    String key_ = String::FromNumber(--LastKeyLen);
                    Insert(key_.Str, 0, key_.Length, VType::NumberT, &number, false);
                }
            }

            LastKey    = nullptr;
            LastKeyLen = 0;
        }

        return *this;
    }

    Document &operator=(const int num) noexcept {
        if (LastKeyLen != 0) {
            double number = static_cast<double>(num);

            if (Ordered) {
                Insert(--LastKeyLen, VType::NumberT, &number, false);
            } else {
                if (LastKey != nullptr) {
                    Insert(LastKey, 0, LastKeyLen, VType::NumberT, &number, false);
                } else {
                    String key_ = String::FromNumber(--LastKeyLen);
                    Insert(key_.Str, 0, key_.Length, VType::NumberT, &number, false);
                }
            }

            LastKey    = nullptr;
            LastKeyLen = 0;
        }

        return *this;
    }

    Document &operator=(const long num) noexcept {
        if (LastKeyLen != 0) {
            double number = static_cast<double>(num);

            if (Ordered) {
                Insert(--LastKeyLen, VType::NumberT, &number, false);
            } else {
                if (LastKey != nullptr) {
                    Insert(LastKey, 0, LastKeyLen, VType::NumberT, &number, false);
                } else {
                    String key_ = String::FromNumber(--LastKeyLen);
                    Insert(key_.Str, 0, key_.Length, VType::NumberT, &number, false);
                }
            }

            LastKey    = nullptr;
            LastKeyLen = 0;
        }

        return *this;
    }

    Document &operator=(const bool value) noexcept {
        if (LastKeyLen != 0) {
            if (Ordered) {
                Insert(--LastKeyLen, (value ? VType::TrueT : VType::FalseT), nullptr, false);
            } else {
                if (LastKey != nullptr) {
                    Insert(LastKey, 0, LastKeyLen, (value ? VType::TrueT : VType::FalseT), nullptr, false);
                } else {
                    String key_ = String::FromNumber(--LastKeyLen);
                    Insert(key_.Str, 0, key_.Length, (value ? VType::TrueT : VType::FalseT), nullptr, false);
                }
            }

            LastKey    = nullptr;
            LastKeyLen = 0;
        }

        return *this;
    }

    void operator+=(const bool value) noexcept {
        if (!Ordered) {
            if (Entries.Size != 0) {
                return;
            }
            Ordered = true;
        }

        Entries += Entry({(value ? VType::TrueT : VType::FalseT), 0, 0});
        Numbers += value ? 1.0 : 0.0;
    }

    void operator+=(const int num) noexcept {
        if (!Ordered) {
            if (Entries.Size != 0) {
                return;
            }
            Ordered = true;
        }

        Entries += Entry({VType::NumberT, 0, Numbers.Size});
        Numbers += static_cast<double>(num);
    }

    void operator+=(const long num) noexcept {
        if (!Ordered) {
            if (Entries.Size != 0) {
                return;
            }
            Ordered = true;
        }

        Entries += Entry({VType::NumberT, 0, Numbers.Size});
        Numbers += static_cast<double>(num);
    }

    void operator+=(const double num) noexcept {
        if (!Ordered) {
            if (Entries.Size != 0) {
                return;
            }
            Ordered = true;
        }

        Entries += Entry({VType::NumberT, 0, Numbers.Size});
        Numbers += num;
    }

    void operator+=(const char *str) noexcept {
        if (!Ordered) {
            if (Entries.Size != 0) {
                return;
            }
            Ordered = true;
        }

        if (str != nullptr) {
            Entries += Entry({VType::StringT, 0, Strings.Size});
            Strings += str;
        } else {
            Entries += Entry({VType::NullT, 0, 0});
        }
    }

    void operator+=(const String &string) noexcept {
        if (!Ordered) {
            if (Entries.Size != 0) {
                return;
            }
            Ordered = true;
        }

        Entries += Entry({VType::StringT, 0, Strings.Size});
        Strings += string;
    }

    void operator+=(String &&string) noexcept {
        if (!Ordered) {
            if (Entries.Size != 0) {
                return;
            }
            Ordered = true;
        }

        Entries += Entry({VType::StringT, 0, Strings.Size});
        Strings += static_cast<String &&>(string);
    }

    void operator+=(const Array<double> &numbers) noexcept {
        if (!Ordered) {
            if (Entries.Size != 0) {
                return;
            }
            Ordered = true;
        }

        UNumber id = Numbers.Size;

        for (UNumber i = 0; i < numbers.Size; i++) {
            Entries += Entry({VType::NumberT, 0, id++});
        }

        Numbers += numbers;
    }

    void operator+=(const Array<String> &strings) noexcept {
        if (!Ordered) {
            if (Entries.Size != 0) {
                return;
            }
            Ordered = true;
        }

        UNumber id = Strings.Size;

        for (UNumber i = 0; i < strings.Size; i++) {
            Entries += Entry({VType::StringT, 0, id++});
        }

        Strings += strings;
    }

    void operator+=(const Array<Document> &documents) noexcept {
        if (!Ordered) {
            if (Entries.Size != 0) {
                return;
            }
            Ordered = true;
        }

        UNumber id = Documents.Size;

        for (UNumber i = 0; i < documents.Size; i++) {
            Entries += Entry({VType::DocumentT, 0, id++});
        }

        Documents += documents;
    }

    void operator+=(Array<double> &&numbers) noexcept {
        if (!Ordered) {
            if (Entries.Size != 0) {
                return;
            }
            Ordered = true;
        }

        UNumber id = Numbers.Size;

        for (UNumber i = 0; i < numbers.Size; i++) {
            Entries += Entry({VType::NumberT, 0, id++});
        }

        Numbers += numbers;
    }

    void operator+=(Array<String> &&strings) noexcept {
        if (!Ordered) {
            if (Entries.Size != 0) {
                return;
            }
            Ordered = true;
        }

        UNumber id = Strings.Size;

        for (UNumber i = 0; i < strings.Size; i++) {
            Entries += Entry({VType::StringT, 0, id++});
        }

        Strings += static_cast<Array<String> &&>(strings);
    }

    void operator+=(Array<Document> &&documents) noexcept {
        if (!Ordered) {
            if (Entries.Size != 0) {
                return;
            }
            Ordered = true;
        }

        UNumber id = Documents.Size;

        for (UNumber i = 0; i < documents.Size; i++) {
            Entries += Entry({VType::DocumentT, 0, id++});
        }

        Documents += static_cast<Array<Document> &&>(documents);
    }

    Document &operator[](const char *key) noexcept {
        const UNumber str_len = String::Count(key);

        if (Ordered) {
            UNumber id;
            if (String::ToNumber(id, key, 0, str_len)) {
                if (Entries[id].Type == VType::DocumentT) {
                    return Documents[Entries[id].ArrayID];
                }

                LastKeyLen = (id + 1);
            }
        } else {
            const Entry *entry = Exist(String::Hash(key, 0, str_len), 0, Table);

            if ((entry != nullptr) && (entry->Type == VType::DocumentT)) {
                return Documents[entry->ArrayID];
            }

            LastKey    = key;
            LastKeyLen = str_len;
        }

        return *this;
    }

    Document &operator[](const String &key) noexcept {
        if (Ordered) {
            UNumber id;
            if (String::ToNumber(id, key.Str, 0, key.Length)) {
                if (Entries[id].Type == VType::DocumentT) {
                    return Documents[Entries[id].ArrayID];
                }

                LastKeyLen = (id + 1);
            }
        } else {
            const Entry *entry = Exist(String::Hash(key.Str, 0, key.Length), 0, Table);

            if ((entry != nullptr) && (entry->Type == VType::DocumentT)) {
                return Documents[entry->ArrayID];
            }

            LastKey    = key.Str;
            LastKeyLen = key.Length;
        }

        return *this;
    }

    Document &operator[](const UNumber id) noexcept {
        if (id < Entries.Size) {
            const Entry &entry = Entries[id];

            if (entry.Type == VType::DocumentT) {
                return Documents[entry.ArrayID];
            }

            if (Ordered) {
                LastKeyLen = (id + 1);
            } else {
                LastKey    = Keys[entry.KeyID].Str;
                LastKeyLen = Keys[entry.KeyID].Length;
            }
        } else {
            if (Entries.Size == 0) {
                Ordered = true;
            }

            LastKeyLen = (id + 1);
        }

        return *this;
    }

    Document &operator[](const int id) noexcept {
        const UNumber ID = static_cast<UNumber>(id);

        if (ID < Entries.Size) {
            const Entry &entry = Entries[ID];

            if (entry.Type == VType::DocumentT) {
                return Documents[entry.ArrayID];
            }

            if (Ordered) {
                LastKeyLen = (ID + 1);
            } else {
                LastKey    = Keys[entry.KeyID].Str;
                LastKeyLen = Keys[entry.KeyID].Length;
            }
        } else {
            if (Entries.Size == 0) {
                Ordered = true;
            }

            LastKeyLen = (ID + 1);
        }

        return *this;
    }
};

} // namespace Qentem

#endif
