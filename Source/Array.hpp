
/**
 * Qentem Array
 *
 * @brief     Unordered Array
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#ifndef QENTEM_ARRAY_H
#define QENTEM_ARRAY_H

#include "Memory.hpp"

namespace Qentem {

template <typename T>
struct Array {
    UNumber Size     = 0;
    UNumber Capacity = 0;
    T *     Storage  = nullptr;
    bool    Shared   = false;

    Array() = default;

    Array(Array<T> &&src) noexcept {
        Storage  = src.Storage;
        Capacity = src.Capacity;
        Size     = src.Size;

        src.Capacity = 0;
        src.Size     = 0;
        src.Storage  = nullptr;
    }

    Array(Array<T> &src) noexcept {
        Add(src, false);
    }

    Array<T> &operator=(const Array<T> &src) noexcept {
        if (this != &src) {
            if (src.Size == 0) {
                Memory::Deallocate<T>(&Storage);
                Size     = 0;
                Capacity = 0;

                return *this;
            }

            Size = src.Size;

            if (Capacity < src.Size) {
                Memory::Deallocate<T>(&Storage);
                Memory::Allocate<T>(&Storage, Size);

                Capacity = src.Size;

                for (UNumber n = 0; n < src.Size; n++) {
                    Storage[n] = static_cast<T &&>(src[n]);
                }

                return *this;
            }

            for (UNumber n = 0; n < src.Size; n++) {
                Storage[n] = static_cast<T &&>(src[n]);
            }
        }

        return *this;
    }

    Array<T> &operator=(Array<T> &&src) noexcept {
        if (this != &src) {
            Memory::Deallocate<T>(&Storage);

            Storage  = src.Storage;
            Capacity = src.Capacity;
            Size     = src.Size;

            src.Capacity = 0;
            src.Size     = 0;
            src.Storage  = nullptr;
        }
        return *this;
    }

    Array<T> &Add(Array<T> &src, bool move = false) noexcept {
        if (src.Size != 0) {
            if ((src.Size + Size) > Capacity) {
                Capacity += src.Size;

                if (Size == 0) {
                    if (!move) {
                        Memory::Allocate<T>(&Storage, Capacity);
                    } else {
                        Storage  = src.Storage;
                        Capacity = src.Capacity;
                        Size     = src.Size;

                        src.Storage  = nullptr;
                        src.Capacity = 0;
                        src.Size     = 0;

                        return *this;
                    }
                } else {
                    T *tmp = Storage;
                    Memory::Allocate<T>(&Storage, Capacity);

                    for (UNumber n = 0; n < Size; n++) {
                        Storage[n] = static_cast<T &&>(tmp[n]);
                    }

                    Memory::Deallocate<T>(&tmp);
                }
            }

            if (move) {
                for (UNumber i = 0; i < src.Size; i++) {
                    Storage[Size++] = static_cast<T &&>(src[i]);
                }

                Memory::Deallocate<T>(&src.Storage);
                src.Storage  = nullptr;
                src.Capacity = 0;
                src.Size     = 0;
            } else {
                for (UNumber i = 0; i < src.Size; i++) {
                    Storage[Size++] = src[i];
                }
            }
        }

        return *this;
    }

    Array<T> &Add(const Array<T> &src) noexcept {
        if (src.Size != 0) {
            if ((src.Size + Size) > Capacity) {
                Capacity += src.Size;

                if (Size == 0) {
                    Memory::Allocate<T>(&Storage, Capacity);
                } else {
                    T *tmp = Storage;
                    Memory::Allocate<T>(&Storage, Capacity);

                    for (UNumber n = 0; n < Size; n++) {
                        Storage[n] = static_cast<T &&>(tmp[n]);
                    }

                    Memory::Deallocate<T>(&tmp);
                }
            }

            for (UNumber i = 0; i < src.Size; i++) {
                Storage[Size++] = src[i];
            }
        }

        return *this;
    }

    inline Array<T> &Add(const T &item) noexcept { // Copy
        if (Size == Capacity) {
            Resize((Capacity + 1) * 4);
        }

        Storage[Size] = item;
        ++Size;

        return *this;
    }

    inline Array<T> &Add(T &&item) noexcept { // Move
        if (Size == Capacity) {
            Resize((Capacity + 1) * 4);
        }

        Storage[Size] = item;
        ++Size;

        return *this;
    }

    inline void SetCapacity(const UNumber _size) noexcept {
        Memory::Deallocate<T>(&Storage);

        Size     = 0;
        Capacity = _size;

        Memory::Allocate<T>(&Storage, _size);
    }

    inline void Resize(const UNumber _size) noexcept {
        Capacity = _size;
        T *tmp   = Storage;

        Memory::Allocate<T>(&Storage, Capacity);

        if (_size < Size) {
            Size = _size;
        }

        for (UNumber n = 0; n < Size; n++) {
            Storage[n] = static_cast<T &&>(tmp[n]);
        }

        Memory::Deallocate<T>(&tmp);
    }

    inline T &operator[](const UNumber __index) const noexcept { // Compare
        return Storage[__index];
    }

    inline void Share(T *_storage, UNumber _size) noexcept {
        Storage = _storage;

        Capacity = _size;
        Size     = _size;
        Shared   = true;
    }

    inline void Reset() noexcept {
        Memory::Deallocate<T>(&Storage);

        Capacity = 0;
        Size     = 0;
    }

    virtual ~Array() noexcept {
        if (!Shared) {
            Reset();
        }
    }
};
} // namespace Qentem

#endif
