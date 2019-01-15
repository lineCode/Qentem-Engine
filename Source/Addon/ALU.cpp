
/**
 * Qentem ALU
 *
 * @brief     For arithmetic and logic evaluation.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Addon/ALU.hpp"

using Qentem::Engine::Expression;
using Qentem::Engine::Match;

Qentem::ALU::ALU() noexcept {
    ParenExpr.Keyword = L"(";
    ParenTail.Keyword = L")";
    ParenExpr.Tail    = &ParenTail;
    ParenExpr.Flag    = Qentem::Engine::Flags::BUBBLE;
    ParenExpr.NestExprs.Add(&ParenExpr);
    ParenExpr.ParseCB = &ParenthesisCallback;
    this->Parens.Add(&ParenExpr);

    // start with ()
    // pop to = < > !
    // (/d+[*\/]/d+)
    /// Pop to + -
    // pop to
}

// e.g. ( 4 + 3 ), ( 2 + ( 4 + ( 1 + 2 ) + 1 ) * 5 - 3 - 2 )
String Qentem::ALU::ParenthesisCallback(const String &block, const Match &match) noexcept {
    String result = block;
    ALU::Execute(result);
    return result;
}

float Qentem::ALU::Evaluate(String &content) noexcept {
    /**
     *
     * e.g. ((2* (1 * 3)) + 1 - 4) + ((10 - 5) - 6 + ((1 + 1) + (1 + 1))) * (8 / 4 + 1) - (1) - (-1) +
     * 2 = 14 e.g. (6 + 1 - 4) + (5 - 6 + 4) * (8 / 4 + 1) - (1) - (-1) + 2 = 14 e.g. 3 + 3 * 3 - 1 +
     * 1 + 2 = 14 e.g. 3 + 9 - 1 + 1 + 2 = 14 e.g. 14 = 14 e.g. 1; means true.
     *
     * Steps:
     * First: Look for parenthesis ( operation or number )
     * Second: Process opreations: * and /
     * Third: Process : + and -
     * Forth: Process logic ( = != > < )
     * Fifth: Return final value or 0;
     */

    content = Engine::Parse(content, Engine::Search(content, this->Parens));

    if (content.Length != 0) {
        return ALU::Execute(content);
    }

    return 0;
}

const Qentem::ALU::Operation Qentem::ALU::GetNextOp(const String &content, size_t &i) noexcept {
    while (i < content.Length) {
        switch (content.Str[i++]) {
            case '+':
                return Operation::Add;
            case '-':
                return Operation::Sub;
            case '*':
                return Operation::Mul;
            case '/':
                return Operation::Div;
            case '=':
                return Operation::Equ;
            case '!':
                return Operation::NEq;
            case '>': {
                return Operation::Big;
            }
            case '<': {
                return Operation::Les;
            }
        }
    }
    return Operation::None;
}

float Qentem::ALU::Calculate(const float left, const float right, const Operation operation) noexcept {
    switch (operation) {
        case Operation::Add:
            return (left + right);
        case Operation::Sub:
            return (left - right);
        case Operation::Mul:
            return (left * right);
        case Operation::Div:
            if (right == 0.0f) {
                return 0.0f;
            }
            return (left / right);
        case Operation::Equ:
            return (left == right) ? 1.0f : 0.0f;
        case Operation::NEq:
            return (left != right) ? 1.0f : 0.0f;
        case Operation::Big:
            return (left > right) ? 1.0f : 0.0f;
        case Operation::Les:
            return (left < right) ? 1.0f : 0.0f;
        case Operation::BEq:
            return (left >= right) ? 1.0f : 0.0f;
        case Operation::LEq:
            return (left <= right) ? 1.0f : 0.0f;
        default:
            return 0.0f;
    }
}

float Qentem::ALU::Execute(String &content) noexcept {

    content = L"0";
    return 0;
}
