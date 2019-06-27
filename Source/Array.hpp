
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
    UNumber Size     = 0;
    UNumber Capacity = 0;
    Number  memID    = -1; // -1 means it has its own pool
    T *     Storage  = nullptr;

    Array() = default;

    Array(Array<T> &&src) noexcept {
        Storage  = src.Storage;
        memID    = src.memID;
        Capacity = src.Capacity;
        Size     = src.Size;

        src.Capacity = 0;
        src.Size     = 0;
        src.memID    = -1;
        src.Storage  = nullptr;
    }

    Array(Array<T> const &src) noexcept {
        Add(src);
    }

    Array<T> &operator=(Array<T> &&src) noexcept {
        if (this != &src) {
            Memory<T>::Deallocate(&Storage);
            Storage  = src.Storage;
            memID    = src.memID;
            Capacity = src.Capacity;
            Size     = src.Size;

            src.Capacity = 0;
            src.Size     = 0;
            src.memID    = -1;
            src.Storage  = nullptr;
        }
        return *this;
    }

    Array<T> &operator=(Array<T> const &src) noexcept {
        if (this != &src) {
            if (Capacity < src.Size) {
                Memory<T>::Deallocate(&Storage);
                Memory<T>::Allocate(&Storage, src.Size);
                Capacity = src.Size;
            }

            for (UNumber n = 0; n < src.Size; n++) {
                Storage[n] = src[n];
            }

            Size = src.Size;
        }

        return *this;
    }

    Array<T> &Add(Array<T> &&src) noexcept {
        if ((src.Size + Size) > Capacity) {
            Resize(src.Size + Size);
        }

        for (UNumber i = 0; i < src.Size; i++) {
            Storage[Size++] = static_cast<T &&>(src[i]);
        }

        src.Reset();

        return *this;
    }

    Array<T> &Add(Array<T> const &src) noexcept {
        if ((src.Size + Size) > Capacity) {
            Resize(src.Size + Size);
        }

        for (UNumber i = 0; i < src.Size; i++) {
            Storage[Size++] = src[i];
        }

        return *this;
    }

    inline Array<T> &Add(T &&item) noexcept { // Move
        if (Size == Capacity) {
            Resize((Capacity + 1) * 2);
        }

        Storage[Size] = item;
        ++Size;

        return *this;
    }

    inline Array<T> &Add(T const &item) noexcept { // Copy
        if (Size == Capacity) {
            Resize((Capacity + 1) * 2);
        }

        Storage[Size] = item;
        ++Size;

        return *this;
    }

    inline void SetCapacity(UNumber const _size) noexcept {
        Memory<T>::Deallocate(&Storage);

        Size     = 0;
        Capacity = _size;

        Memory<T>::Allocate(&Storage, _size);
    }

    inline void Resize(UNumber const _size) noexcept {
        T *tmp   = Storage;
        Storage  = nullptr;
        Capacity = _size;

        if (_size < Size) {
            Size = _size;
        }

        Memory<T>::Allocate(&Storage, _size);

        for (UNumber n = 0; n < Size;) {
            Storage[n] = static_cast<T &&>(tmp[n]);
            ++n;
        }

        Memory<T>::Deallocate(&tmp);
    }

    inline T &operator[](UNumber const __index) const noexcept { // Compare
        return Storage[__index];
    }

    inline void Share(T *_storage, UNumber _size) noexcept {
        Storage  = _storage;
        Capacity = _size;
        Size     = _size;
        memID    = -2;
    }

    inline void Reset() noexcept {
        Memory<T>::Deallocate(&Storage);

        Capacity = 0;
        Size     = 0;
        memID    = -1;
    }

    virtual ~Array() noexcept {
        if (memID == -2) { // Shared
            Storage = nullptr;
        }

        Reset();
    }
};
} // namespace Qentem

#endif
