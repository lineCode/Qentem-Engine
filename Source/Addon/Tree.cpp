
/**
 * Qentem Tree
 *
 * @brief     Unordered Array
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Addon/Tree.hpp"

using Qentem::Hash;
using Qentem::String;

void Qentem::Tree::Add(const String &key, double number) noexcept {
    Hash _hash;
    _hash.Key       = key;
    _hash.Type      = VType::NumberT;
    _hash.HashValue = String::Hash(key, 0, key.Length);
    _hash.ExactID   = Numbers.Size;

    InsertHash(_hash);
    Numbers.Add(number);
}

void Qentem::Tree::Add(const String &key, const String &_data) noexcept {
    Hash _hash;
    _hash.Key       = key;
    _hash.Type      = VType::StringT;
    _hash.HashValue = String::Hash(key, 0, key.Length);
    _hash.ExactID   = Strings.Size;

    InsertHash(_hash);
    Strings.Add(_data);
}

void Qentem::Tree::Add(const String &key, const Array<String> &_data) noexcept {
    Hash _hash;
    _hash.Key       = key;
    _hash.Type      = VType::ArrayT;
    _hash.HashValue = String::Hash(key, 0, key.Length);
    _hash.ExactID   = Arrays.Size;

    InsertHash(_hash);
    Arrays.Add(_data);
}

void Qentem::Tree::Add(const String &key, const Tree &_data) noexcept {
    Hash _hash;
    _hash.Key       = key;
    _hash.Type      = VType::TreeT;
    _hash.HashValue = String::Hash(key, 0, key.Length);
    _hash.ExactID   = VArray.Size;

    InsertHash(_hash);
    VArray.Add(_data);
}

void Qentem::Tree::InsertHash(Hash &_hash) noexcept {
    if (this->Table.Size == 0) {
        this->Table.SetSize(_base);
        this->Table.Size = _base;
    }

    this->Table[(_hash.HashValue % _base)].set(_hash, _base, 1);
}

Hash *Qentem::Tree::GetInfo(const String &key) const noexcept {
    UNumber hash = String::Hash(key, 0, key.Length);
    return &(Table[(hash % _base)].get(hash, _base, 1));
}

// Key form can be: name, name[id1], name[id1][id2], name[id1][id2][idx]...
String *Qentem::Tree::GetValue(const String &key) const noexcept {
    String L_key    = key;
    String id       = L"";
    String reminder = L"";

    if (DecodeKey(L_key, id, reminder)) {
        Hash *_hash = GetInfo(L_key);

        if ((_hash->HashValue == 0) || (_hash->Type == VType::NullT)) {
            return nullptr;
        }

        if (_hash->Type == VType::StringT) {
            return &(Strings[_hash->ExactID]);
        }

        if (id.Length != 0) {
            if (_hash->Type == VType::ArrayT) {
                double nid = 0.0;
                if ((String::ToNumber(id, nid)) && (static_cast<UNumber>(nid) < Arrays[_hash->ExactID].Size)) {
                    return &(Arrays[_hash->ExactID][static_cast<UNumber>(nid)]);
                }
            } else if (_hash->Type == VType::TreeT) {
                if (reminder.Length != 0) {
                    id += reminder;
                }

                return VArray[_hash->ExactID].GetValue(id);
            }
        }
    }
    return nullptr;
}

bool Qentem::Tree::DecodeKey(String &key, String &id, String &reminder) noexcept {
    // key = L"glob[rrr][ccc]"; // For testing!
    id       = L"";
    id       = L"";
    reminder = L"";
    if (key.Length != 0) {
        if (key.Str[(key.Length - 1)] != ']') {
            // Just a simple name like: var1
            return true;
        }

        String l_key = L"";
        // It has to have at least one letter before [
        UNumber index = 0;
        while ((index < key.Length) && (key.Str[++index] != '[')) { // There is \0 at the end of wchar_t[]
        };

        if (index == key.Length) {
            return false;
        }

        l_key = String::Part(key, 0, index);

        while ((index < key.Length) && (key.Str[++index] != ']')) {
        };

        UNumber len = (index - (l_key.Length + 1));

        if (len == 0) {
            return false;
        }

        id = String::Part(key, (l_key.Length + 1), len);

        ++index;
        if (index != key.Length) {
            reminder = String::Part(key, index, key.Length - index);
            if (reminder.Length < 3) {
                // Empty.
                return false;
            }
        }

        key = l_key;
        return true;
    }

    return false;
}
