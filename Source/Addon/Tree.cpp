
/**
 * Qentem Tree
 *
 * @brief     Ordered Array With Hasing capability.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Addon/Tree.hpp"

using Qentem::Array;
using Qentem::Hash;
using Qentem::String;
using Qentem::Tree;

void Qentem::Hash::Set(Hash *_hash, const UNumber base, const UNumber level) noexcept {
    if (this->HashValue == 0) {
        this->HashValue = _hash->HashValue;
        this->ExactID   = _hash->ExactID;
        this->Key       = _hash->Key;
        this->Type      = _hash->Type;
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

void Qentem::Tree::InsertHash(Hash *_hash) noexcept {
    if (this->Table.Size == 0) {
        this->Table.SetCapacity(HashBase);
        this->Table.Size = HashBase;
    }

    this->Table[(_hash->HashValue % HashBase)].Set(_hash, HashBase, 1);
}

void Qentem::Tree::Drop(const String &key, UNumber offset, UNumber limit) noexcept {
    Hash *      _hash;
    const Tree *storage = GetInfo(&_hash, key, offset, limit);

    switch (_hash->Type) {
        // case VType::NumberT:
        //     _hash->Storage->Numbers[_hash->ExactID] = 0; // Waste of time
        //     break;
        case VType::StringT:
            storage->Strings[_hash->ExactID].Clear();
            break;
        case VType::OStringsT:
            storage->OStrings[_hash->ExactID].Clear();
            break;
        case VType::Child:
            storage->Child[_hash->ExactID] = Tree();
            break;
        default:
            break;
    }

    _hash->HashValue = 0;
}

void Qentem::Tree::Set(const String &key, double number) noexcept {
    Hash *  p_hash      = nullptr;
    UNumber _hash_value = String::Hash(key, 0, key.Length);
    UNumber id          = (_hash_value % HashBase);

    if (id < Table.Size) {
        p_hash = Table[id].Get(_hash_value, HashBase, 1);
    }

    if ((p_hash == nullptr) || (p_hash->HashValue != _hash_value)) {
        Hash _hash;
        _hash.ExactID   = Numbers.Size;
        _hash.HashValue = String::Hash(key, 0, key.Length);
        _hash.Key       = key;
        _hash.Type      = VType::NumberT;

        InsertHash(&_hash);
        Numbers.Add(number);
    } else {
        Numbers[p_hash->ExactID] = number;
    }
}

void Qentem::Tree::Set(const String &key, const String &text) noexcept {
    Hash *  p_hash      = nullptr;
    UNumber _hash_value = String::Hash(key, 0, key.Length);
    UNumber id          = (_hash_value % HashBase);

    if (id < Table.Size) {
        p_hash = Table[id].Get(_hash_value, HashBase, 1);
    }

    if ((p_hash == nullptr) || (p_hash->HashValue != _hash_value)) {
        Hash _hash;
        _hash.ExactID   = Strings.Size;
        _hash.HashValue = _hash_value;
        _hash.Key       = key;
        _hash.Type      = VType::StringT;
        InsertHash(&_hash);
        Strings.Add(text);
    } else {
        Strings[p_hash->ExactID] = text;
    }
}

void Qentem::Tree::Set(const String &key, const Array<String> &strings) noexcept {
    Hash *  p_hash      = nullptr;
    UNumber _hash_value = String::Hash(key, 0, key.Length);
    UNumber id          = (_hash_value % HashBase);

    if (id < Table.Size) {
        p_hash = Table[id].Get(_hash_value, HashBase, 1);
    }

    if ((p_hash == nullptr) || (p_hash->HashValue != _hash_value)) {
        Hash _hash;
        _hash.ExactID   = OStrings.Size;
        _hash.HashValue = _hash_value;
        _hash.Key       = key;
        _hash.Type      = VType::OStringsT;

        InsertHash(&_hash);
        OStrings.Add(strings);
    } else {
        OStrings[p_hash->ExactID] = strings;
    }
}

void Qentem::Tree::Set(const String &key, const Tree &child) noexcept {
    Hash *  p_hash      = nullptr;
    UNumber _hash_value = String::Hash(key, 0, key.Length);
    UNumber id          = (_hash_value % HashBase);

    if (id < Table.Size) {
        p_hash = Table[id].Get(_hash_value, HashBase, 1);
    }

    if ((p_hash == nullptr) || (p_hash->HashValue != _hash_value)) {
        Hash _hash;
        _hash.ExactID   = Child.Size;
        _hash.HashValue = _hash_value;
        _hash.Key       = key;
        _hash.Type      = VType::Child;

        InsertHash(&_hash);
        Child.Add(child);
    } else {
        Child[p_hash->ExactID] = child;
    }
}

const Tree *Qentem::Tree::GetInfo(Hash **hash, const String &key, UNumber offset, UNumber limit) const noexcept {
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
            if ((*hash)->Type == VType::Child) {
                return this->Child[(*hash)->ExactID].GetInfo(hash, key, end_offset, (limit - (end_offset - offset)));
            }

            return this;
        }
    }

    *hash = nullptr;
    return nullptr;
}

bool Qentem::Tree::GetID(UNumber &id, const String &key, UNumber offset, UNumber limit) noexcept {
    UNumber end = (offset + (limit - 1));

    while (key.Str[--end] != L'[') {
    }
    end++;
    limit--;

    return String::ToNumber(key, id, end, (limit - (end - offset)));
}

const Array<String> *Qentem::Tree::GetOStrings(const String &key, UNumber offset, UNumber limit) const noexcept {
    Hash *      _hash;
    const Tree *storage = GetInfo(&_hash, key, offset, limit);

    if ((storage != nullptr) && (_hash->Type == VType::OStringsT)) {
        return &(storage->OStrings[_hash->ExactID]);
    }

    return nullptr;
}

const Tree *Qentem::Tree::GetChild(const String &key, UNumber offset, UNumber limit) const noexcept {
    Hash *      _hash;
    const Tree *storage = GetInfo(&_hash, key, offset, limit);

    if ((storage != nullptr) && (_hash->Type == VType::Child)) {
        return &(storage->Child[_hash->ExactID]);
    }

    return nullptr;
}

bool Qentem::Tree::GetString(String &value, const String &key, UNumber offset, UNumber limit) const noexcept {
    Hash *      _hash;
    const Tree *storage = GetInfo(&_hash, key, offset, limit);

    if (storage != nullptr) {
        if (_hash->Type == VType::StringT) {
            value = storage->Strings[_hash->ExactID];
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
        }

        if (_hash->Type == VType::NumberT) {
            value = String::FromNumber(storage->Numbers[_hash->ExactID]);
            return true;
        }
    }

    return false;
}

bool Qentem::Tree::GetNumber(UNumber &value, const String &key, UNumber offset, UNumber limit) const noexcept {
    Hash *      _hash;
    const Tree *storage = GetInfo(&_hash, key, offset, limit);

    if (storage != nullptr) {
        if (_hash->Type == VType::StringT) {
            return String::ToNumber(storage->Strings[_hash->ExactID], value);
        }

        UNumber id;
        if (_hash->Type == VType::OStringsT) {
            if (Tree::GetID(id, key, offset, limit)) {
                if (id < storage->OStrings[_hash->ExactID].Size) {
                    String::ToNumber(storage->OStrings[_hash->ExactID][id], value);
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

bool Qentem::Tree::GetDouble(double &value, const String &key, UNumber offset, UNumber limit) const noexcept {
    Hash *      _hash;
    const Tree *storage = GetInfo(&_hash, key, offset, limit);

    if (storage != nullptr) {
        if (_hash->Type == VType::StringT) {
            return String::ToNumber(storage->Strings[_hash->ExactID], value);
        }

        UNumber id;
        if (_hash->Type == VType::OStringsT) {
            if (Tree::GetID(id, key, offset, limit)) {
                if (id < storage->OStrings[_hash->ExactID].Size) {
                    String::ToNumber(storage->OStrings[_hash->ExactID][id], value);
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
