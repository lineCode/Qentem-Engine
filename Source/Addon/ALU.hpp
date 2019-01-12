
/**
 * Qentem ALU
 *
 * @brief     For arithmetic and logic evaluation.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#ifndef QENTEM_ALU_H
#define QENTEM_ALU_H

#include "Engine.hpp"
#include "Global.hpp"

using Qentem::Engine::Expressions;
using Qentem::Engine::Match;

namespace Qentem {

struct ALU {
    enum Operation { None = 0, Mul, Div, Add, Sub, Equ, NEq, Big, Les, BEq, LEq };

    static bool        _initialized;
    static Expressions Ops;

    static void            Init() noexcept;
    static String          ParenthesisCallback(const String &block, const Match &match) noexcept;
    static const Operation GetNextOp(const String &content, size_t &i) noexcept;
    static const float     Evaluate(String &content) noexcept;
    static const float     Calculate(const float left, const float right, const Operation operation) noexcept;
    static const float     Execute(String &content) noexcept;
};
} // namespace Qentem

#endif
