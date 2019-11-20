
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

    explicit Array(UNumber capacity) noexcept : Capacity(capacity) {
        Memory::Allocate<Type>(&Storage, Capacity);
    }

    Array(Array<Type> &&src) noexcept : Size(src.Size), Storage(src.Storage), Capacity(src.Capacity) {
        src.Size     = 0;
        src.Storage  = nullptr;
        src.Capacity = 0;
    }

    explicit Array(const Array<Type> &src) noexcept : Capacity(src.Size) {
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
            const UNumber nSize = (Size + src.Size);
            if (nSize > Capacity) {
                Resize(nSize);
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

    Array<Type> &Add(const Array<Type> &src) noexcept {
        const UNumber nSize = (Size + src.Size);

        if (nSize > Capacity) {
            Resize(nSize);
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

    Array<Type> &Add(const Type &item) noexcept { // Copy
        if (Size == Capacity) {
            Resize(Capacity * 2);
        }

        Storage[Size++] = item;

        return *this;
    }

    void SetCapacity(const UNumber size) noexcept {
        Memory::Deallocate<Type>(&Storage);

        Size     = 0;
        Capacity = size;

        Memory::Allocate<Type>(&Storage, Capacity);
    }

    void Resize(const UNumber size) noexcept {
        Capacity  = ((size != 0) ? size : 2);
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

    Array<Type> &operator=(const Array<Type> &src) noexcept {
        if (this != &src) {
            const UNumber nSize = (Size + src.Size);

            if (nSize > Capacity) {
                Resize(nSize);
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

    inline void operator+=(const Type &item) noexcept {
        Add(item);
    }

    inline void operator+=(Array<Type> &&item) noexcept {
        Add(static_cast<Array<Type> &&>(item));
    }

    inline void operator+=(const Array<Type> &item) noexcept {
        Add(item);
    }

    inline void Reset() noexcept {
        Memory::Deallocate<Type>(&Storage);

        Size     = 0;
        Capacity = 0;
    }

    inline Type &operator[](const UNumber offset) const noexcept {
        return Storage[offset];
    }

    ~Array() noexcept {
        Memory::Deallocate<Type>(&Storage);
    }
};

} // namespace Qentem

#endif
