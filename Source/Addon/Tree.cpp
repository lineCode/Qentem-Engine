
/**
 * Qentem Tree
 *
 * @brief     Ordered Array With Hasing capability And JSON build-in
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Addon/QRegex.hpp"
#include "Addon/Tree.hpp"

using Qentem::Array;
using Qentem::Hash;
using Qentem::String;
using Qentem::Tree;
using Qentem::Engine::Expression;
using Qentem::Engine::Expressions;
using Qentem::Engine::Flags;

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

Qentem::Field &Qentem::Field::operator=(const double value) noexcept {
    if (this->Storage != nullptr) {
        this->Storage->Set(this->Key, value, 0, this->Key.Length);
    }
    return *this;
}

Qentem::Field &Qentem::Field::operator=(const Array<double> &value) noexcept {
    if (this->Storage != nullptr) {
        this->Storage->Set(this->Key, value, 0, this->Key.Length);
    }
    return *this;
}

Qentem::Field &Qentem::Field::operator=(const String &value) noexcept {
    if (this->Storage != nullptr) {
        this->Storage->Set(this->Key, value, 0, this->Key.Length);
    }
    return *this;
}

Qentem::Field &Qentem::Field::operator=(const wchar_t *value) noexcept {
    if (this->Storage != nullptr) {
        if (value != nullptr) {
            this->Storage->Set(this->Key, String(value), 0, this->Key.Length);
        } else {
            this->Storage->Set(this->Key, 0, this->Key.Length);
        }
    }
    return *this;
}

Qentem::Field &Qentem::Field::operator=(const Array<String> &value) noexcept {
    if (this->Storage != nullptr) {
        this->Storage->Set(this->Key, value, 0, this->Key.Length);
    }
    return *this;
}

Qentem::Field &Qentem::Field::operator=(const Tree &value) noexcept {
    if (this->Storage != nullptr) {
        this->Storage->Set(this->Key, value, 0, this->Key.Length);
    }
    return *this;
}

Qentem::Field &Qentem::Field::operator=(const Array<Tree> &value) noexcept {
    if (this->Storage != nullptr) {
        this->Storage->Set(this->Key, value, 0, this->Key.Length);
    }
    return *this;
}

Qentem::Field &Qentem::Field::operator=(const Field &_field) noexcept {
    if (this->Storage != nullptr) {
        Tree *tree = _field.Storage->GetChild(_field.Key);
        if (tree != nullptr) {
            this->Storage->Set(this->Key, *tree, 0, this->Key.Length);
        }
    }

    return *this;
}

Qentem::Field &Qentem::Field::operator=(const bool value) noexcept {
    if (this->Storage != nullptr) {
        this->Storage->Set(this->Key, value, 0, this->Key.Length);
    }
    return *this;
}

Qentem::Field Qentem::Field::operator[](const String &key) noexcept {
    if (this->Storage != nullptr) {
        Tree *tree = this->Storage->GetChild(this->Key, 0, this->Key.Length);

        if (tree != nullptr) {
            return (*tree)[key];
        }
        // else {
        //     this->Storage->Set(this->Key, Tree());
        //     Field _field;
        //     _field.Key     = key;
        //     _field.Storage = &(this->Storage->Child[(this->Storage->Child.Size - 1)]);
        //     return _field;
        // }
    }

    return Field();
}

void Qentem::Tree::InsertHash(const Hash &_hash) noexcept {
    if (this->Table.Size == 0) {
        this->Table.SetCapacity(HashBase);
        this->Table.Size = HashBase;
    }

    this->Table[(_hash.HashValue % HashBase)].Set(_hash, HashBase, 1);
    Hashes.Add(_hash.HashValue);
}

void Qentem::Tree::Drop(Hash &_hash, Tree &storage) noexcept {
    _hash.HashValue = 0;

    switch (_hash.Type) {
        // case VType::NumberT:
        //     storage.Numbers[_hash.ExactID] = 0; // Waste of time
        //     break;
        case VType::ONumbersT:
            storage.ONumbers[_hash.ExactID].Clear();
            break;
        case VType::StringT:
            storage.Strings[_hash.ExactID].Clear();
            break;
        case VType::OStringsT:
            storage.OStrings[_hash.ExactID].Clear();
            break;
        case VType::ChildT:
            storage.Child[_hash.ExactID] = Tree();
            break;
        case VType::OChildrenT:
            storage.OChildren[_hash.ExactID].Clear();
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

void Qentem::Tree::Set(const String &key, UNumber offset, UNumber limit) noexcept {
    Hash *  p_hash      = nullptr;
    UNumber _hash_value = String::Hash(key, offset, (offset + limit));
    UNumber id          = (_hash_value % HashBase);

    if (id < Table.Size) {
        p_hash = Table[id].Get(_hash_value, HashBase, 1);
        if ((p_hash->HashValue == _hash_value) && (p_hash->Type != VType::NullT)) {
            Tree::Drop(*p_hash, *this);
        }
    }

    if ((p_hash == nullptr) || (p_hash->HashValue != _hash_value)) {
        Hash _hash;
        _hash.ExactID   = Numbers.Size;
        _hash.HashValue = _hash_value;

        if (limit == key.Length) {
            _hash.Key = key;
        } else {
            _hash.Key = String::Part(key, offset, limit);
        }

        _hash.Type = VType::NullT;

        InsertHash(_hash);
    }
}

void Qentem::Tree::Set(const String &key, double value, UNumber offset, UNumber limit) noexcept {
    Hash *  p_hash      = nullptr;
    UNumber _hash_value = String::Hash(key, offset, (offset + limit));
    UNumber id          = (_hash_value % HashBase);

    if (id < Table.Size) {
        p_hash = Table[id].Get(_hash_value, HashBase, 1);
        if ((p_hash->HashValue == _hash_value) && (p_hash->Type != VType::NumberT)) {
            Tree::Drop(*p_hash, *this);
        }
    }

    if ((p_hash == nullptr) || (p_hash->HashValue != _hash_value)) {
        Hash _hash;
        _hash.ExactID   = Numbers.Size;
        _hash.HashValue = _hash_value;

        if (limit == key.Length) {
            _hash.Key = key;
        } else {
            _hash.Key = String::Part(key, offset, limit);
        }

        _hash.Type = VType::NumberT;

        InsertHash(_hash);
        Numbers.Add(value);
    } else {
        Numbers[p_hash->ExactID] = value;
    }
}

void Qentem::Tree::Set(const String &key, const Array<double> &value, UNumber offset, UNumber limit) noexcept {
    Hash *  p_hash      = nullptr;
    UNumber _hash_value = String::Hash(key, offset, (offset + limit));
    UNumber id          = (_hash_value % HashBase);

    if (id < Table.Size) {
        p_hash = Table[id].Get(_hash_value, HashBase, 1);
        if ((p_hash->HashValue == _hash_value) && (p_hash->Type != VType::ONumbersT)) {
            Tree::Drop(*p_hash, *this);
        }
    }

    if ((p_hash == nullptr) || (p_hash->HashValue != _hash_value)) {
        Hash _hash;
        _hash.ExactID   = ONumbers.Size;
        _hash.HashValue = _hash_value;

        if (limit == key.Length) {
            _hash.Key = key;
        } else {
            _hash.Key = String::Part(key, offset, limit);
        }

        _hash.Type = VType::ONumbersT;

        InsertHash(_hash);
        ONumbers.Add(value);
    } else {
        ONumbers[p_hash->ExactID] = value;
    }
}

void Qentem::Tree::Set(const String &key, const String &value, UNumber offset, UNumber limit) noexcept {
    Hash *  p_hash      = nullptr;
    UNumber _hash_value = String::Hash(key, offset, (offset + limit));
    UNumber id          = (_hash_value % HashBase);

    if (id < Table.Size) {
        p_hash = Table[id].Get(_hash_value, HashBase, 1);
        if ((p_hash->HashValue == _hash_value) && (p_hash->Type != VType::StringT)) {
            Tree::Drop(*p_hash, *this);
        }
    }

    if ((p_hash == nullptr) || (p_hash->HashValue != _hash_value)) {
        Hash _hash;
        _hash.ExactID   = Strings.Size;
        _hash.HashValue = _hash_value;

        if (limit == key.Length) {
            _hash.Key = key;
        } else {
            _hash.Key = String::Part(key, offset, limit);
        }

        _hash.Type = VType::StringT;
        InsertHash(_hash);
        Strings.Add(value);
    } else {
        Strings[p_hash->ExactID] = value;
    }
}

void Qentem::Tree::Set(const String &key, const Array<String> &value, UNumber offset, UNumber limit) noexcept {
    Hash *  p_hash      = nullptr;
    UNumber _hash_value = String::Hash(key, offset, (offset + limit));
    UNumber id          = (_hash_value % HashBase);

    if (id < Table.Size) {
        p_hash = Table[id].Get(_hash_value, HashBase, 1);
        if ((p_hash->HashValue == _hash_value) && (p_hash->Type != VType::OStringsT)) {
            Tree::Drop(*p_hash, *this);
        }
    }

    if ((p_hash == nullptr) || (p_hash->HashValue != _hash_value)) {
        Hash _hash;
        _hash.ExactID   = OStrings.Size;
        _hash.HashValue = _hash_value;

        if (limit == key.Length) {
            _hash.Key = key;
        } else {
            _hash.Key = String::Part(key, offset, limit);
        }

        _hash.Type = VType::OStringsT;

        InsertHash(_hash);
        OStrings.Add(value);
    } else {
        OStrings[p_hash->ExactID] = value;
    }
}

void Qentem::Tree::Set(const String &key, const Tree &value, UNumber offset, UNumber limit) noexcept {
    Hash *  p_hash      = nullptr;
    UNumber _hash_value = String::Hash(key, offset, (offset + limit));
    UNumber id          = (_hash_value % HashBase);

    if (id < Table.Size) {
        p_hash = Table[id].Get(_hash_value, HashBase, 1);
        if ((p_hash->HashValue == _hash_value) && (p_hash->Type != VType::ChildT)) {
            Tree::Drop(*p_hash, *this);
        }
    }

    if ((p_hash == nullptr) || (p_hash->HashValue != _hash_value)) {
        Hash _hash;
        _hash.ExactID   = Child.Size;
        _hash.HashValue = _hash_value;

        if (limit == key.Length) {
            _hash.Key = key;
        } else {
            _hash.Key = String::Part(key, offset, limit);
        }

        _hash.Type = VType::ChildT;

        InsertHash(_hash);
        Child.Add(value);
    } else {
        Child[p_hash->ExactID] = value;
    }
}

void Qentem::Tree::Set(const String &key, const Array<Tree> &value, UNumber offset, UNumber limit) noexcept {
    Hash *  p_hash      = nullptr;
    UNumber _hash_value = String::Hash(key, offset, (offset + limit));
    UNumber id          = (_hash_value % HashBase);

    if (id < Table.Size) {
        p_hash = Table[id].Get(_hash_value, HashBase, 1);
        if ((p_hash->HashValue == _hash_value) && (p_hash->Type != VType::OChildrenT)) {
            Tree::Drop(*p_hash, *this);
        }
    }

    if ((p_hash == nullptr) || (p_hash->HashValue != _hash_value)) {
        Hash _hash;
        _hash.ExactID   = OChildren.Size;
        _hash.HashValue = _hash_value;

        if (limit == key.Length) {
            _hash.Key = key;
        } else {
            _hash.Key = String::Part(key, offset, limit);
        }

        _hash.Type = VType::OChildrenT;

        InsertHash(_hash);
        OChildren.Add(value);
    } else {
        OChildren[p_hash->ExactID] = value;
    }
}

void Qentem::Tree::Set(const String &key, const bool value, UNumber offset, UNumber limit) noexcept {
    Hash *  p_hash      = nullptr;
    UNumber _hash_value = String::Hash(key, offset, (offset + limit));
    UNumber id          = (_hash_value % HashBase);

    if (id < Table.Size) {
        p_hash = Table[id].Get(_hash_value, HashBase, 1);
        if ((p_hash->HashValue == _hash_value) && (p_hash->Type != VType::BooleanT)) {
            Tree::Drop(*p_hash, *this);
        }
    }

    if ((p_hash == nullptr) || (p_hash->HashValue != _hash_value)) {
        Hash _hash;
        _hash.ExactID   = Numbers.Size;
        _hash.HashValue = _hash_value;

        if (limit == key.Length) {
            _hash.Key = key;
        } else {
            _hash.Key = String::Part(key, offset, limit);
        }

        _hash.Type = VType::BooleanT;

        InsertHash(_hash);
        Numbers.Add((value ? 1.0 : 0.0));
    } else {
        Numbers[p_hash->ExactID] = (value ? 1.0 : 0.0);
    }
}

const bool Qentem::Tree::GetID(UNumber &id, const String &key, UNumber offset, UNumber limit) noexcept {
    UNumber end = (offset + (limit - 1));

    while ((end > offset) && (key.Str[--end] != L'[')) {
    }
    end++;
    limit--;

    return String::ToNumber(key, id, end, (limit - (end - offset)));
}

Tree *Qentem::Tree::GetChild(const String &key, UNumber offset, UNumber limit) const noexcept {
    Hash *_hash;
    Tree *storage = GetInfo(&_hash, key, offset, limit);

    if ((storage != nullptr) && (_hash->Type == VType::ChildT)) {
        return storage;
    }

    return nullptr;
}

// Key form can be: name, name[id1], name[id1][id2], name[id1][id2][idx]...
Tree *Qentem::Tree::GetInfo(Hash **hash, const String &key, UNumber offset, UNumber limit) const noexcept {
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
            if ((*hash)->Type == VType::ChildT) {
                limit -= (end_offset - offset);

                if (limit != 0) {
                    return this->Child[(*hash)->ExactID].GetInfo(hash, key, end_offset, limit);
                }

                return &(this->Child[(*hash)->ExactID]);
            }

            return const_cast<Tree *>(this);
        }
    }

    *hash = nullptr;
    return nullptr;
}

Array<String> *Qentem::Tree::GetOStrings(const String &key, UNumber offset, UNumber limit) const noexcept {
    Hash *      _hash;
    const Tree *storage = GetInfo(&_hash, key, offset, limit);

    if ((storage != nullptr) && (_hash->Type == VType::OStringsT)) {
        return &(storage->OStrings[_hash->ExactID]);
    }

    return nullptr;
}

const bool Qentem::Tree::GetString(String &value, const String &key, UNumber offset, UNumber limit) const noexcept {
    Hash *      _hash;
    const Tree *storage = GetInfo(&_hash, key, offset, limit);

    if (storage != nullptr) {
        if (_hash->Type == VType::StringT) {
            value = storage->Strings[_hash->ExactID];
            return true;
        }

        if (_hash->Type == VType::BooleanT) {
            value = ((storage->Numbers[_hash->ExactID] == 1.0) ? L"true" : L"false");
            return true;
        }

        UNumber id;
        if (_hash->Type == VType::OStringsT) {
            if (Tree::GetID(id, key, offset, limit)) {
                if (id < storage->OStrings[_hash->ExactID].Size) {
                    value = storage->OStrings[_hash->ExactID][id];
                    return true;
                }
            }
        } else if (_hash->Type == VType::ONumbersT) {
            if (Tree::GetID(id, key, offset, limit)) {
                if (id < storage->ONumbers[_hash->ExactID].Size) {
                    value = String::FromNumber(storage->ONumbers[_hash->ExactID][id]);
                    return true;
                }
            }
        } else if (_hash->Type == VType::NumberT) {
            value = String::FromNumber(storage->Numbers[_hash->ExactID]);
            return true;
        }
    }

    return false;
}

const bool Qentem::Tree::GetNumber(UNumber &value, const String &key, UNumber offset, UNumber limit) const noexcept {
    Hash *      _hash;
    const Tree *storage = GetInfo(&_hash, key, offset, limit);

    if (storage != nullptr) {
        if (_hash->Type == VType::StringT) {
            return String::ToNumber(storage->Strings[_hash->ExactID], value);
        }

        if (_hash->Type == VType::BooleanT) {
            value = ((storage->Numbers[_hash->ExactID] == 1.0) ? 1 : 0);
            return true;
        }

        UNumber id;
        if (_hash->Type == VType::OStringsT) {
            if (Tree::GetID(id, key, offset, limit)) {
                if (id < storage->OStrings[_hash->ExactID].Size) {
                    String::ToNumber(storage->OStrings[_hash->ExactID][id], value);
                    return true;
                }
            }
        } else if (_hash->Type == VType::ONumbersT) {
            if (Tree::GetID(id, key, offset, limit)) {
                if (id < storage->ONumbers[_hash->ExactID].Size) {
                    value = static_cast<UNumber>(storage->ONumbers[_hash->ExactID][id]);
                    return true;
                }
            }
        } else if (_hash->Type == VType::NumberT) {
            value = static_cast<UNumber>(storage->Numbers[_hash->ExactID]);
            return true;
        }
    }

    return false;
}

const bool Qentem::Tree::GetDouble(double &value, const String &key, UNumber offset, UNumber limit) const noexcept {
    Hash *      _hash;
    const Tree *storage = GetInfo(&_hash, key, offset, limit);

    if (storage != nullptr) {
        if (_hash->Type == VType::StringT) {
            return String::ToNumber(storage->Strings[_hash->ExactID], value);
        }

        if (_hash->Type == VType::BooleanT) {
            value = ((storage->Numbers[_hash->ExactID] == 1.0) ? 1.0 : 0.0);
            return true;
        }

        UNumber id;
        if (_hash->Type == VType::OStringsT) {
            if (Tree::GetID(id, key, offset, limit)) {
                if (id < storage->OStrings[_hash->ExactID].Size) {
                    String::ToNumber(storage->OStrings[_hash->ExactID][id], value);
                    return true;
                }
            }
        } else if (_hash->Type == VType::ONumbersT) {
            if (Tree::GetID(id, key, offset, limit)) {
                if (id < storage->ONumbers[_hash->ExactID].Size) {
                    value = storage->ONumbers[_hash->ExactID][id];
                    return true;
                }
            }
        } else if (_hash->Type == VType::NumberT) {
            value = storage->Numbers[_hash->ExactID];
            return true;
        }
    }

    return false;
}

const bool Qentem::Tree::GetBool(bool &value, const String &key, UNumber offset, UNumber limit) const noexcept {
    Hash *      _hash;
    const Tree *storage = GetInfo(&_hash, key, offset, limit);

    if (storage != nullptr) {
        if ((_hash->Type == VType::BooleanT) || (_hash->Type == VType::NumberT)) {
            value = (storage->Numbers[_hash->ExactID] == 1);
            return true;
        }

        if (_hash->Type == VType::StringT) {
            value = (storage->Strings[_hash->ExactID] == L"true");
            return true;
        }
    }

    return false;
}

String Qentem::Tree::ToJSON() const noexcept {
    StringStream ss;
    bool         flag = false;

    Hash *  _hash;
    UNumber id;

    ss += L'{';
    for (UNumber i = 0; i < Hashes.Size; i++) {
        id    = Hashes[i];
        _hash = Table[(id % HashBase)].Get(id, HashBase, 1);

        if (_hash->HashValue != 0) {
            if (flag) {
                ss += L',';
            }

            switch (_hash->Type) {
                case VType::NullT: {
                    ss += L'"';
                    ss += _hash->Key;
                    ss += L"\":null";
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
                    ss += QRegex::Replace(Strings[_hash->ExactID], L"\"", L"\\\"");
                    ss += L'"';
                } break;
                case VType::NumberT: {
                    ss += L'"';
                    ss += _hash->Key;
                    ss += L"\":";
                    ss += String::FromNumber(Numbers[_hash->ExactID]);
                } break;
                case VType::ChildT: {
                    ss += L'"';
                    ss += _hash->Key;
                    ss += L"\":";
                    ss += Child[_hash->ExactID].ToJSON();
                } break;
                case VType::OStringsT: {
                    ss += L'"';
                    ss += _hash->Key;
                    ss += L"\":[";

                    Array<String> *ars = &(OStrings[_hash->ExactID]);
                    for (UNumber j = 0; j < ars->Size; j++) {
                        if (j > 0) {
                            ss += L',';
                        }
                        ss += L'"';
                        ss += QRegex::Replace(ars->operator[](j), L"\"", L"\\\"");
                        ss += L'"';
                    }
                    ss += L']';
                } break;
                case VType::ONumbersT: {
                    ss += L'"';
                    ss += _hash->Key;
                    ss += L"\":[";

                    Array<double> *ars = &(ONumbers[_hash->ExactID]);
                    for (UNumber j = 0; j < ars->Size; j++) {
                        if (j > 0) {
                            ss += L',';
                        }
                        ss += String::FromNumber(ars->operator[](j));
                    }
                    ss += L']';
                } break;
                case VType::OChildrenT: {
                    ss += L'"';
                    ss += _hash->Key;
                    ss += L"\":[";

                    Array<Tree> *ars = &(OChildren[_hash->ExactID]);
                    for (UNumber j = 0; j < ars->Size; j++) {
                        if (j > 0) {
                            ss += L',';
                        }
                        ss += ars->operator[](j).ToJSON();
                    }
                    ss += L']';
                } break;
                default:
                    break;
            }

            flag = true;
        }
    }

    ss += L'}';

    return ss.Eject();
}

Tree Qentem::Tree::MakeTree(const String &content, const Array<Match> &items) noexcept {
    Tree tree = Tree();

    if ((items.Size == 0) || (items[0].NestMatch.Size == 0)) {
        return tree;
    }

    Match * json = &(items[0]);
    Match * key;
    Match * data;
    UNumber data_id;
    wchar_t ch;
    double  number;

    for (UNumber i = 0; i < json->NestMatch.Size; (i += 2)) {
        key     = &(json->NestMatch[i]);
        data_id = (i + 1);
        if (data_id != json->NestMatch.Size) {
            data = &(json->NestMatch[data_id]);

            if (key->NestMatch.Size != 0) {
                if ((key->Length - 2) != 0) {
                    if (data->NestMatch.Size == 0) {
                        // Number or true, false, null
                        ch = content.Str[data->Offset];
                        if ((data->Length <= 5) && ((ch == L't') || (ch == L'f') || (ch == L'n'))) {
                            if (ch == L'n') {
                                // Null
                                tree.Set(content, (key->NestMatch[0].Offset + 1), (key->NestMatch[0].Length - 2));
                            } else {
                                // True, False
                                tree.Set(content, (ch == L't'), (key->NestMatch[0].Offset + 1),
                                         (key->NestMatch[0].Length - 2));
                            }
                        } else if (String::ToNumber(content, number, data->Offset, data->Length)) {
                            // Number
                            tree.Set(content, number, (key->NestMatch[0].Offset + 1), (key->NestMatch[0].Length - 2));
                        } else {
                            // Error converting number.
                        }
                    } else if (data->NestMatch.Size == 1) {
                        if (data->NestMatch[0].Expr->Keyword == L']') {
                            // Ordered array []
                            Match *ns = &(data->NestMatch[0]);

                            if (ns->NestMatch.Size == 0) {
                                // Only one ordered number.
                                if (String::ToNumber(content, number, (ns->Offset + ns->OLength),
                                                     (ns->Length - (ns->OLength + ns->CLength)))) {
                                    tree.Set(content, Array<double>().Add(number), (key->NestMatch[0].Offset + 1),
                                             (key->NestMatch[0].Length - 2));
                                } else {
                                    // Error converting number.
                                }
                            } else if (ns->NestMatch[0].Expr->Keyword == L'}') {
                                // Only one ordered object.
                                tree.Set(content, Array<Tree>().Add(Tree::MakeTree(content, ns->NestMatch)),
                                         (key->NestMatch[0].Offset + 1), (key->NestMatch[0].Length - 2));
                            } else if (ns->NestMatch.Size == 1) {
                                // Only one ordered string.
                                tree.Set(content,
                                         Array<String>().Add(String::Part(content, (ns->NestMatch[0].Offset + 1),
                                                                          (ns->NestMatch[0].Length - 2))),
                                         (key->NestMatch[0].Offset + 1), (key->NestMatch[0].Length - 2));

                            } else if ((ns->NestMatch[0].NestMatch.Size != 0) &&
                                       ns->NestMatch[0].NestMatch[0].Expr->Keyword == L'}') {
                                // Objects {}
                                tree.Set(content, Array<Tree>(), (key->NestMatch[0].Offset + 1),
                                         (key->NestMatch[0].Length - 2));

                                Array<Tree> *ts = &(tree.OChildren[(tree.OChildren.Size - 1)]);
                                ts->SetCapacity(ns->NestMatch.Size);

                                for (UNumber j = 0; j < ns->NestMatch.Size; j++) {
                                    ts->Add(Tree::MakeTree(content, ns->NestMatch[j].NestMatch));
                                }
                            } else if (ns->NestMatch[0].NestMatch.Size == 1) {
                                // Strings
                                tree.Set(content, Array<String>(), (key->NestMatch[0].Offset + 1),
                                         (key->NestMatch[0].Length - 2));
                                Array<String> *st = &(tree.OStrings[(tree.OStrings.Size - 1)]);
                                st->SetCapacity(ns->NestMatch.Size);

                                for (UNumber j = 0; j < ns->NestMatch.Size; j++) {
                                    st->Add(String::Part(content, (ns->NestMatch[j].NestMatch[0].Offset + 1),
                                                         (ns->NestMatch[j].NestMatch[0].Length - 2)));
                                }
                            } else {
                                // Numbers
                                tree.Set(content, Array<double>(), (key->NestMatch[0].Offset + 1),
                                         (key->NestMatch[0].Length - 2));

                                Array<double> *nt = &(tree.ONumbers[(tree.ONumbers.Size - 1)]);
                                nt->SetCapacity(ns->NestMatch.Size);

                                for (UNumber j = 0; j < ns->NestMatch.Size; j++) {
                                    if (String::ToNumber(content, number, ns->NestMatch[j].Offset,
                                                         ns->NestMatch[j].Length)) {
                                        nt->Add(number);
                                    } else {
                                        // Error converting number.
                                    }
                                }
                            }
                        } else if (data->NestMatch[0].Expr->Keyword == L'}') {
                            // Object {}
                            tree.Set(content, Tree::MakeTree(content, data->NestMatch), (key->NestMatch[0].Offset + 1),
                                     (key->NestMatch[0].Length - 2));
                        } else if (data->NestMatch[0].NestMatch.Size == 0) {
                            // String
                            tree.Set(
                                content,
                                String::Part(content, (data->NestMatch[0].Offset + 1), (data->NestMatch[0].Length - 2)),
                                (key->NestMatch[0].Offset + 1), (key->NestMatch[0].Length - 2));
                        }
                    }
                } else {
                    // Missing quotation.
                }
            } else {
                // Key is empty.
            }
        } else {
            // Extra comma.
        }
    }

    return tree;
}

Tree Qentem::Tree::FromJSON(const String &content) noexcept {
    Expressions json_expres;
    Expression  comma = Expression();
    Expression  colon = Expression();

    Expression opened_square_bracket = Expression();
    Expression closed_square_bracket = Expression();

    Expression quotation_start = Expression();
    Expression quotation_end   = Expression();

    Expression opened_curly_bracket = Expression();
    Expression closed_curly_bracket = Expression();

    Expression esc_quotation = Expression();
    esc_quotation.Keyword    = L"\\\"";
    esc_quotation.Flag       = Flags::IGNORE;

    Expression comment1      = Expression();
    Expression comment_next1 = Expression();
    comment1.Keyword         = L"/*";
    comment_next1.Keyword    = L"*/";
    comment1.Connected       = &comment_next1;

    Expression comment2      = Expression();
    Expression comment_next2 = Expression();
    comment2.Keyword         = L"//";
    comment_next2.Keyword    = L"\n";
    comment2.Connected       = &comment_next2;

    quotation_start.Keyword   = L'"';
    quotation_end.Keyword     = L'"';
    quotation_start.Connected = &quotation_end;
    quotation_end.NestExprs.Add(&esc_quotation);

    comma.Keyword = L',';
    comma.Flag    = Flags::SPLIT | Flags::TRIM;

    colon.Keyword = L':';
    colon.Flag    = Flags::SPLIT | Flags::TRIM;

    opened_curly_bracket.Keyword   = L'{';
    closed_curly_bracket.Keyword   = L'}';
    opened_curly_bracket.Connected = &closed_curly_bracket;
    closed_curly_bracket.Flag      = Flags::SPLITNEST;
    closed_curly_bracket.NestExprs.Add(&opened_curly_bracket)
        .Add(&opened_square_bracket)
        .Add(&quotation_start)
        .Add(&colon)
        .Add(&comma);

    opened_square_bracket.Keyword   = L'[';
    closed_square_bracket.Keyword   = L']';
    opened_square_bracket.Connected = &closed_square_bracket;
    closed_square_bracket.Flag      = Flags::SPLITNEST;
    closed_square_bracket.NestExprs.Add(&opened_curly_bracket).Add(&quotation_start).Add(&comma);

    json_expres.Add(&opened_curly_bracket); // .Add(&comment1).Add(&comment2)

    Array<Match> items = Engine::Search(content, json_expres);

    return Tree::MakeTree(content, items);
}
