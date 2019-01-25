
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

    Array(Array<T> &&src) noexcept {
        if (this != &src) {
            // Move
            this->Storage   = src.Storage;
            this->_capacity = src._capacity;
            this->Size      = src.Size;
            src._capacity   = 0;
            src.Size        = 0;
            src.Storage     = nullptr;
        }
    }

    Array(const Array<T> &src) noexcept {
        if ((this != &src) && (src._capacity != 0)) {
            Array::_init(&src, this);
        }
    }

    Array(T &&item) noexcept {
        this->Add(item);
    }

    Array(const T &item) noexcept {
        this->Add(item);
    }

    void SetSize(UNumber size) noexcept {
        delete[] this->Storage;
        this->Storage = nullptr;

        this->Size      = 0;
        this->_capacity = size;

        if (size != 0) {
            this->Storage = new T[size];
        }
    }

    inline virtual ~Array() noexcept {
        delete[] this->Storage;
    }

    inline void Clear() noexcept {
        delete[] this->Storage;
        this->Storage   = nullptr;
        this->_capacity = 0;
        this->Size      = 0;
    }

    void Move(Array<T> &src) noexcept {
        if (this != &src) {
            // Move
            delete[] this->Storage;
            this->Storage   = src.Storage;
            this->_capacity = src._capacity;
            this->Size      = src.Size;

            src._capacity = 0;
            src.Size      = 0;
            src.Storage   = nullptr;
        }
    }

    Array<T> &Add(const Array<T> &src) noexcept {
        if (src.Size != 0) {
            if ((this->Size + src.Size) > this->_capacity) {
                this->_capacity += src.Size;

                auto tmp = new T[this->_capacity];
                for (UNumber n = 0; n < this->Size; n++) {
                    tmp[n] = this->Storage[n];
                }

                delete[] this->Storage;
                this->Storage = tmp;
            }

            for (UNumber i = 0; i < src.Size; i++) {
                this->Storage[this->Size++] = src.Storage[i];
            }
        }

        return *this;
    }

    Array<T> &Add(const T &item) noexcept {
        if (this->Size == this->_capacity) {
            if (this->_capacity == 0) {
                this->_capacity = 1;
            } else {
                this->_capacity *= 2;
            }

            auto tmp = new T[this->_capacity];
            for (UNumber n = 0; n < this->Size; n++) {
                tmp[n] = this->Storage[n];
            }

            delete[] this->Storage;
            this->Storage = tmp;
        }

        *(this->Storage + this->Size++) = item;

        return *this;
    }

    Array<T> &Add(T &&item) noexcept {
        if (this->Size == this->_capacity) {
            if (this->_capacity == 0) {
                this->_capacity = 1;
            } else {
                this->_capacity *= 2;
            }

            auto tmp = new T[this->_capacity];
            for (UNumber n = 0; n < this->Size; n++) {
                tmp[n] = this->Storage[n];
            }

            delete[] this->Storage;
            this->Storage = tmp;
        }

        *(this->Storage + this->Size++) = item;

        return *this;
    }

    Array<T> &operator=(Array<T> &&src) noexcept {
        if (this != &src) {
            // Move
            delete[] this->Storage;
            this->Storage   = src.Storage;
            this->_capacity = src._capacity;
            this->Size      = src.Size;

            src._capacity = 0;
            src.Size      = 0;
            src.Storage   = nullptr;
        }
        return *this;
    }

    Array<T> &operator=(const Array<T> &src) noexcept {
        if ((this != &src) && (src._capacity != 0)) {
            delete[] this->Storage;
            Array::_init(&src, this);
        }
        return *this;
    }

    inline T &operator[](const UNumber id) const {
        if (id >= this->Size) {
            throw;
        }

        return *(this->Storage + id);
    }
};
} // namespace Qentem

#endif
