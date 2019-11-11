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
    String r(Engine::Parse(item.NestMatch, block, item.Offset, item.Length));
    String::ToNumber(number, r.Str, 0, r.Length);
}

static String LogicCallback(wchar_t const *block, Match const &item, UNumber const length, void *other) noexcept {
    double number1 = 0.0;
    double number2 = 0.0;

    Match *m2;

    Match const *m1    = &(item.NestMatch[0]);
    UShort       op_id = m1->Expr->ID;

    if (m1->NestMatch.Size != 0) {
        NestNumber(number1, block, *m1);
    } else {
        String::ToNumber(number1, block, m1->Offset, m1->Length);
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

    String::ToNumber(number1, block, m1->Offset, m1->Length);

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

    if (m1->NestMatch.Size != 0) {
        NestNumber(number1, block, *m1);
    } else {
        String::ToNumber(number1, block, m1->Offset, m1->Length);
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

static Expressions const &getMathExpres() noexcept {
    static constexpr UNumber flags_no_pop = Flags::SPLIT | Flags::GROUPED | Flags::TRIM;
    static constexpr UNumber flags_ops    = flags_no_pop | Flags::POP;

    static Expressions expres(2);

    if (expres.Size == 0) {
        static Expression math_mul;
        math_mul.SetKeyword(L"*");
        math_mul.ID      = 1;
        math_mul.Flag    = flags_no_pop;
        math_mul.ParseCB = &(MultiplicationCallback);

        static Expression math_div;
        math_div.SetKeyword(L"/");
        math_div.ID      = 2;
        math_div.Flag    = flags_no_pop;
        math_div.ParseCB = &(MultiplicationCallback);

        static Expression math_exp;
        math_exp.SetKeyword(L"^");
        math_exp.ID      = 3;
        math_exp.Flag    = flags_no_pop;
        math_exp.ParseCB = &(MultiplicationCallback);

        static Expression math_rem;
        math_rem.SetKeyword(L"%");
        math_rem.ID      = 4;
        math_rem.Flag    = flags_no_pop;
        math_rem.ParseCB = &(MultiplicationCallback);
        ///////////////////////////////////////////
        static Expression math_add;
        math_add.SetKeyword(L"+");
        math_add.ID      = 1;
        math_add.Flag    = flags_ops;
        math_add.ParseCB = &(AdditionCallback);
        math_add.NestExprs.SetCapacity(4);
        math_add.NestExprs.Add(&math_exp).Add(&math_rem).Add(&math_div).Add(&math_mul);

        static Expression math_sub;
        math_sub.SetKeyword(L"-");
        math_sub.ID        = 2;
        math_sub.Flag      = flags_no_pop;
        math_sub.ParseCB   = &(AdditionCallback);
        math_sub.NestExprs = math_add.NestExprs;
        ///////////////////////////////////////////
        static Expression logic_equ2;
        logic_equ2.SetKeyword(L"==");
        logic_equ2.ID      = 1;
        logic_equ2.Flag    = flags_ops;
        logic_equ2.ParseCB = &(EqualCallback);
        logic_equ2.NestExprs.Add(&math_add).Add(&math_sub);

        static Expression logic_equ;
        logic_equ.SetKeyword(L"=");
        logic_equ.ID        = 2;
        logic_equ.Flag      = flags_ops;
        logic_equ.ParseCB   = &(EqualCallback);
        logic_equ.NestExprs = logic_equ2.NestExprs;

        static Expression logic_not_equ;
        logic_not_equ.SetKeyword(L"!=");
        logic_not_equ.ID        = 3;
        logic_not_equ.Flag      = flags_no_pop;
        logic_not_equ.ParseCB   = &(EqualCallback);
        logic_not_equ.NestExprs = logic_equ2.NestExprs;

        static Expression logic_less_equ;
        logic_less_equ.SetKeyword(L"<=");
        logic_less_equ.ID        = 4;
        logic_less_equ.Flag      = flags_no_pop;
        logic_less_equ.ParseCB   = &(EqualCallback);
        logic_less_equ.NestExprs = logic_equ2.NestExprs;

        static Expression logic_less;
        logic_less.SetKeyword(L"<");
        logic_less.ID        = 5;
        logic_less.Flag      = flags_no_pop;
        logic_less.ParseCB   = &(EqualCallback);
        logic_less.NestExprs = logic_equ2.NestExprs;

        static Expression logic_big_equ;
        logic_big_equ.SetKeyword(L">=");
        logic_big_equ.ID        = 6;
        logic_big_equ.Flag      = flags_no_pop;
        logic_big_equ.ParseCB   = &(EqualCallback);
        logic_big_equ.NestExprs = logic_equ2.NestExprs;

        static Expression logic_big;
        logic_big.SetKeyword(L">");
        logic_big.ID        = 7;
        logic_big.Flag      = flags_no_pop;
        logic_big.ParseCB   = &(EqualCallback);
        logic_big.NestExprs = logic_equ2.NestExprs;
        ///////////////////////////////////////////
        static Expression logic_and;
        logic_and.SetKeyword(L"&&");
        logic_and.ID      = 1;
        logic_and.Flag    = flags_ops;
        logic_and.ParseCB = &(LogicCallback);
        logic_and.NestExprs.SetCapacity(7);
        logic_and.NestExprs.Add(&logic_equ2)
            .Add(&logic_equ)
            .Add(&logic_not_equ)
            .Add(&logic_less_equ)
            .Add(&logic_less)
            .Add(&logic_big_equ)
            .Add(&logic_big);

        static Expression logic_or;
        logic_or.SetKeyword(L"||");
        logic_or.ID        = 2;
        logic_or.Flag      = flags_no_pop;
        logic_or.ParseCB   = &(LogicCallback);
        logic_or.NestExprs = logic_and.NestExprs;
        ///////////////////////////////////////////

        expres.Add(&logic_and).Add(&logic_or);
    }

    return expres;
}

// e.g. ( 4 + 3 ), ( 2 + ( 4 + ( 1 + 2 ) + 1 ) * 5 - 3 - 2 )
static String ParenthesisCallback(wchar_t const *block, Match const &item, UNumber const length, void *other) noexcept {
    UNumber const limit = (length - 2);
    return Engine::Parse(Engine::Search(getMathExpres(), block, 1, limit), block, 1, limit);
}

static Expressions const &getParensExpres() noexcept {
    static Expression  ParensExpr;
    static Expression  ParensCLose;
    static Expressions expres(1);

    if (expres.Size == 0) {
        ParensExpr.SetKeyword(L"(");
        ParensCLose.SetKeyword(L")");
        ParensExpr.Connected = &ParensCLose;
        ParensCLose.Flag     = Flags::BUBBLE | Flags::TRIM;
        ParensCLose.ParseCB  = &(ParenthesisCallback);
        ParensCLose.NestExprs.SetCapacity(1);
        ParensCLose.NestExprs.Add(&ParensExpr);

        expres.Add(&ParensExpr);
    }

    return expres;
}

static double Evaluate(wchar_t const *content, UNumber const offset, UNumber const limit) noexcept {
    /**
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
    String n_content(Engine::Parse(Engine::Search(getParensExpres(), content, offset, limit), content, offset, limit));
    if ((n_content.Length == 0) || (n_content == L"0")) {
        return 0.0;
    }

    // The rest:
    double num = 0.0;
    n_content  = Engine::Parse(Engine::Search(getMathExpres(), n_content.Str, 0, n_content.Length), n_content.Str, 0, n_content.Length);
    if ((n_content.Length != 0) && String::ToNumber(num, n_content.Str, 0, n_content.Length)) {
        return num;
    }

    return 0.0;
}

} // namespace ALU
} // namespace Qentem

#endif
