
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
using Qentem::Engine::Flags;
using Qentem::Engine::Match;

Qentem::ALU::ALU() noexcept {
    ParenExpr.Keyword = L"(";
    ParenTail.Keyword = L")";
    ParenExpr.Tail    = &ParenTail;
    ParenExpr.Flag    = Flags::BUBBLE;
    ParenExpr.NestExprs.Add(&ParenExpr);
    ParenExpr.ParseCB = &(ALU::ParenthesisCallback);
    ParenExpr.Pocket  = &(this->MathExprs);

    this->ParenExprs.Add(&ParenExpr);

    MathMul.Keyword = L"*";
    MathMul.Flag    = Flags::SPLIT;
    MathMul.ParseCB = &(ALU::MultiplicationCallback);

    MathAdd.Keyword = L"+";
    MathAdd.Flag    = Flags::SPLIT | Flags::POP;
    MathAdd.NestExprs.Add(&MathMul);
    MathAdd.ParseCB = &(ALU::AdditionCallback);

    MathEqu.Keyword = L"=";
    MathEqu.Flag    = Flags::SPLIT | Flags::POP;
    MathEqu.NestExprs.Add(&MathAdd);
    MathEqu.ParseCB = &(ALU::EqualCallback);

    this->MathExprs.Add(&MathEqu);

    // start with = < > !
    // pop to * /
    // (/d+[*\/]/d+)
    /// Pop to + -
}

String Qentem::ALU::EqualCallback(const String &block, const Match &match) noexcept {
    bool op        = false;
    bool string_op = false;

    if (match.NestMatch.Size > 0) {
        Match *nm      = &(match.NestMatch[0]);
        float  temnum1 = 0.0f;
        float  temnum2 = 0.0f;
        String temstr1 = L"";
        String r       = block.Part(nm->Offset, nm->Length);

        if (r.Length == 0) {
            return L"0";
        }

        if (!String::ToNumber(r, temnum1)) {
            string_op = true;
            temstr1   = r;
        }

        for (size_t i = 1; i < match.NestMatch.Size; i++) {
            nm = &(match.NestMatch[i]);
            r  = block.Part(nm->Offset, nm->Length);

            if (string_op) {
                op = (String::Trim(temstr1) == String::Trim(r));
            } else {
                if (!String::ToNumber(r, temnum2)) {
                    break;
                }

                op = (temnum1 == temnum2);
            }

            if (!op) {
                break;
            }
        }
    }

    return op ? L"1" : L"0";
}

String Qentem::ALU::MultiplicationCallback(const String &block, const Match &match) noexcept {
    float number = 1.0f;

    if (match.NestMatch.Size > 0) {
        String r      = L"";
        float  temnum = 1.0f;

        Match *nm;
        for (size_t i = 0; i < match.NestMatch.Size; i++) {
            nm = &(match.NestMatch[i]);
            r  = block.Part(nm->Offset, nm->Length);

            if ((r.Length == 0) || !String::ToNumber(r, temnum)) {
                return L"0";
            }

            number *= temnum;
        }
    }

    return String::ToString(number);
}

String Qentem::ALU::AdditionCallback(const String &block, const Match &match) noexcept {
    float number = 0.0f;

    if (match.NestMatch.Size > 0) {
        String r      = L"";
        float  temnum = 0.0f;

        Match *nm;
        for (size_t i = 0; i < match.NestMatch.Size; i++) {
            nm = &(match.NestMatch[i]);

            if (nm->NestMatch.Size != 0) {
                r = Engine::Parse(block, nm->NestMatch, nm->Offset, nm->Offset + nm->Length);
            } else {
                r = block.Part(nm->Offset, nm->Length);
            }

            if ((r.Length == 0) || !String::ToNumber(r, temnum)) {
                return L"0";
            }

            number += temnum;
        }
    }

    return String::ToString(number);
}

// e.g. ( 4 + 3 ), ( 2 + ( 4 + ( 1 + 2 ) + 1 ) * 5 - 3 - 2 )
String Qentem::ALU::ParenthesisCallback(const String &block, const Match &match) noexcept {
    String result = block.Part(match.OLength, (block.Length - (match.OLength + match.CLength)));
    return Engine::Parse(result, Engine::Search(result, *((Expressions *)match.Expr->Pocket)));
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

    content = Engine::Parse(content, Engine::Search(content, this->ParenExprs));

    if ((content.Length == 0) || (content == L"0")) {
        return 0;
    }

    content = Engine::Parse(content, Engine::Search(content, this->MathExprs));

    float num = 0;
    if ((content.Length == 0) || (content == L"0") || !String::ToNumber(content, num)) {
        return 0;
    }

    return num;
}
