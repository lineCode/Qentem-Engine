
/**
 * Qentem Engine
 *
 * @brief     Array class for Qentem Engine
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#ifndef QENTEM_ARRAY_H
#define QENTEM_ARRAY_H

#include "Global.hpp"
// #include <initializer_list>
// #include <stdarg.h>

namespace Qentem {

// There is no .cpp file because of the use of template <type T>
template <typename T>
class Array {
  private:
    size_t _index    = 0;
    size_t _capacity = 0;

    static void _copy(const Array<T> *from, Array<T> *to) noexcept {
        if (to->Storage != nullptr) {
            // TODO:: I don't think this is necessary...
            delete[] to->Storage;
            to->Storage = nullptr;
        }

        if (from->_index != 0) {
            to->Storage = new T[from->_index];

            for (size_t n = 0; n < from->_index; n++) {
                to->Storage[n] = from->Storage[n];
            }
        }

        to->_capacity = from->_index;
        to->_index    = from->_index;
    }

  public:
    T *Storage       = nullptr;
    explicit Array() = default;

    Array(const Array<T> &src) noexcept {
        if ((this != &src) && (src._capacity != 0)) {
            Array::_copy(&src, this);
        }
    }

    // static Array<T> Group(T items, ...) noexcept {
    //     T *item = &items;

    //     va_list ptr;
    //     va_start(ptr, items);

    //     Array<T> arr = Array<T>(3);
    //     do {
    //         arr.Add(*item);
    //         item = va_arg(ptr, T *);
    //     } while (item != nullptr);

    //     va_end(ptr);

    //     return arr;
    // }

    // Array(const std::initializer_list<T> &items) noexcept {
    //     if (items.size() != 0) {
    //         this->_capacity = items.size();
    //         this->Storage  = new T[this->_capacity];

    //         const T *item = items.begin();
    //         while ((item != items.end())) {
    //             this->Add(*(item++));
    //         }
    //     }
    // }

    explicit Array(const size_t size) noexcept {
        if (size != 0) {
            this->_capacity = size;
            this->Storage   = new T[size];
        }
    }

    inline virtual ~Array() noexcept {
        delete[] this->Storage;
    }

    Array<T> &Add(const Array<T> &src) noexcept {
        if (src.Size() != 0) {
            if ((this->_index + src._index) > this->_capacity) {
                this->_capacity += src._index;

                T *tmp = new T[this->_capacity];
                for (size_t n = 0; n < this->_index; n++) {
                    tmp[n] = this->Storage[n];
                }

                delete[] this->Storage;
                this->Storage = tmp;
            }

            for (size_t i = 0; i < src._index; i++) {
                this->Storage[this->_index++] = src.Storage[i];
            }
        }

        return *this;
    }

    Array<T> &Add(const T &item) noexcept {
        if (this->_index == this->_capacity) {
            if (this->_capacity == 0) {
                this->_capacity = 1;
            } else {
                this->_capacity *= 2;
            }

            T *tmp = new T[this->_capacity];
            for (size_t n = 0; n < this->_index; n++) {
                tmp[n] = this->Storage[n];
            }

            delete[] this->Storage;
            this->Storage = tmp;
        }

        *(this->Storage + this->_index++) = item;

        return *this;
    }

    inline constexpr size_t Size() const noexcept {
        return this->_index;
    }

    Array<T> &operator=(const Array<T> &src) noexcept {
        if ((this != &src) && (src._capacity != 0)) {
            Array::_copy(&src, this);
        }
        return *this;
    }

    Array<T> &operator=(Array<T> &&src) noexcept {
        if (this != &src) {
            // Move
            delete[] this->Storage;
            this->Storage   = src.Storage;
            this->_capacity = src._capacity;
            this->_index    = src._index;

            src._capacity = 0;
            src._index    = 0;
            src.Storage   = nullptr;
        }
        return *this;
    }

    inline constexpr T &operator[](const size_t id) const noexcept {
        // if (id >= this->_index) {
        //     throw("index out of range!");
        // }

        return *(this->Storage + id);
    }
}; // namespace Qentem
} // namespace Qentem

#endif
