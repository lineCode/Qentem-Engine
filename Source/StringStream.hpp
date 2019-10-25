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

enum SType { Bit = 0, PBit = 1, Bits = 2 };

struct StringBit {
    SType   Type;
    UNumber ID;
};

struct StringStream {
    UNumber               Length = 0;
    Array<String>         _strings;
    Array<String const *> p_strings;
    Array<StringStream>   collections;
    Array<StringBit>      bits;

    StringStream() = default;

    void operator+=(StringStream &&col) noexcept {
        if (col.Length != 0) {
            Length += col.Length;
            bits += {SType::Bits, collections.Size};
            collections += static_cast<StringStream &&>(col);
        }
    }

    void operator+=(StringStream const &col) noexcept {
        if (col.Length != 0) {
            Length += col.Length;
            bits += {SType::Bits, collections.Size};
            collections += col;
        }
    }

    void operator+=(String &&src) noexcept {
        if (src.Length != 0) {
            Length += src.Length;
            bits += {SType::Bit, _strings.Size};
            _strings += static_cast<String &&>(src);
        }
    }

    void operator+=(String const &src) noexcept {
        if (src.Length != 0) {
            Length += src.Length;
            bits += {SType::Bit, _strings.Size};
            _strings += src;
        }
    }

    void Share(String const *src) noexcept {
        if (src->Length != 0) {
            Length += src->Length;
            bits += {SType::PBit, p_strings.Size};
            p_strings += src;
        }
    }

    static void _pack(StringStream &_ss, String &buk) noexcept {
        String const *   sstr;
        StringBit const *ss_bit;
        UNumber          j = 0;

        for (UNumber i = 0; i < _ss.bits.Size; i++) {
            ss_bit = &(_ss.bits[i]);
            if (ss_bit->Type == SType::PBit) {
                sstr = _ss.p_strings[ss_bit->ID];

                for (j = 0; j < sstr->Length;) {
                    buk[buk.Length] = sstr->Str[j];
                    ++buk.Length;
                    ++j;
                }

                continue;
            }

            if (ss_bit->Type == SType::Bit) {
                sstr = &(_ss._strings[ss_bit->ID]);

                for (j = 0; j < sstr->Length;) {
                    buk[buk.Length] = sstr->Str[j];
                    ++buk.Length;
                    ++j;
                }

                continue;
            }

            _pack(_ss.collections[ss_bit->ID], buk);
        }
    }

    String Eject() noexcept {
        String tmp;
        tmp.SetLength(Length); // Will always add 1 for \0

        if (Length != 0) {
            _pack(*this, tmp);
            Length = 0;
            _strings.Reset();
            p_strings.Reset();
            collections.Reset();
            bits.Reset();
        }

        tmp[tmp.Length] = L'\0'; // Null trimmming

        return tmp;
    }
};

} // namespace Qentem

#endif
