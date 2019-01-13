
/**
 * Qentem ALU
 *
 * @brief     For arithmetic and logic evaluation.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "ALU.hpp"

using Qentem::Engine::Expression;
using Qentem::Engine::Match;

Qentem::ALU::ALU() noexcept {
    ParenExpr.Keyword = L"(";
    ParenTail.Keyword = L")";
    ParenExpr.Flag    = Qentem::Engine::Flags::BUBBLE;
    ParenExpr.NestExprs.Add(&ParenExpr);
    ParenExpr.Tail    = &ParenTail;
    ParenExpr.ParseCB = &ParenthesisCallback;
    this->Parens.Add(&ParenExpr);
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

// TODO: Needs complate rewrite! Hint: use custom search /d+[+|-]/d+=>Nest->/d+[*|//d+=>Nest->/d+^/d+
float Qentem::ALU::Execute(String &content) noexcept {
    // e.g.  4 + 3 = ?,  2 + 5 - 3 - 2 = ?,  2 * 5 - 4 / 2 = ?
    // Get left side number;
    // Get operation;
    // Get right side number;
    // place the result into the content
    // 1+2*3=7; 8/4=2;
    //
    Operation op        = Operation::Add; // Alway have the left side number and an op;
    size_t    i         = 0;
    size_t    string_op = 0;
    bool      num_op    = true;
    float     result    = 0; // Temp variable

    String left_side  = L"";
    String right_side = L"";

    // Feature: Allow: content = L"4+3*2"; //  10 - 5 * 2 = 0
    wchar_t c = '\0';
    for (; i <= content.Length; i++) { // Don't worry. Strings have '\0' at the end
        c = content.Str[i];

        if ((string_op == 0) && (c > 47) && (c < 58)) {
            // Feature: Might add GetNextNumber(const String &content, size_t &start_at)
            right_side += c;
        } else if ((op != Operation::None) && (right_side.Length != 0)) {
            if (string_op == 0) {
                num_op = true;
                // to do * / before + - and before =
                // Operation next_op = ALU::GetNextOp(content, i);
                // One way to do it, is by cutting the string and sed it back to this function:
                // content = L"1+3*2"; will be 1+ ALU::Execute((3*2))
                // but this will effect the result of == or != , as it needs the first part first.
                // But might split it at = to solve this problem.
                //
                // Nother way is to have parenthesis in the first place 1+(2*3).
                //
                result     = ALU::Calculate(result, String::ToNumber(right_side), op);
                op         = ALU::GetNextOp(content, i);
                right_side = L"";
                i--;
            } else if (left_side.Length == 0) {
                left_side  = right_side;
                right_side = L"";
                i--;
            } else {
                if ((i != content.Length) && (c != ' ')) {
                    right_side += c;
                } else {
                    if (op == Operation::Equ) {
                        result = ((left_side == right_side) ? 1.0f : 0.0f);
                    } else if (op == Operation::NEq) {
                        result = ((left_side != right_side) ? 1.0f : 0.0f);
                    } else {
                        content = L"0";
                        return 0;
                    }
                }
            }
        } else if ((op == Operation::None) && ((op = GetNextOp(content, i)) != Operation::None)) {
            if (content.Str[i + 1] == '=') {
                if (op == Operation::Big) {
                    op = Operation::BEq; // >=
                } else if (op == Operation::Les) {
                    op = Operation::LEq; // <=
                }
                // else: it should be !=
                i++;
            }
        } else if ((i != content.Length) && (c != ' ')) {
            if (c == '-') {
                if (op == Operation::Sub) {
                    op = Operation::Add;
                } else if (op == Operation::Add) {
                    op = Operation::Sub;
                } else {
                    result *= -1;
                }
            } else if (c == '+') {
                op = Operation::Add;
            } else if (num_op) {
                content = L"0";
                return 0;
            } else {
                right_side += c;
                if (string_op == 0.0f) {
                    string_op  = i;
                    op         = GetNextOp(content, i);
                    right_side = content.Part(string_op, (i - 1 - string_op));
                    string_op  = 1;
                    if (op == Operation::Equ) {
                        i--;
                    }
                }
            }
        }
    }

    if (result != 0) {
        if (string_op > 0) {
            content = L"1";
            return 1;
        }

        content = String::ToString(result);

        return result;
    }

    content = L"0";
    return 0;
}
