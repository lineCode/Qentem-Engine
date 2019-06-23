
/**
 * Qentem Common
 *
 * @brief     Common file for Qentem.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#ifndef QENTEM_MEMORY_H
#define QENTEM_MEMORY_H

#include "Common.hpp"

namespace Qentem {

struct Memory {
    template <typename T>
    static void Allocate(T **_p, UNumber _size) noexcept {
        *_p = new T[_size];
    }

    template <typename T>
    static void Deallocate(T **_p) noexcept {
        delete[] * _p;
        *_p = nullptr;
    }

    template <typename T>
    static void AllocateBit(T **_p) noexcept {
        *_p = new T;
    }

    template <typename T>
    static void DeallocateBit(T **_p) noexcept {
        delete *_p;
        *_p = nullptr;
    }
};

} // namespace Qentem

#endif
