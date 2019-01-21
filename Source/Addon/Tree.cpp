
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

using Qentem::String;

void Qentem::Tree::Add(const String &key, const String &_data) noexcept {
    Keys.Add(key);
    ExactID.Add(Strings.Size);
    Types.Add(VType::StringT);

    Strings.Add(_data);
}

void Qentem::Tree::Add(const String &key, const Array<String> &_data) noexcept {
    Keys.Add(key);
    ExactID.Add(Arrays.Size);
    Types.Add(VType::ArrayT);

    Arrays.Add(_data);
}

void Qentem::Tree::Add(const String &key, const Tree &_data) noexcept {
    Keys.Add(key);
    ExactID.Add(VArray.Size);
    Types.Add(VType::TreeT);

    VArray.Add(_data);
}

bool Qentem::Tree::GetIndex(const String &key, UNumber &index) const noexcept {
    // TODO(Hani): Use hashing!
    for (UNumber id = 0; id <= Keys.Size; id++) {
        if (key == Keys[id]) {
            index = id;
            return true;
        }
    }

    return false;
}

// Key from can be: name, name[id1], name[id1][id2], name[id1][id2][idx]...
String *Qentem::Tree::GetValue(const String &key) noexcept {
    String L_key    = key;
    String id       = L"";
    String reminder = L"";

    if (Tree::DecodeKey(L_key, id, reminder)) {
        UNumber index = 0;
        if (Tree::GetIndex(L_key, index)) {
            const VType type = Types[index];
            index            = ExactID[index];
            if (type == VType::StringT) {
                return &(Strings[index]);
            }

            if (type == VType::NullT) {
                return nullptr;
            }

            if (id.Length != 0) {
                if (type == VType::ArrayT) {
                    double nid = 0.0;
                    if ((String::ToNumber(id, nid)) && (static_cast<UNumber>(nid) < Arrays[index].Size)) {
                        return &(Arrays[index][static_cast<UNumber>(nid)]);
                    }
                } else if (type == VType::TreeT) {
                    if (reminder.Length != 0) {
                        id += reminder;
                    }

                    return VArray[index].GetValue(id);
                }
            }
        }
    }
    return nullptr;
}

bool Qentem::Tree::DecodeKey(String &key, String &id, String &reminder) noexcept {
    // key = L"glob[rrr][ccc]"; // For testing only!
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
                // It's just an empty []
                return false;
            }
        }

        key = l_key;
        return true;
    }

    return false;
}
