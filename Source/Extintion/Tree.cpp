
/**
 * Qentem Tree
 *
 * @brief     Ordered/Unordered array with hasing capability and JSON build-in
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Extintion/QRegex.hpp"
#include "Extintion/Tree.hpp"

using Qentem::Array;
using Qentem::Hash;
using Qentem::String;
using Qentem::StringStream;
using Qentem::Tree;
using Qentem::Engine::Expression;
using Qentem::Engine::Expressions;
using Qentem::Engine::Flags;

Expressions Tree::JsonQuot   = Expressions(); // See SetJsonQuot()
Expressions Tree::JsonDeQuot = Expressions();

void Qentem::Hash::Set(const Hash &_hash, const UNumber base, const UNumber level) noexcept {
    if (this->HashValue == 0) {
        this->HashValue = _hash.HashValue;
        this->ExactID   = _hash.ExactID;
        this->Key       = _hash.Key;
        this->Type      = _hash.Type;
        return;
    }

    if (this->Table.Size == 0) {
        this->Table.SetCapacity(base);
        this->Table.Size = base;
    }

    this->Table[((this->HashValue + level) % base)].Set(_hash, base, (level + 3));
}

Hash *Qentem::Hash::Get(const UNumber _hash_value, const UNumber base, const UNumber level) noexcept {
    if ((this->HashValue != _hash_value) && (this->Table.Size != 0)) {
        return this->Table[((this->HashValue + level) % base)].Get(_hash_value, base, (level + 3));
    }

    return this;
}

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

Qentem::Field &Qentem::Field::operator=(Tree &value) noexcept {
    if (this->Storage != nullptr) {
        this->Storage->Insert(this->Key, 0, this->Key.Length, VType::BranchT, &value, false);
    }
    return *this;
}

Qentem::Field &Qentem::Field::operator=(Tree &&value) noexcept {
    if (this->Storage != nullptr) {
        this->Storage->Insert(this->Key, 0, this->Key.Length, VType::BranchT, &value, true);
    }
    return *this;
}

Qentem::Field &Qentem::Field::operator=(Array<double> &value) noexcept {
    if (this->Storage != nullptr) {
        Tree tmp;
        tmp.Ordered = true;
        tmp.Numbers = value;
        this->Storage->Insert(this->Key, 0, this->Key.Length, VType::BranchT, &tmp, true);
    }
    return *this;
}

Qentem::Field &Qentem::Field::operator=(Array<String> &value) noexcept {
    if (this->Storage != nullptr) {
        Tree tmp;
        tmp.Ordered = true;
        tmp.Strings = value;
        this->Storage->Insert(this->Key, 0, this->Key.Length, VType::BranchT, &tmp, true);
    }
    return *this;
}

Qentem::Field &Qentem::Field::operator=(Array<Tree> &value) noexcept {
    if (this->Storage != nullptr) {
        Tree tmp;
        tmp.Ordered  = true;
        tmp.Branches = value;
        this->Storage->Insert(this->Key, 0, this->Key.Length, VType::BranchT, &tmp, true);
    }
    return *this;
}

Qentem::Field Qentem::Field::operator[](const String &key) noexcept {
    if (this->Storage != nullptr) {
        Tree *tree = this->Storage->GetBranch(this->Key, 0, this->Key.Length);

        if (tree != nullptr) {
            return (*tree)[key];
        }
    }

    return Field();
}
////////////////

void Qentem::Tree::Drop(Hash &_hash, Tree &storage) noexcept {
    _hash.HashValue = 0;

    switch (_hash.Type) {
        // case VType::NumberT:
        //     storage.Numbers[_hash.ExactID] = 0; // Waste of time
        //     break;
        case VType::StringT:
            storage.Strings[_hash.ExactID].Clear();
            break;
        case VType::BranchT:
            storage.Branches[_hash.ExactID] = Tree(); // TODO: Add Clear()
            break;
        default:
            break;
    }
}

void Qentem::Tree::Drop(const String &key, UNumber offset, UNumber limit) noexcept {
    Hash *_hash;
    Tree *storage = GetInfo(&_hash, key, offset, limit);

    if (_hash != nullptr) {
        Tree::Drop(*_hash, *storage);
    }
}

bool Qentem::Tree::GetID(UNumber &id, const String &key, UNumber offset, UNumber limit) noexcept {
    UNumber end = (offset + (limit - 1));

    while ((end > offset) && (key.Str[--end] != L'[')) {
    }
    end++;
    limit--;

    return String::ToNumber(key, id, end, (limit - (end - offset)));
}

// Key form can be: name, name[id1], name[id1][sub-id2], name[id1][sub-id2][sub-sub-idx]...
Tree *Qentem::Tree::GetInfo(Hash **hash, const String &key, UNumber offset, UNumber limit) noexcept {
    if (limit == 0) {
        limit = key.Length - offset;
    }

    UNumber end_offset = (offset + limit);
    UNumber _hash_value;

    if (key.Str[offset] == L'[') {
        offset++;
        end_offset = offset;

        while (key.Str[end_offset] != L']') {
            end_offset++;

            if (end_offset > (offset + limit)) {
                return nullptr;
            }
        }

        _hash_value = String::Hash(key, offset, end_offset);
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
        _hash_value = String::Hash(key, offset, end_offset);
    } else {
        _hash_value = String::Hash(key, offset, end_offset);
    }

    UNumber id = (_hash_value % HashBase);
    if (id < Table.Size) {
        *hash = (Table[id]).Get(_hash_value, HashBase, 1);
        if ((*hash)->HashValue != 0) {
            if ((*hash)->Type == VType::BranchT) {
                limit -= (end_offset - offset);
                Tree *tmp = &(this->Branches[(*hash)->ExactID]);

                if ((limit != 0) && !tmp->Ordered) {
                    return tmp->GetInfo(hash, key, end_offset, limit);
                }

                return tmp;
            }

            return this;
        }
    }

    *hash = nullptr;
    return nullptr;
}

bool Qentem::Tree::GetString(String &value, const String &key, UNumber offset, UNumber limit) noexcept {
    value.Clear();

    Hash *      _hash;
    const Tree *storage = GetInfo(&_hash, key, offset, limit);

    if (storage != nullptr) {
        if (!storage->Ordered) {
            if (_hash->Type == VType::StringT) {
                value = storage->Strings[_hash->ExactID];
                return true;
            }

            if (_hash->Type == VType::BooleanT) {
                value = ((storage->Numbers[_hash->ExactID] == 1.0) ? L"true" : L"false");
                return true;
            }

            if (_hash->Type == VType::NumberT) {
                value = String::FromNumber(storage->Numbers[_hash->ExactID]);
                return true;
            }
        } else {
            UNumber id = 0;
            storage->GetID(id, key, offset, limit);

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

bool Qentem::Tree::GetNumber(UNumber &value, const String &key, UNumber offset, UNumber limit) noexcept {
    value = 0;
    Hash *      _hash;
    const Tree *storage = GetInfo(&_hash, key, offset, limit);

    if (storage != nullptr) {
        if (!storage->Ordered) {
            if (_hash->Type == VType::StringT) {
                return String::ToNumber(storage->Strings[_hash->ExactID], value);
            }

            if (_hash->Type == VType::BooleanT) {
                value = ((storage->Numbers[_hash->ExactID] == 1.0) ? 1 : 0);
                return true;
            }

            if (_hash->Type == VType::NumberT) {
                value = static_cast<UNumber>(storage->Numbers[_hash->ExactID]);
                return true;
            }
        } else {
            UNumber id = 0;
            storage->GetID(id, key, offset, limit);

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

bool Qentem::Tree::GetDouble(double &value, const String &key, UNumber offset, UNumber limit) noexcept {
    value = 0.0;
    Hash *      _hash;
    const Tree *storage = GetInfo(&_hash, key, offset, limit);

    if (storage != nullptr) {
        if (!storage->Ordered) {
            if (_hash->Type == VType::StringT) {
                return String::ToNumber(storage->Strings[_hash->ExactID], value);
            }

            if (_hash->Type == VType::BooleanT) {
                value = ((storage->Numbers[_hash->ExactID] == 1.0) ? 1.0 : 0.0);
                return true;
            }

            if (_hash->Type == VType::NumberT) {
                value = storage->Numbers[_hash->ExactID];
                return true;
            }
        } else {
            UNumber id = 0;
            storage->GetID(id, key, offset, limit);

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

bool Qentem::Tree::GetBool(bool &value, const String &key, UNumber offset, UNumber limit) noexcept {
    Hash *      _hash;
    const Tree *storage = GetInfo(&_hash, key, offset, limit);

    if (storage != nullptr) {
        if (!storage->Ordered) {
            if ((_hash->Type == VType::BooleanT) || (_hash->Type == VType::NumberT)) {
                value = (storage->Numbers[_hash->ExactID] == 1);
                return true;
            }

            if (_hash->Type == VType::StringT) {
                value = (storage->Strings[_hash->ExactID] == L"true");
                return true;
            }
        } else {
            UNumber id = 0;
            storage->GetID(id, key, offset, limit);

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

Tree *Qentem::Tree::GetBranch(const String &key, UNumber offset, UNumber limit) noexcept {
    Hash *_hash;
    Tree *storage = GetInfo(&_hash, key, offset, limit);

    if ((storage != nullptr) && (_hash->Type == VType::BranchT)) {
        return storage;
    }

    return nullptr;
}

void Qentem::Tree::InsertHash(const Hash &_hash) noexcept {
    if (this->Table.Size == 0) {
        this->Table.SetCapacity(HashBase);
        this->Table.Size = HashBase;
    }

    // TODO: This is ok but slow. Needs complate rewrite/redesign
    this->Table[(_hash.HashValue % HashBase)].Set(_hash, HashBase, 1);
    Hashes.Add(_hash.HashValue); // TODO: Use pointers!
}

void Qentem::Tree::Insert(const String &key, UNumber offset, UNumber limit, const VType type, void *ptr,
                          const bool move) noexcept {
    Hash *  p_hash      = nullptr;
    UNumber _hash_value = String::Hash(key, offset, (offset + limit));
    UNumber id          = (_hash_value % HashBase);

    if (id < Table.Size) {
        p_hash = Table[id].Get(_hash_value, HashBase, 1);
        if ((p_hash->HashValue == _hash_value) && (p_hash->Type != type) && (p_hash->Type != VType::NullT)) {
            Tree::Drop(*p_hash, *this);
        }
    }

    if ((p_hash == nullptr) || (p_hash->HashValue != _hash_value)) {
        Hash _hash;
        _hash.HashValue = _hash_value;
        _hash.Type      = type;

        if (limit == key.Length) {
            _hash.Key = key;
        } else {
            _hash.Key = String::Part(key, offset, limit);
        }

        switch (type) {
            // case VType::NullT:
            //     break;
            case VType::BooleanT:
            case VType::NumberT: {
                _hash.ExactID = Numbers.Size;
                Numbers.Add(*(static_cast<double *>(ptr)));
            } break;
            case VType::StringT: {
                _hash.ExactID = Strings.Size;
                if (move) {
                    Strings.Add(static_cast<String &&>(*(static_cast<String *>(ptr))));
                } else {
                    Strings.Add(*(static_cast<String *>(ptr)));
                }
            } break;
            case VType::BranchT: {
                _hash.ExactID = Branches.Size;
                if (move) {
                    Branches.Add(static_cast<Tree &&>(*(static_cast<Tree *>(ptr))));
                } else {
                    Branches.Add(*(static_cast<Tree *>(ptr)));
                }
            } break;
            default:
                break;
        }

        InsertHash(_hash);
    } else { // Updating existing item
        // TODO: implement move
        switch (type) {
            // case VType::NullT:
            //     break;
            case VType::BooleanT:
            case VType::NumberT: {
                Numbers[p_hash->ExactID] = *(static_cast<double *>(ptr));
            } break;
            case VType::StringT: {
                Strings[p_hash->ExactID] = *(static_cast<String *>(ptr));
            } break;
            case VType::BranchT: {
                Branches[p_hash->ExactID] = *(static_cast<Tree *>(ptr));
            } break;
            default:
                break;
        }
    }
}

void Qentem::Tree::_makeNumberedTree(Tree &tree, const String &content, const Match &item) noexcept {
    double  tn;
    UNumber end;
    UNumber start = (item.Offset + 1);
    UNumber to    = (item.Offset + item.Length);

    for (UNumber i = start; i < to; i++) {
        if ((content.Str[i] == L',') || (content.Str[i] == L']')) {
            end = i;
            String::SoftTrim(content, start, end);

            if (String::ToNumber(content, tn, start, ((end + 1) - start))) {
                tree.Numbers.Add(tn);
            }

            start = end + 2;
        }
    }
}

void Qentem::Tree::_makeTree(Tree &tree, const String &content, const Array<Match> &items) noexcept {
    if (!tree.Ordered) {
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

                        tree.Insert(content, (key->Offset + 1), (key->Length - 2), VType::StringT, &ts, true);
                        done = true;
                    } break;
                    case L'{': {
                        i++;

                        Tree uno_tree = Tree();
                        Tree::_makeTree(uno_tree, content, items[i].NestMatch);
                        tree.Insert(content, (key->Offset + 1), (key->Length - 2), VType::BranchT, &uno_tree, true);

                        done = true;
                    } break;
                    case L'[': {
                        i++;

                        Tree o_tree    = Tree();
                        o_tree.Ordered = true;
                        Match *data    = &(items[i]);

                        if (items[i].NestMatch.Size > 0) {
                            Tree::_makeTree(o_tree, content, data->NestMatch);
                        } else {
                            Tree::_makeNumberedTree(o_tree, content, *data);
                        }

                        tree.Insert(content, (key->Offset + 1), (key->Length - 2), VType::BranchT, &o_tree, true);
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
                            tree.Insert(content, (key->Offset + 1), (key->Length - 2), VType::BooleanT, &tn, false);
                        } else if (content.Str[start] == L'f') {
                            // False
                            tn = 0;
                            tree.Insert(content, (key->Offset + 1), (key->Length - 2), VType::BooleanT, &tn, false);
                        } else if (content.Str[start] == L'n') {
                            // Nullcontent.Str[i]
                            tree.Insert(content, (key->Offset + 1), (key->Length - 2), VType::NullT, nullptr, false);
                        } else if (String::ToNumber(content, tn, start, ((end + 1) - start))) {
                            // Number
                            tree.Insert(content, (key->Offset + 1), (key->Length - 2), VType::NumberT, &tn, false);
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
                tree.Strings.SetCapacity(items.Size);

                Match *data;
                for (UNumber i = 0; i < items.Size; i++) {
                    data = &(items[i]);
                    if (data->NestMatch.Size == 0) {
                        tree.Strings.Add(String::Part(content, (data->Offset + 1), (data->Length - 2)));
                    } else {
                        // TODO: Do cleanup this part.
                        String *rs = &(tree.Strings[tree.Strings.Size]);
                        tree.Strings.Size++;

                        *rs = String::Part(content, (data->Offset + 1), (data->Length - 2));
                        Engine::Parse(*rs, Engine::Search(*rs, JsonDeQuot));
                    }
                }
            } else if (items[0].Expr->Keyword == L'}') { // Unordered arrays
                tree.Branches.SetCapacity(items.Size);

                for (UNumber i = 0; i < items.Size; i++) {
                    Tree *te = &(tree.Branches[tree.Branches.Size]);
                    tree.Branches.Size++;

                    _makeTree(*te, content, items[i].NestMatch);
                }
            } else if (items[0].Expr->Keyword == L']') { // Ordered arrays
                Match *data;
                tree.Branches.SetCapacity(items.Size);

                for (UNumber i = 0; i < items.Size; i++) {
                    Tree *te = &(tree.Branches[tree.Branches.Size]);
                    tree.Branches.Size++;
                    te->Ordered = true;

                    data = &(items[i]);
                    if (data->NestMatch.Size > 0) {
                        Tree::_makeTree(*te, content, data->NestMatch);
                    } else {
                        Tree::_makeNumberedTree(*te, content, *data);
                    }
                }
            }
        } else {
            Tree::_makeNumberedTree(tree, content, items[0]);
        }
    }
}

void Qentem::Tree::MakeTree(Tree &tree, const String &content, const Array<Match> &items) noexcept {
    if (items.Size != 0) {
        Match *_item = &(items[0]);
        if (_item->Expr->Keyword == L']') {
            tree.Ordered = true;
        }

        if (_item->NestMatch.Size != 0) {
            Tree::_makeTree(tree, content, _item->NestMatch);
        } else if (tree.Ordered) {
            // Might be: true, false, null or a number.
            // [1,2,3]
            Tree::_makeNumberedTree(tree, content, *_item);
        }
    }
}

String Qentem::Tree::ToJSON() const noexcept {
    Tree::SetJsonQuot();

    return Tree::_ToJSON().Eject();
}

StringStream Qentem::Tree::_ToJSON() const noexcept {
    StringStream ss;

    Hash *  _hash;
    UNumber id;

    if (!Ordered) {
        ss += L'{';
        for (UNumber i = 0; i < Hashes.Size; i++) {
            id    = Hashes[i];
            _hash = Table[(id % HashBase)].Get(id, HashBase, 1);

            if (_hash->HashValue != 0) {
                if (ss.Length != 1) {
                    ss += L',';
                }

                switch (_hash->Type) {
                    case VType::NullT: {
                        ss += L'"';
                        ss += _hash->Key;
                        ss += L"\":null"; // TODO: implement share(); to have a pointer to this inseat of copying it.
                    } break;
                    case VType::BooleanT: {
                        ss += L'"';
                        ss += _hash->Key;

                        if (Numbers[_hash->ExactID] != 0) {
                            ss += L"\":true";
                        } else {
                            ss += L"\":false";
                        }
                    } break;
                    case VType::StringT: {
                        ss += L'"';
                        ss += _hash->Key;
                        ss += L"\":\"";
                        ss += Engine::Parse(Strings[_hash->ExactID], Engine::Search(Strings[_hash->ExactID], JsonQuot));
                        ss += L'"';
                    } break;
                    case VType::NumberT: {
                        ss += L'"';
                        ss += _hash->Key;
                        ss += L"\":";
                        ss += String::FromNumber(Numbers[_hash->ExactID]);
                    } break;
                    case VType::BranchT: {
                        ss += L'"';
                        ss += _hash->Key;
                        ss += L"\":";
                        ss += Branches[_hash->ExactID]._ToJSON().Eject();
                    } break;
                    default:
                        break;
                }
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
        } else if (Branches.Size != 0) {
            for (UNumber i = 0; i < Branches.Size; i++) {
                if (ss.Length != 1) {
                    ss += L',';
                }

                ss += Branches[i]._ToJSON().Eject();
            }
        }

        ss += L']';
    }

    return ss;
}

Expressions Qentem::Tree::GetJsonExpres() noexcept {
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

Tree Qentem::Tree::FromJSON(const String &content) noexcept {
    Tree tree;
    // Engine::Search(content, GetJsonExpres());
    Tree::MakeTree(tree, content, Engine::Search(content, GetJsonExpres()));
    return tree;
}
