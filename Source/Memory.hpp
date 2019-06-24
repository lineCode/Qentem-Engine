
/**
 * Qentem Memory
 *
 * @brief     Memory for Qentem.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#ifndef QENTEM_MEMORY_H
#define QENTEM_MEMORY_H

#include "Common.hpp"

namespace Qentem {

template <typename T>
struct Memory {
    static void Allocate(T **_p, UNumber _size) noexcept {
        *_p = new T[_size];
    }

    static void Deallocate(T **_p) noexcept {
        delete[] * _p;
        *_p = nullptr;
    }

    static void AllocateBit(T **_p) noexcept {
        *_p = new T;
    }

    static void DeallocateBit(T **_p) noexcept {
        delete *_p;
        *_p = nullptr;
    }
};

} // namespace Qentem

#endif
