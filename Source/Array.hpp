
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

#include "Global.hpp"

namespace Qentem {

template <typename T>
class Array {
  private:
    size_t _capacity = 0;

    static void _init(const Array<T> *from, Array<T> *to) noexcept {
        if (from->Size != 0) {
            to->Storage = new T[from->Size];

            for (size_t n = 0; n < from->Size; n++) {
                to->Storage[n] = from->Storage[n];
            }
        }

        to->_capacity = from->Size;
        to->Size      = from->Size;
    }

  public:
    size_t Size      = 0;
    T *    Storage   = nullptr;
    explicit Array() = default;

    Array(const Array<T> &src) noexcept {
        if ((this != &src) && (src._capacity != 0)) {
            Array::_init(&src, this);
        }
    }

    explicit Array(const size_t size) noexcept {
        if (size != 0) {
            this->_capacity = size;
            this->Storage   = new T[size];
        }
    }

    Array(Array<T> &&src) noexcept {
        if (src.Size != 0) {
            this->Storage   = src.Storage;
            this->_capacity = src._capacity;
            this->Size      = src.Size;

            src._capacity = 0;
            src.Size      = 0;
            src.Storage   = nullptr;
        }
    }

    inline virtual ~Array() noexcept {
        delete[] this->Storage;
    }

    Array<T> &Add(const Array<T> &src) noexcept {
        if (src.Size != 0) {
            if ((this->Size + src.Size) > this->_capacity) {
                this->_capacity += src.Size;

                auto *tmp = new T[this->_capacity];
                for (size_t n = 0; n < this->Size; n++) {
                    tmp[n] = this->Storage[n];
                }

                delete[] this->Storage;
                this->Storage = tmp;
            }

            for (size_t i = 0; i < src.Size; i++) {
                this->Storage[this->Size++] = src.Storage[i];
            }
        }

        return *this;
    }

    Array<T> &Add(const T &item) noexcept { // Do not add move, it will break CPU::prefetch
        if (this->Size == this->_capacity) {
            if (this->_capacity == 0) {
                this->_capacity = 1;
            } else {
                this->_capacity *= 2;
            }

            auto *tmp = new T[this->_capacity];
            for (size_t n = 0; n < this->Size; n++) {
                tmp[n] = this->Storage[n];
            }

            delete[] this->Storage;
            this->Storage = tmp;
        }

        *(this->Storage + this->Size++) = item;

        return *this;
    }

    Array<T> &operator=(const Array<T> &src) noexcept {
        if ((this != &src) && (src._capacity != 0)) {
            Array::_init(&src, this);
        }
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

    inline T &operator[](const size_t id) const {
        if (id >= this->Size) {
            throw;
        }

        return *(this->Storage + id);
    }
};
} // namespace Qentem

#endif
