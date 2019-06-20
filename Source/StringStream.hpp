
/**
 * Qentem String
 *
 * @brief     String Stream object for Qentem Engine.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#ifndef QENTEM_STRINGSTREAM_H
#define QENTEM_STRINGSTREAM_H

#include "Array.hpp"
#include "String.hpp"

namespace Qentem {

enum SType { Bit = 0, PBit, Bits };

struct StringBit {
    SType   Type;
    UNumber Index;
};

class StringStream {
  public:
    Array<String>       _strings;
    Array<String *>     p_strings;
    Array<StringStream> collections;

    Array<StringBit> bits;

    UNumber Length = 0;

    StringStream() = default;

    void operator+=(StringStream &&col) noexcept {
        if (col.Length != 0) {
            Length += col.Length;

            if (collections.Size == collections.Capacity) {
                collections.Resize((collections.Size + 1) * 4);
            }

            collections.Storage[collections.Size] = col;
            bits.Add({SType::Bits, collections.Size});
            ++collections.Size;
        }
    }

    void operator+=(String &&src) noexcept {
        if (src.Length != 0) {
            Length += src.Length;

            if (_strings.Size == _strings.Capacity) {
                _strings.Resize((_strings.Size + 1) * 4);
            }

            _strings.Storage[_strings.Size] = src;
            bits.Add({SType::Bit, _strings.Size});
            ++_strings.Size;
        }
    }

    void operator+=(const String &src) noexcept {
        if (src.Length != 0) {
            Length += src.Length;

            if (_strings.Size == _strings.Capacity) {
                _strings.Resize((_strings.Size + 1) * 4);
            }

            _strings.Storage[_strings.Size] = src;
            bits.Add({SType::Bit, _strings.Size});
            ++_strings.Size;
        }
    }

    void Share(String *src) noexcept {
        if (src->Length != 0) {
            Length += src->Length;

            if (p_strings.Size == p_strings.Capacity) {
                p_strings.Resize((p_strings.Size + 1) * 4);
            }

            p_strings.Storage[p_strings.Size] = src;
            bits.Add({SType::PBit, p_strings.Size});
            ++p_strings.Size;
        }
    }

    static void _pack(StringStream &_ss, String &buk) noexcept {
        String *sstr;
        UNumber j = 0;

        for (UNumber i = 0; i < _ss.bits.Size; i++) {
            switch (_ss.bits.Storage[i].Type) {
                case SType::Bits: {
                    _pack(_ss.collections.Storage[_ss.bits.Storage[i].Index], buk);
                    break;
                }
                case SType::PBit: {
                    sstr = _ss.p_strings.Storage[_ss.bits.Storage[i].Index];
                    for (j = 0; j < sstr->Length; j++) {
                        buk.Str[buk.Length++] = sstr->Str[j];
                    }
                    break;
                }
                default: {
                    sstr = &(_ss._strings.Storage[_ss.bits.Storage[i].Index]);
                    for (j = 0; j < sstr->Length; j++) {
                        buk.Str[buk.Length++] = sstr->Str[j];
                    }
                    break;
                }
            }
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

        tmp.Str[tmp.Length] = L'\0'; // Null trimmming
        return tmp;
    }
};
} // namespace Qentem

#endif
