
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

namespace Qentem {

using Qentem::String;
using Qentem::Engine::Expression;
using Qentem::Engine::Expressions;
using Qentem::Engine::Match;

struct ALU {
    Expressions ParensExprs;
    Expressions MathExprs;
    Expression  ParensExpr = Expression();
    Expression  ParensNext = Expression();

    Expression MathEqu = Expression();
    Expression MathAdd = Expression();
    Expression MathMul = Expression();

    explicit ALU() noexcept;
    double Evaluate(String &content) const noexcept;

    static UNumber OR(const String &, const Expression &, Match *, UNumber &, UNumber &, UNumber) noexcept;

    static String ParenthesisCallback(const String &, const Match &) noexcept;
    static bool   NestNumber(const String &, const Match &, double &) noexcept;
    static String EqualCallback(const String &, const Match &) noexcept;
    static String MultiplicationCallback(const String &, const Match &) noexcept;
    static String AdditionCallback(const String &, const Match &) noexcept;
};
} // namespace Qentem

#endif
