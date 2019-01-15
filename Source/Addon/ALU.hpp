
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
    Expressions ParenExprs;
    Expressions MathExprs;
    Expression  ParenExpr = Expression();
    Expression  ParenTail = Expression();

    Expression MathEqu = Expression();
    Expression MathAdd = Expression();
    Expression MathMul = Expression();

    explicit ALU() noexcept;
    float Evaluate(String &content) noexcept;

    static String ParenthesisCallback(const String &block, const Match &match) noexcept;
    static String EqualCallback(const String &block, const Match &match) noexcept;
    static String MultiplicationCallback(const String &block, const Match &match) noexcept;
    static String AdditionCallback(const String &block, const Match &match) noexcept;
};
} // namespace Qentem

#endif
