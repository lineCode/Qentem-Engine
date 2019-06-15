
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
        this->Storage  = src.Storage;
        this->Capacity = src.Capacity;
        this->Size     = src.Size;

        src.Capacity = 0;
        src.Size     = 0;
        src.Storage  = nullptr;
    }

    Array(Array<T> &src) {
        Add(src, false);
    }

    Array<T> &operator=(const Array<T> &src) noexcept {
        if (this != &src) {
            delete[] this->Storage;
            this->Capacity = src.Size;
            this->Size     = src.Size;

            this->Storage = new T[this->Size];
            for (UNumber n = 0; n < src.Size; n++) {
                this->Storage[n] = static_cast<T &&>(src.Storage[n]);
            }
        }
        return *this;
    }

    Array<T> &operator=(Array<T> &&src) noexcept {
        if (this != &src) {
            delete[] this->Storage;
            this->Storage  = src.Storage;
            this->Capacity = src.Capacity;
            this->Size     = src.Size;

            src.Capacity = 0;
            src.Size     = 0;
            src.Storage  = nullptr;
        }
        return *this;
    }

    Array<T> &Add(Array<T> &src, bool move = false) noexcept {
        if (src.Size != 0) {
            this->Capacity += src.Size;

            if (this->Size != 0) {
                T *tmp        = this->Storage;
                this->Storage = new T[this->Capacity];

                for (UNumber n = 0; n < this->Size; n++) {
                    this->Storage[n] = static_cast<T &&>(tmp[n]);
                }

                delete[] tmp;
            } else {
                this->Storage = new T[this->Capacity];
            }

            if (move) {
                for (UNumber i = 0; i < src.Size; i++) {
                    this->Storage[this->Size] = static_cast<T &&>(src.Storage[i]);
                }
            } else {
                for (UNumber i = 0; i < src.Size; i++) {
                    this->Storage[this->Size] = src.Storage[i];
                }
            }

            this->Size += src.Size;

            delete[] src.Storage;
            src.Storage = nullptr;
        }

        src.Capacity = 0;
        src.Size     = 0;

        return *this;
    }

    Array<T> &Add(const T &item) noexcept { // Copy
        if (this->Size == this->Capacity) {
            ExpandTo((this->Size == 0 ? 1 : (this->Capacity * 2)));
        }

        this->Storage[this->Size] = item;
        ++this->Size;

        return *this;
    }

    Array<T> &Add(T &&item) noexcept { // Move
        if (this->Size == this->Capacity) {
            ExpandTo((this->Size == 0 ? 1 : (this->Capacity * 2)));
        }

        this->Storage[this->Size] = item;
        ++this->Size;

        return *this;
    }

    void SetCapacity(const UNumber size) noexcept {
        delete[] this->Storage;
        this->Storage = nullptr;

        this->Size     = 0;
        this->Capacity = size;

        if (size != 0) {
            this->Storage = new T[size];
        }
    }

    // static void Expand2(Array<T> &src, const UNumber size) noexcept {
    //     src.Capacity = size;
    //     T *tmp       = src.Storage;

    //     src.Storage = new T[src.Capacity];

    //     for (UNumber n = 0; n < src.Size; n++) {
    //         src.Storage[n] = static_cast<T &&>(tmp[n]);
    //     }

    //     delete[] tmp;
    // }

    void ExpandTo(const UNumber size) noexcept {
        this->Capacity = size;
        T *tmp         = this->Storage;

        this->Storage = new T[this->Capacity];

        for (UNumber n = 0; n < this->Size; n++) {
            this->Storage[n] = static_cast<T &&>(tmp[n]);
        }

        delete[] tmp;
    }

    void Clear() noexcept {
        delete[] this->Storage;
        this->Storage  = nullptr;
        this->Capacity = 0;
        this->Size     = 0;
    }

    virtual ~Array() noexcept {
        Clear();
    }

}; // namespace Qentem
} // namespace Qentem

#endif
