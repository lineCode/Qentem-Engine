
/**
 * Qentem QArray
 *
 * @brief     Dynamic array for Array<String, String>; Map
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "QArray.hpp"

void Qentem::QArray::Add(const String &key, const String &_data) noexcept {
    Keys.Add(key);
    RealID.Add(Strings.Size());
    Types.Add(VType::StringT);

    Strings.Add(_data);
}

void Qentem::QArray::Add(const String &key, const Array<String> &_data) noexcept {
    Keys.Add(key);
    RealID.Add(Arrays.Size());
    Types.Add(VType::ArrayT);

    Arrays.Add(_data);
}

void Qentem::QArray::Add(const String &key, const QArray &_data) noexcept {
    Keys.Add(key);
    RealID.Add(VArray.Size());
    Types.Add(VType::QArrayT);

    VArray.Add(_data);
}

bool Qentem::QArray::GetIndex(const String &key, size_t &index) const noexcept {
    // TODO: Use hashing!
    for (size_t id = 0; id <= Keys.Size(); id++) {
        if (key == Keys[id]) {
            index = id;
            return true;
        }
    }

    return false;
}

// Key form can be: name, name[id1], name[id1][id2], name[id1][id2][idx]...
String *Qentem::QArray::GetValue(const String &key) noexcept {
    String L_key    = key;
    String id       = L"";
    String reminder = L"";

    if (DecodeKey(L_key, id, reminder)) {
        size_t index = 0;
        if (GetIndex(L_key, index)) {
            const VType type = Types[index];
            index            = RealID[index];
            if (type == VType::StringT) {
                return &(Strings[index]);
            } else if (type == VType::NullT) {
                return nullptr;
            }

            if (id.Length != 0) {
                if (type == VType::ArrayT) {
                    size_t nid = (size_t)(String::ToNumber(id));
                    if (nid < Arrays[index].Size()) {
                        return &(Arrays[index][nid]);
                    }
                } else if (type == VType::QArrayT) {
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

bool Qentem::QArray::DecodeKey(String &key, String &id, String &reminder) noexcept {
    // key = L"glob[rrr][ccc]"; // For testing only!
    id       = L"";
    id       = L"";
    reminder = L"";
    if (key.Length != 0) {
        if (key.Str[(key.Length - 1)] != ']') {
            // Just a simple name like: var1
            return true;
        } else {
            String l_key = L"";
            // It has to have at least one letter before [
            size_t index = 0;
            while ((index < key.Length) && (key.Str[++index] != '[')) { // There is \0 at the end of wchar_t[]
            };                                                          // TODO: index might be out of order

            if (index == key.Length) {
                return false;
            }

            l_key = key.Part(0, index);

            while ((index < key.Length) && (key.Str[++index] != ']')) {
            };

            size_t len = (index - (l_key.Length + 1));

            if (len == 0) {
                return false;
            }

            id = key.Part((l_key.Length + 1), len);

            ++index;
            if (index != key.Length) {
                reminder = key.Part(index, key.Length - index);
                if (reminder.Length < 3) {
                    // It's just an empty []
                    return false;
                }
            }

            key = l_key;
            return true;
        }
    }

    return false;
}
