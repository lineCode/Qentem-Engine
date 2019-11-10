
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
    UNumber Size{0};
    Type *  Storage{nullptr};
    UNumber Capacity{0};

    explicit Array() = default;

    explicit Array(UNumber _capacity) noexcept : Capacity(_capacity) {
        Memory::Allocate<Type>(&Storage, Capacity);
    }

    Array(Array<Type> &&src) noexcept : Size(src.Size), Storage(src.Storage), Capacity(src.Capacity) {
        src.Size     = 0;
        src.Storage  = nullptr;
        src.Capacity = 0;
    }

    explicit Array(Array<Type> const &src) noexcept : Capacity(src.Size) {
        if (src.Size != 0) {
            Memory::Allocate<Type>(&Storage, Capacity);

            for (UNumber i = 0; i < src.Size; i++) {
                Storage[Size++] = src[i];
            }
        }
    }

    Array<Type> &Add(Array<Type> &&src) noexcept {
        if (Capacity == 0) {
            Size     = src.Size;
            Storage  = src.Storage;
            Capacity = src.Capacity;
        } else {
            UNumber const _nSize = (Size + src.Size);
            if (_nSize > Capacity) {
                Resize(_nSize);
            }

            for (UNumber i = 0; i < src.Size; i++) {
                Storage[Size++] = static_cast<Type &&>(src[i]);
            }
        }

        src.Size     = 0;
        src.Storage  = nullptr;
        src.Capacity = 0;

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

        Storage[Size++] = item;

        return *this;
    }

    void SetCapacity(UNumber const _size) noexcept {
        Memory::Deallocate<Type>(&Storage);

        Size     = 0;
        Capacity = _size;

        Memory::Allocate<Type>(&Storage, Capacity);
    }

    void Resize(UNumber const _size) noexcept {
        Capacity  = ((_size == 0) ? 2 : _size);
        Type *tmp = Storage;
        Memory::Allocate<Type>(&Storage, Capacity);

        for (UNumber n = 0; n < Size; n++) {
            Storage[n] = static_cast<Type &&>(tmp[n]);
        }

        Memory::Deallocate<Type>(&tmp);
    }

    Array<Type> &operator=(Array<Type> &&src) noexcept {
        if (this != &src) {
            if (Storage != nullptr) {
                Memory::Deallocate<Type>(&Storage);
            }

            Size         = src.Size;
            src.Size     = 0;
            Storage      = src.Storage;
            src.Storage  = nullptr;
            Capacity     = src.Capacity;
            src.Capacity = 0;
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

        Size     = 0;
        Capacity = 0;
    }

    inline Type &operator[](UNumber const offset) const noexcept {
        return Storage[offset];
    }

    ~Array() noexcept {
        Memory::Deallocate<Type>(&Storage);
    }
};

} // namespace Qentem

#endif
