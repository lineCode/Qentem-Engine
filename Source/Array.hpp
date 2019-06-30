
/**
 * Qentem Array
 *
 * @brief     Ordered Array
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Memory.hpp"

#ifndef QENTEM_ARRAY_H
#define QENTEM_ARRAY_H

namespace Qentem {

template <typename T>
struct Array {
    UNumber Index    = 0;
    UNumber Capacity = 0;
    T *     Storage  = nullptr;

    Array() = default;

    Array(Array<T> &&src) noexcept {
        Storage  = src.Storage;
        Capacity = src.Capacity;
        Index    = src.Index;

        src.Capacity = 0;
        src.Index    = 0;
        src.Storage  = nullptr;
    }

    Array(Array<T> const &src) noexcept {
        Add(src);
    }

    Array<T> &operator=(Array<T> &&src) noexcept {
        if (this != &src) {
            Memory<T>::Deallocate(&Storage);
            Storage  = src.Storage;
            Capacity = src.Capacity;
            Index    = src.Index;

            src.Capacity = 0;
            src.Index    = 0;
            src.Storage  = nullptr;
        }
        return *this;
    }

    Array<T> &operator=(Array<T> const &src) noexcept {
        if (this != &src) {
            if ((src.Index + Index) > Capacity) {
                Resize(src.Index + Index);
            }

            for (UNumber i = 0; i < src.Index; i++) {
                Storage[Index++] = src[i];
            }
        }

        return *this;
    }

    Array<T> &Add(Array<T> &&src) noexcept {
        if ((src.Index + Index) > Capacity) {
            Resize(src.Index + Index);
        }

        for (UNumber i = 0; i < src.Index; i++) {
            Storage[Index++] = static_cast<T &&>(src[i]);
        }

        src.Reset();

        return *this;
    }

    Array<T> &Add(Array<T> const &src) noexcept {
        if ((src.Index + Index) > Capacity) {
            Resize(src.Index + Index);
        }

        for (UNumber i = 0; i < src.Index; i++) {
            Storage[Index++] = src[i];
        }

        return *this;
    }

    inline Array<T> &Add(T &&item) noexcept { // Move
        if (Index == Capacity) {
            Resize(Capacity * 2);
        }

        Storage[Index] = item;
        ++Index;

        return *this;
    }

    inline Array<T> &Add(T const &item) noexcept { // Copy
        if (Index == Capacity) {
            Resize(Capacity * 2);
        }

        Storage[Index] = item;
        ++Index;

        return *this;
    }

    inline void SetCapacity(UNumber _size) noexcept {
        Memory<T>::Deallocate(&Storage);

        if (_size == 0) {
            _size = 2;
        }

        Index    = 0;
        Capacity = _size;

        Memory<T>::Allocate(&Storage, _size);
    }

    inline void Resize(UNumber _size) noexcept {
        if (_size == 0) {
            _size = 2;
        }

        T *tmp   = Storage;
        Storage  = nullptr;
        Capacity = _size;

        if (_size < Index) {
            Index = _size;
        }

        Memory<T>::Allocate(&Storage, _size);

        for (UNumber n = 0; n < Index;) {
            Storage[n] = static_cast<T &&>(tmp[n]);
            ++n;
        }

        Memory<T>::Deallocate(&tmp);
    }

    inline T &operator[](UNumber const __index) const noexcept { // Compare
        return Storage[__index];
    }

    inline void Reset() noexcept {
        Memory<T>::Deallocate(&Storage);

        Capacity = 0;
        Index    = 0;
        Storage  = nullptr;
    }

    virtual ~Array() noexcept {
        Reset();
    }
};
} // namespace Qentem

#endif
