
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
class Array {
  private:
    UNumber _capacity = 0;

    static void _init(const Array<T> *from, Array<T> *to) noexcept {
        if (from->Size != 0) {
            to->Storage = new T[from->Size];

            for (UNumber n = 0; n < from->Size; n++) {
                to->Storage[n] = from->Storage[n];
            }
        }

        to->_capacity = from->Size;
        to->Size      = from->Size;
    }

  public:
    UNumber Size    = 0;
    T *     Storage = nullptr;

    explicit Array() noexcept = default;

    void Move(Array<T> &src) noexcept {
        delete[] this->Storage;
        this->Storage   = src.Storage;
        this->_capacity = src._capacity;
        this->Size      = src.Size;

        src._capacity = 0;
        src.Size      = 0;
        src.Storage   = nullptr;
    }

    Array(Array<T> &&src) noexcept {
        Move(src);
    }

    Array(const Array<T> &src) noexcept {
        if ((this != &src) && (src._capacity != 0)) {
            Array::_init(&src, this);
        }
    }

    Array<T> &operator=(Array<T> &&src) noexcept {
        if (this != &src) {
            Move(src);
        }
        return *this;
    }

    Array<T> &operator=(const Array<T> &src) noexcept {
        if (this != &src) {
            delete[] this->Storage;
            Array::_init(&src, this);
        }
        return *this;
    }

    void Expand() noexcept {
        if (this->_capacity == 0) {
            this->_capacity = 1;
        } else {
            this->_capacity *= 2;
        }

        T *tmp        = this->Storage;
        this->Storage = new T[this->_capacity];

        for (UNumber n = 0; n < this->Size; n++) {
            this->Storage[n] = static_cast<T &&>(tmp[n]);
        }

        delete[] tmp;
    }

    void ExpandTo(const UNumber size) noexcept {
        this->_capacity += size;
        T *tmp = this->Storage;

        this->Storage = new T[this->_capacity];

        for (UNumber n = 0; n < this->Size; n++) {
            this->Storage[n] = static_cast<T &&>(tmp[n]);
        }

        delete[] tmp;
    }

    Array<T> &Add(Array<T> &src, bool move = false) noexcept {
        if (src.Size != 0) {
            if ((this->Size + src.Size) > this->_capacity) {
                this->_capacity += src.Size;

                if (this->Size != 0) {
                    T *tmp        = this->Storage;
                    this->Storage = new T[this->_capacity];

                    for (UNumber n = 0; n < this->Size; n++) {
                        this->Storage[n] = static_cast<T &&>(tmp[n]);
                    }

                    delete[] tmp;
                } else {
                    this->Storage = new T[this->_capacity];
                }
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
        }

        return *this;
    }

    void SetCapacity(const UNumber size) noexcept {
        delete[] this->Storage;
        this->Storage = nullptr;

        this->Size      = 0;
        this->_capacity = size;

        if (size != 0) {
            this->Storage = new T[size];
        }
    }

    T &Last(bool expand) noexcept { // TODO: Remove "expand" and  add "increment" to prevent forgetting to increase index
        if (!expand) {
            return this->Storage[(this->Size - 1)];
        }

        if (this->Size == this->_capacity) {
            Expand();
        }

        return this->Storage[this->Size];
    }

    Array<T> &Add(T &&item) noexcept { // Move
        if (this->Size == this->_capacity) {
            Expand();
        }

        this->Storage[this->Size] = item;
        this->Size++;

        return *this;
    }

    Array<T> &Add(const T &item) noexcept { // Copy
        if (this->Size == this->_capacity) {
            Expand();
        }

        this->Storage[this->Size] = item;
        this->Size++;

        return *this;
    }

    // constexpr
    T &operator[](const UNumber id) const {
        if (id < this->_capacity) {
            return this->Storage[id];
        }

        throw;
    }

    void Clear() noexcept {
        delete[] this->Storage;
        this->Storage   = nullptr;
        this->_capacity = 0;
        this->Size      = 0;
    }

    virtual ~Array() noexcept {
        Clear();
    }

}; // namespace Qentem
} // namespace Qentem

#endif
