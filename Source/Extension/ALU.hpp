/**
 * Qentem ALU
 *
 * @brief     For arithmetic and logic evaluation.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Engine.hpp"

#ifndef QENTEM_ALU_H
#define QENTEM_ALU_H

namespace Qentem {
namespace ALU {

using Engine::Expression;
using Engine::Expressions;
using Engine::Flags;
using Engine::Match;

static void NestNumber(double &number, wchar_t const *block, Match const &item) noexcept {
    String r(Engine::Parse(block, item.NestMatch, item.Offset, item.Length));
    if (r.Length != 0) {
        String::ToNumber(number, r.Str, 0, r.Length);
    }
}

static String LogicCallback(wchar_t const *block, Match const &item, UNumber const length, void *other) noexcept {
    double number1 = 0.0;
    double number2 = 0.0;

    Match *m2;

    Match const *m1    = &(item.NestMatch[0]);
    UShort       op_id = m1->Expr->ID;

    if (m1->Length != 0) {
        if (m1->NestMatch.Size != 0) {
            NestNumber(number1, block, *m1);
        } else {
            String::ToNumber(number1, block, m1->Offset, m1->Length);
        }
    }

    for (UNumber i = 1; i < item.NestMatch.Size; i++) {
        m2 = &(item.NestMatch[i]);

        if (m2->Length != 0) {
            if (m2->NestMatch.Size != 0) {
                NestNumber(number2, block, *m2);
            } else {
                String::ToNumber(number2, block, m2->Offset, m2->Length);
            }

            switch (op_id) {
                case 1:
                    number1 = ((number1 > 0) && (number2 > 0)) ? 1.0 : 0.0;
                    break;
                case 2:
                    number1 = ((number1 > 0) || (number2 > 0)) ? 1.0 : 0.0;
                    break;
                default:
                    number1 = 0.0;
                    break;
            }
        }

        op_id = m2->Expr->ID;
    }

    return String::FromNumber(number1, 1, 0, 3);
}

static String EqualCallback(wchar_t const *block, Match const &item, UNumber const length, void *other) noexcept {
    double number1 = 0.0;
    double number2 = 0.0;

    Match *m2;

    Match const *m1    = &(item.NestMatch[0]);
    UShort       op_id = m1->Expr->ID;

    if (m1->Length != 0) {
        if (m1->NestMatch.Size != 0) {
            NestNumber(number1, block, *m1);
        } else {
            wchar_t const c = block[m1->Offset];
            if (((c <= 57) && (c >= 48)) || ((c == L'+') || (c == L'-'))) {
                String::ToNumber(number1, block, m1->Offset, m1->Length);
            } else if (item.NestMatch.Size == 2) { // String
                if (String::Compare(block, m1->Offset, m1->Length, block, item.NestMatch[1].Offset, item.NestMatch[1].Length)) {
                    return L"1";
                }

                return L"0";
            }
        }
    }

    for (UNumber i = 1; i < item.NestMatch.Size; i++) {
        m2 = &(item.NestMatch[i]);

        if (m2->Length != 0) {
            if (m2->NestMatch.Size != 0) {
                NestNumber(number2, block, *m2);
            } else {
                String::ToNumber(number2, block, m2->Offset, m2->Length);
            }

            switch (op_id) {
                case 1:
                case 2:
                    number1 = (number1 == number2) ? 1.0 : 0.0;
                    break;
                case 3:
                    number1 = (number1 != number2) ? 1.0 : 0.0;
                    break;
                case 4:
                    number1 = (number1 <= number2) ? 1.0 : 0.0;
                    break;
                case 5:
                    number1 = (number1 < number2) ? 1.0 : 0.0;
                    break;
                case 6:
                    number1 = (number1 >= number2) ? 1.0 : 0.0;
                    break;
                case 7:
                    number1 = (number1 > number2) ? 1.0 : 0.0;
                    break;
                default:
                    number1 = 0.0;
                    break;
            }
        }

        op_id = m2->Expr->ID;
    }

    return String::FromNumber(number1, 1, 0, 3);
}

static String MultiplicationCallback(wchar_t const *block, Match const &item, UNumber const length, void *other) noexcept {
    double number1 = 0.0;
    double number2 = 0.0;

    Match *m2;

    Match const *m1    = &(item.NestMatch[0]);
    UShort       op_id = m1->Expr->ID;

    if (m1->Length != 0) {
        String::ToNumber(number1, block, m1->Offset, m1->Length);
    }

    for (UNumber i = 1; i < item.NestMatch.Size; i++) {
        m2 = &(item.NestMatch[i]);

        if (m2->Length != 0) {

            String::ToNumber(number2, block, m2->Offset, m2->Length);

            switch (op_id) {
                case 1: // *
                    number1 *= number2;
                    break;
                case 2: // /
                    if (number2 == 0) {
                        return L"0";
                    }

                    number1 /= number2;
                    break;
                case 3: // ^
                    if (number2 != 0.0) {
                        UNumber const times = static_cast<UNumber>(number2);
                        for (UNumber k = 1; k < times; k++) {
                            number1 *= number1;
                        }

                        if (number2 < 0) {
                            number1 = 1 / number1;
                        }

                        break;
                    }

                    number1 = 1;
                    break;
                case 4: // &
                    number1 = static_cast<double>(static_cast<UNumber>(number1) % static_cast<UNumber>(number2));
                    break;
                default:
                    number1 = 0.0;
                    break;
            }
        } else {
            return L"0";
        }

        op_id = m2->Expr->ID;
    }

    return String::FromNumber(number1, 1, 0, 3);
}

static String AdditionCallback(wchar_t const *block, Match const &item, UNumber const length, void *other) noexcept {
    double number1 = 0.0;
    double number2 = 0.0;

    Match *m2;

    Match const *m1    = &(item.NestMatch[0]);
    UShort       op_id = m1->Expr->ID;

    if (m1->Length != 0) {
        if (m1->NestMatch.Size != 0) {
            NestNumber(number1, block, *m1);
        } else {
            String::ToNumber(number1, block, m1->Offset, m1->Length);
        }
    }

    for (UNumber i = 1; i < item.NestMatch.Size; i++) {
        m2 = &(item.NestMatch[i]);

        if (m2->Length != 0) {

            if (m2->NestMatch.Size != 0) {
                NestNumber(number2, block, *m2);
            } else {
                String::ToNumber(number2, block, m2->Offset, m2->Length);
            }

            switch (op_id) {
                case 1:
                    number1 += number2;
                    break;
                case 2:
                    number1 -= number2;
                    break;
                default:
                    number1 = 0.0;
                    break;
            }

            op_id = m2->Expr->ID;
        } else if ((op_id == 2) && (op_id == m2->Expr->ID)) {
            op_id = 1; // Two --
        } else {
            op_id = m2->Expr->ID;
        }
    }

    return String::FromNumber(number1, 1, 0, 3);
}

static Expressions const &getMathExprs() noexcept {
    static Expression MathExp;
    static Expression MathRem;
    static Expression MathDiv;
    static Expression MathMul;

    static Expression MathAdd;
    static Expression MathSub;

    static Expression MathEqu2;
    static Expression MathEqu;
    static Expression MathNEqu;
    static Expression MathLEqu;
    static Expression MathLess;
    static Expression MathBEqu;
    static Expression MathBig;

    static Expression LogicAnd;
    static Expression LogicOr;

    static constexpr UNumber flags_no_pop = Flags::SPLIT | Flags::GROUPED | Flags::TRIM;
    static constexpr UNumber flags_ops    = flags_no_pop | Flags::POP;

    static Expressions tags;

    if (tags.Size == 0) {
        MathMul.SetKeyword(L"*");
        MathMul.ID      = 1;
        MathMul.Flag    = flags_no_pop;
        MathMul.ParseCB = &(MultiplicationCallback);

        MathDiv.SetKeyword(L"/");
        MathDiv.ID      = 2;
        MathDiv.Flag    = flags_no_pop;
        MathDiv.ParseCB = &(MultiplicationCallback);

        MathExp.SetKeyword(L"^");
        MathExp.ID      = 3;
        MathExp.Flag    = flags_no_pop;
        MathExp.ParseCB = &(MultiplicationCallback);

        MathRem.SetKeyword(L"%");
        MathRem.ID      = 4;
        MathRem.Flag    = flags_no_pop;
        MathRem.ParseCB = &(MultiplicationCallback);
        ///////////////////////////////////////////
        MathAdd.SetKeyword(L"+");
        MathAdd.ID      = 1;
        MathAdd.Flag    = flags_ops;
        MathAdd.ParseCB = &(AdditionCallback);
        MathAdd.NestExprs.SetCapacity(4);
        MathAdd.NestExprs.Add(&MathExp).Add(&MathRem).Add(&MathDiv).Add(&MathMul);

        MathSub.SetKeyword(L"-");
        MathSub.ID        = 2;
        MathSub.Flag      = flags_no_pop;
        MathSub.ParseCB   = &(AdditionCallback);
        MathSub.NestExprs = MathAdd.NestExprs;
        ///////////////////////////////////////////
        MathEqu2.SetKeyword(L"==");
        MathEqu2.ID      = 1;
        MathEqu2.Flag    = flags_ops;
        MathEqu2.ParseCB = &(EqualCallback);
        MathEqu2.NestExprs.Add(&MathAdd).Add(&MathSub);

        MathEqu.SetKeyword(L"=");
        MathEqu.ID        = 2;
        MathEqu.Flag      = flags_ops;
        MathEqu.ParseCB   = &(EqualCallback);
        MathEqu.NestExprs = MathEqu2.NestExprs;

        MathNEqu.SetKeyword(L"!=");
        MathNEqu.ID        = 3;
        MathNEqu.Flag      = flags_no_pop;
        MathNEqu.ParseCB   = &(EqualCallback);
        MathNEqu.NestExprs = MathEqu2.NestExprs;

        MathLEqu.SetKeyword(L"<=");
        MathLEqu.ID        = 4;
        MathLEqu.Flag      = flags_no_pop;
        MathLEqu.ParseCB   = &(EqualCallback);
        MathLEqu.NestExprs = MathEqu2.NestExprs;

        MathLess.SetKeyword(L"<");
        MathLess.ID        = 5;
        MathLess.Flag      = flags_no_pop;
        MathLess.ParseCB   = &(EqualCallback);
        MathLess.NestExprs = MathEqu2.NestExprs;

        MathBEqu.SetKeyword(L">=");
        MathBEqu.ID        = 6;
        MathBEqu.Flag      = flags_no_pop;
        MathBEqu.ParseCB   = &(EqualCallback);
        MathBEqu.NestExprs = MathEqu2.NestExprs;

        MathBig.SetKeyword(L">");
        MathBig.ID        = 7;
        MathBig.Flag      = flags_no_pop;
        MathBig.ParseCB   = &(EqualCallback);
        MathBig.NestExprs = MathEqu2.NestExprs;
        ///////////////////////////////////////////
        LogicAnd.SetKeyword(L"&&");
        LogicAnd.ID      = 1;
        LogicAnd.Flag    = flags_ops;
        LogicAnd.ParseCB = &(LogicCallback);
        LogicAnd.NestExprs.SetCapacity(7);
        LogicAnd.NestExprs.Add(&MathEqu2).Add(&MathEqu).Add(&MathNEqu).Add(&MathLEqu).Add(&MathLess).Add(&MathBEqu).Add(&MathBig);

        LogicOr.SetKeyword(L"||");
        LogicOr.ID        = 2;
        LogicOr.Flag      = flags_no_pop;
        LogicOr.ParseCB   = &(LogicCallback);
        LogicOr.NestExprs = LogicAnd.NestExprs;
        ///////////////////////////////////////////

        tags.Add(&LogicAnd).Add(&LogicOr);
    }

    return tags;
}

// e.g. ( 4 + 3 ), ( 2 + ( 4 + ( 1 + 2 ) + 1 ) * 5 - 3 - 2 )
static String ParenthesisCallback(wchar_t const *block, Match const &item, UNumber const length, void *other) noexcept {
    static Expressions const &_mathExprs = getMathExprs();

    UNumber const limit = (length - 2);
    return Engine::Parse(block, Engine::Search(block, _mathExprs, 1, limit), 1, limit);
}

static Expressions const &getParensExprs() noexcept {
    static Expression  ParensExpr;
    static Expression  ParensNext;
    static Expressions tags;

    if (tags.Size == 0) {
        ParensExpr.SetKeyword(L"(");
        ParensNext.SetKeyword(L")");
        ParensExpr.Connected = &ParensNext;
        ParensNext.Flag      = Flags::BUBBLE | Flags::TRIM;
        ParensNext.ParseCB   = &(ParenthesisCallback);
        ParensNext.NestExprs = Expressions().Add(&ParensExpr);

        tags.SetCapacity(1);
        tags.Add(&ParensExpr);
    }

    return tags;
}

static double Evaluate(wchar_t const *content, UNumber const offset, UNumber const limit) noexcept {
    static Expressions const &_parensExprs = getParensExprs();
    static Expressions const &_mathExprs   = getMathExprs();

    /**
     *
     * e.g. ((2* (1 * 3)) + 1 - 4) + ((10 - 5) - 6 + ((1 + 1) + (1 + 1))) * (8 / 4 + 1) - (1) - (-1) + 2 == 14
     * e.g. (6 + 1 - 4) + (5 - 6 + 4) * (8 / 4 + 1) - (1) - (-1) + 2 = 14
     * e.g. 3 + 3 * 3 - 1 + 1 + 2 = 14
     * e.g. 3 + 9 - 1 - -1 + 2 == 14
     * e.g. 14 = 14
     * 1 means true.
     *
     * Steps:
     * 1: Look for parenthesis ( operation or number )
     * 2: Process opreations: * / % ^
     * 3: Process : + and -
     * 4: Process logic ( = != > < ... )
     * 5: Return final value or 0;
     */

    // Parenthesis:
    String _content(Engine::Parse(content, Engine::Search(content, _parensExprs, offset, limit), offset, limit));
    if ((_content.Length == 0) || (_content == L"0")) {
        return 0.0;
    }

    // The rest:
    double num = 0.0;
    _content   = Engine::Parse(_content.Str, Engine::Search(_content.Str, _mathExprs, 0, _content.Length), 0, _content.Length);
    if ((_content.Length != 0) && String::ToNumber(num, _content.Str, 0, _content.Length)) {
        return num;
    }

    return 0.0;
}

} // namespace ALU
} // namespace Qentem

#endif
