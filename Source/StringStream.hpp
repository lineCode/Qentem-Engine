
/**
 * Qentem String Stream
 *
 * @brief     String Stream object for Qentem Engine.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Array.hpp"
#include "String.hpp"

#ifndef QENTEM_STRINGSTREAM_H
#define QENTEM_STRINGSTREAM_H

namespace Qentem {

enum SType { Bit = 0, PBit, Bits };

struct StringBit {
    SType   Type;
    UNumber Index;
};

class StringStream {
  public:
    Array<String>         _strings;
    Array<String const *> p_strings;
    Array<StringStream>   collections;

    Array<StringBit> bits;

    UNumber Length = 0;

    StringStream() = default;

    void operator+=(StringStream &&col) noexcept {
        if (col.Length != 0) {
            Length += col.Length;

            if (collections.Size == collections.Capacity) {
                collections.Resize((collections.Size + 1) * 2);
            }

            collections[collections.Size] = col;
            bits.Add({SType::Bits, collections.Size});
            ++collections.Size;
        }
    }

    void operator+=(String &&src) noexcept {
        if (src.Length != 0) {
            Length += src.Length;

            if (_strings.Size == _strings.Capacity) {
                _strings.Resize((_strings.Size + 1) * 2);
            }

            _strings[_strings.Size] = src;
            bits.Add({SType::Bit, _strings.Size});
            ++_strings.Size;
        }
    }

    void operator+=(String const &src) noexcept {
        if (src.Length != 0) {
            Length += src.Length;

            if (_strings.Size == _strings.Capacity) {
                _strings.Resize((_strings.Size + 1) * 2);
            }

            _strings[_strings.Size] = src;
            bits.Add({SType::Bit, _strings.Size});
            ++_strings.Size;
        }
    }

    void Share(String const *src) noexcept {
        if (src->Length != 0) {
            Length += src->Length;

            if (p_strings.Size == p_strings.Capacity) {
                p_strings.Resize((p_strings.Size + 1) * 2);
            }

            p_strings[p_strings.Size] = src;
            bits.Add({SType::PBit, p_strings.Size});
            ++p_strings.Size;
        }
    }

    static void _pack(StringStream &_ss, String &buk) noexcept {
        String const *sstr;
        UNumber       j = 0;

        for (UNumber i = 0; i < _ss.bits.Size; i++) {
            if (_ss.bits[i].Type == SType::Bit) {
                sstr = &(_ss._strings[_ss.bits[i].Index]);
                for (j = 0; j < sstr->Length;) {
                    buk[buk.Length] = sstr->operator[](j);
                    ++j;
                    ++buk.Length;
                }
                continue;
            }

            if (_ss.bits[i].Type == SType::PBit) {
                sstr = _ss.p_strings[_ss.bits[i].Index];
                for (j = 0; j < sstr->Length;) {
                    buk[buk.Length] = sstr->operator[](j);
                    ++j;
                    ++buk.Length;
                }
                continue;
            }

            _pack(_ss.collections[_ss.bits[i].Index], buk);
        }
    }

    String Eject() noexcept {
        String tmp;
        tmp.SetLength(Length); // Will always add 1 for \0

        if (Length != 0) {
            _pack(*this, tmp);
            // To allow the reuse of this object
            Length           = 0;
            _strings.Size    = 0;
            p_strings.Size   = 0;
            collections.Size = 0;
            bits.Size        = 0;
        }

        tmp[tmp.Length] = L'\0'; // Null trimmming
        return tmp;
    }
};
} // namespace Qentem

#endif
