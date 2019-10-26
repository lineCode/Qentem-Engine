
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
    UNumber Size     = 0;
    UNumber Capacity = 0;
    Type *  Storage  = nullptr;

    Array() = default;

    Array(Array<Type> &&src) noexcept {
        Storage  = src.Storage;
        Capacity = src.Capacity;
        Size     = src.Size;

        src.Size     = 0;
        src.Capacity = 0;
        src.Storage  = nullptr;
    }

    Array(Array<Type> const &src) noexcept {
        Add(src);
    }

    Array<Type> &Add(Array<Type> &&src) noexcept {
        UNumber const _nSize = (Size + src.Size);

        if (_nSize > Capacity) {
            if (Capacity == 0) {
                Capacity = src.Capacity;
                Size     = src.Size;
                Storage  = src.Storage;

                src.Storage  = nullptr;
                src.Size     = 0;
                src.Capacity = 0;

                return *this;
            }

            Resize(_nSize);
        }

        for (UNumber i = 0; i < src.Size; i++) {
            Storage[Size++] = static_cast<Type &&>(src[i]);
        }

        src.Reset();

        return *this;
    }

    Array<Type> &Add(Array<Type> const &src) noexcept {
        UNumber const _nSize = (Size + src.Size);
        if (_nSize > Capacity) {
            Resize(_nSize);
        }

        for (UNumber i = 0; i < src.Size; i++) {
            Storage[Size++] = src[i];
        }

        return *this;
    }

    Array<Type> &Add(Type &&item) noexcept { // Move
        if (Size == Capacity) {
            Resize(Capacity * 2);
        }

        Storage[Size++] = static_cast<Type &&>(item);

        return *this;
    }

    inline Array<Type> &Add(Type const &item) noexcept { // Copy
        if (Size == Capacity) {
            Resize(Capacity * 2);
        }

        Storage[Size] = item;
        ++Size;

        return *this;
    }

    void SetCapacity(UNumber _size) noexcept {
        Memory::Deallocate<Type>(&Storage);

        Size     = 0;
        Capacity = _size;

        Memory::Allocate<Type>(&Storage, Capacity);
    }

    void Resize(UNumber _size) noexcept {
        if (_size == 0) {
            _size = 2;
        }

        // if (_size <= Size) {
        //     Size = _size;
        //     return;
        // }

        Type *tmp = Storage;
        Storage   = nullptr;
        Capacity  = _size;

        Memory::Allocate<Type>(&Storage, Capacity);

        for (UNumber n = 0; n < Size; n++) {
            Storage[n] = static_cast<Type &&>(tmp[n]);
        }

        Memory::Deallocate<Type>(&tmp);
    }

    Array<Type> &operator=(Array<Type> &&src) noexcept {
        if (this != &src) {
            Memory::Deallocate<Type>(&Storage);
            Storage  = src.Storage;
            Capacity = src.Capacity;
            Size     = src.Size;

            src.Size     = 0;
            src.Capacity = 0;
            src.Storage  = nullptr;
        }

        return *this;
    }

    Array<Type> &operator=(Array<Type> const &src) noexcept {
        if (this != &src) {
            UNumber const _nSize = (Size + src.Size);

            if (_nSize > Capacity) {
                Resize(_nSize);
            }

            for (UNumber i = 0; i < src.Size; i++) {
                Storage[Size++] = src[i];
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
        Memory::Deallocate<Type>(&Storage);

        Capacity = 0;
        Size     = 0;
        Storage  = nullptr;
    }

    inline Type &operator[](UNumber const offset) const noexcept {
        return Storage[offset];
    }

    virtual ~Array() noexcept {
        Reset();
    }
};

} // namespace Qentem

#endif
