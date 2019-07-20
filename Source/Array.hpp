
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

template <typename Type>
struct Array {
    UNumber Index    = 0;
    UNumber Capacity = 0;
    Type *  Storage  = nullptr;

    Array() = default;

    Array(Array<Type> &&src) noexcept {
        Storage  = src.Storage;
        Capacity = src.Capacity;
        Index    = src.Index;

        src.Capacity = 0;
        src.Index    = 0;
        src.Storage  = nullptr;
    }

    Array(Array<Type> const &src) noexcept {
        Add(src);
    }

    Array<Type> &Add(Array<Type> &&src) noexcept {
        if ((src.Index + Index) > Capacity) {
            if (Capacity == 0) {
                Storage  = src.Storage;
                Capacity = src.Capacity;
                Index    = src.Index;

                src.Capacity = 0;
                src.Index    = 0;
                src.Storage  = nullptr;
                return *this;
            }

            Resize(src.Index + Index);
        }

        for (UNumber i = 0; i < src.Index; i++) {
            Storage[Index++] = static_cast<Type &&>(src[i]);
        }

        src.Reset();

        return *this;
    }

    Array<Type> &Add(Array<Type> const &src) noexcept {
        if ((src.Index + Index) > Capacity) {
            Resize(src.Index + Index);
        }

        for (UNumber i = 0; i < src.Index; i++) {
            Storage[Index++] = src[i];
        }

        return *this;
    }

    inline Array<Type> &Add(Type &&item) noexcept { // Move
        if (Index == Capacity) {
            Resize(Capacity * 2);
        }

        Storage[Index] = static_cast<Type &&>(item);
        ++Index;

        return *this;
    }

    inline Array<Type> &Add(Type const &item) noexcept { // Copy
        if (Index == Capacity) {
            Resize(Capacity * 2);
        }

        Storage[Index] = item;
        ++Index;

        return *this;
    }

    inline void SetCapacity(UNumber _size) noexcept {
        Memory<Type>::Deallocate(&Storage);

        if (_size == 0) {
            _size = 2;
        }

        Index    = 0;
        Capacity = _size;

        Memory<Type>::Allocate(&Storage, _size);
    }

    inline void Resize(UNumber _size) noexcept {
        if (_size == 0) {
            _size = 2;
        }

        // if (_size <= Index) {
        //     Index = _size;
        //     return;
        // }

        Type *tmp = Storage;
        Storage   = nullptr;
        Capacity  = _size;

        Memory<Type>::Allocate(&Storage, _size);

        for (UNumber n = 0; n < Index;) {
            Storage[n] = static_cast<Type &&>(tmp[n]);
            ++n;
        }

        Memory<Type>::Deallocate(&tmp);
    }

    Array<Type> &operator=(Array<Type> &&src) noexcept {
        if (this != &src) {
            Memory<Type>::Deallocate(&Storage);
            Storage  = src.Storage;
            Capacity = src.Capacity;
            Index    = src.Index;

            src.Capacity = 0;
            src.Index    = 0;
            src.Storage  = nullptr;
        }
        return *this;
    }

    Array<Type> &operator=(Array<Type> const &src) noexcept {
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

    inline void operator+=(Type &&item) noexcept {
        Add(static_cast<Type &&>(item));
    }

    inline void operator+=(Type const &item) noexcept {
        Add(item);
    }

    inline void operator+=(Array<Type> &&item) noexcept {
        Add(static_cast<Array<Type> &&>(item));
    }

    inline void operator+=(Array<Type> const &item) noexcept {
        Add(item);
    }

    inline void Reset() noexcept {
        Memory<Type>::Deallocate(&Storage);

        Capacity = 0;
        Index    = 0;
        Storage  = nullptr;
    }

    inline Type &operator[](UNumber const __index) const noexcept { // Compare
        return Storage[__index];
    }

    virtual ~Array() noexcept {
        Reset();
    }
};
} // namespace Qentem

#endif
