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
namespace Memory {

template <typename Type>
static inline void Allocate(Type **pinter, const UNumber size) noexcept {
    *pinter = new Type[size];
}

template <typename Type>
static inline void Deallocate(Type **pinter) noexcept {
    delete[] * pinter;
    *pinter = nullptr;
}

template <typename Type>
static inline void AllocateBit(Type **pinter) noexcept {
    *pinter = new Type;
}

template <typename Type>
static inline void DeallocateBit(Type **pinter) noexcept {
    delete *pinter;
    *pinter = nullptr;
}

} // namespace Memory
} // namespace Qentem

#endif
