
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

#include "Common.hpp"

namespace Qentem {

template <typename T>
struct Array {
    UNumber Size     = 0;
    UNumber Capacity = 0;
    T *     Storage  = nullptr;

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
                if (Storage != nullptr) {
                    delete[] Storage;
                    Storage  = nullptr;
                    Size     = 0;
                    Capacity = 0;
                }

                return *this;
            }

            Size = src.Size;

            if (Capacity < src.Size) {
                if (Storage != nullptr) {
                    delete[] Storage;
                }

                Storage  = new T[Size];
                Capacity = src.Size;

                for (UNumber n = 0; n < src.Size; n++) {
                    Storage[n] = static_cast<T &&>(src.Storage[n]);
                }

                return *this;
            }

            for (UNumber n = 0; n < src.Size; n++) {
                Storage[n] = static_cast<T &&>(src.Storage[n]);
            }
        }

        return *this;
    }

    Array<T> &operator=(Array<T> &&src) noexcept {
        if (this != &src) {
            if (Storage != nullptr) {
                delete[] Storage;
            }

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
                        Storage = new T[Capacity];
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
                    T *tmp  = Storage;
                    Storage = new T[Capacity];

                    for (UNumber n = 0; n < Size; n++) {
                        Storage[n] = static_cast<T &&>(tmp[n]);
                    }

                    delete[] tmp;
                }
            }

            if (move) {
                for (UNumber i = 0; i < src.Size; i++) {
                    Storage[Size++] = static_cast<T &&>(src.Storage[i]);
                }

                delete[] src.Storage;
                src.Storage  = nullptr;
                src.Capacity = 0;
                src.Size     = 0;
            } else {
                for (UNumber i = 0; i < src.Size; i++) {
                    Storage[Size++] = src.Storage[i];
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
                    Storage = new T[Capacity];
                } else {
                    T *tmp  = Storage;
                    Storage = new T[Capacity];

                    for (UNumber n = 0; n < Size; n++) {
                        Storage[n] = static_cast<T &&>(tmp[n]);
                    }

                    delete[] tmp;
                }
            }

            for (UNumber i = 0; i < src.Size; i++) {
                Storage[Size++] = src.Storage[i];
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
        if (Storage != nullptr) {
            delete[] Storage;
            Storage = nullptr;
        }

        Size     = 0;
        Capacity = _size;

        if (_size != 0) {
            Storage = new T[_size];
        }
    }

    inline void Resize(const UNumber _size) noexcept {
        Capacity = _size;
        T *tmp   = Storage;

        Storage = new T[Capacity];

        if (_size < Size) {
            Size = _size;
        }

        for (UNumber n = 0; n < Size; n++) {
            Storage[n] = static_cast<T &&>(tmp[n]);
        }

        if (tmp != nullptr) {
            delete[] tmp;
        }
    }

    inline void Reset() noexcept {
        if (Storage != nullptr) {
            delete[] Storage;
            Storage = nullptr;
        }

        Capacity = 0;
        Size     = 0;
    }

    virtual ~Array() noexcept {
        Reset();
    }
};
} // namespace Qentem

#endif
