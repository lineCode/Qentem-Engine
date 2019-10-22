
/**
 * Qentem Memory
 *
 * @brief     Memory for Qentem.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Common.hpp"

#ifndef QENTEM_MEMORY_H
#define QENTEM_MEMORY_H

namespace Qentem {

template <typename Type>
struct Memory {
    static inline void Allocate(Type **_p, UNumber _size) noexcept {
        *_p = new Type[_size];
    }

    static inline void Deallocate(Type **_p) noexcept {
        delete[] * _p;
        *_p = nullptr;
    }

    static inline void AllocateBit(Type **_p) noexcept {
        *_p = new Type;
    }

    static inline void DeallocateBit(Type **_p) noexcept {
        delete *_p;
        *_p = nullptr;
    }
};

} // namespace Qentem

#endif
