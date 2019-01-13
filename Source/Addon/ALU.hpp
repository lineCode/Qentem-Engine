
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

using Qentem::Engine::Expression;
using Qentem::Engine::Expressions;
using Qentem::Engine::Match;

namespace Qentem {

struct ALU {
    enum Operation { None = 0, Mul, Div, Add, Sub, Equ, NEq, Big, Les, BEq, LEq };

    Expressions Parens;
    Expression  ParenExpr = Expression();
    Expression  ParenTail = Expression();

    explicit ALU() noexcept;
    float Evaluate(String &content) noexcept;

    static String          ParenthesisCallback(const String &block, const Match &match) noexcept;
    static const Operation GetNextOp(const String &content, size_t &i) noexcept;
    static float           Calculate(const float left, const float right, const Operation operation) noexcept;
    static float           Execute(String &content) noexcept;
};
} // namespace Qentem

#endif
