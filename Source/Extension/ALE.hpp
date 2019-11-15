/**
 * Qentem Arithmetic & logic Evaluator
 *
 * @brief     For arithmetic and logic evaluation.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Engine.hpp"

#ifndef QENTEM_ALE_H
#define QENTEM_ALE_H

namespace Qentem {
namespace ALE {

using Engine::Expression;
using Engine::Expressions;
using Engine::Flags;
using Engine::MatchBit;

static double Calculate(Array<MatchBit> const &items, wchar_t const *content, UNumber offset, UNumber limit) noexcept;

static double Logic(wchar_t const *block, MatchBit const &item) noexcept {
    double number1 = 0.0;
    double number2 = 0.0;

    MatchBit const *mb    = &(item.NestMatch[0]);
    UShort          op_id = mb->Expr->ID;

    if (mb->NestMatch.Size != 0) {
        number1 = Calculate(mb->NestMatch, block, mb->Offset, mb->Length);
    } else {
        String::ToNumber(number1, block, mb->Offset, mb->Length);
    }

    for (UNumber i = 1; i < item.NestMatch.Size; i++) {
        mb = &(item.NestMatch[i]);

        if (mb->Length != 0) {
            if (mb->NestMatch.Size != 0) {
                number2 = Calculate(mb->NestMatch, block, mb->Offset, mb->Length);
            } else {
                String::ToNumber(number2, block, mb->Offset, mb->Length);
            }

            switch (op_id) {
                case 1:
                    number1 = ((number1 > 0) && (number2 > 0)) ? 1.0 : 0.0;
                    break;
                case 2:
                    number1 = ((number1 > 0) || (number2 > 0)) ? 1.0 : 0.0;
                    break;
                default:
                    return 0.0;
            }
        }

        op_id = mb->Expr->ID;
    }

    return number1;
}

static double Equal(wchar_t const *block, MatchBit const &item) noexcept {
    double number1 = 0.0;
    double number2 = 0.0;

    MatchBit const *mb    = &(item.NestMatch[0]);
    UShort          op_id = mb->Expr->ID;

    if (mb->NestMatch.Size != 0) {
        number1 = Calculate(mb->NestMatch, block, mb->Offset, mb->Length);
    } else {
        wchar_t const c = block[mb->Offset];
        if (((c <= 57) && (c >= 48)) || ((c == L'+') || (c == L'-'))) {
            String::ToNumber(number1, block, mb->Offset, mb->Length);
        } else if (item.NestMatch.Size == 2) { // String
            if (String::Compare(block, mb->Offset, mb->Length, block, item.NestMatch[1].Offset, item.NestMatch[1].Length)) {
                return 1.0;
            }

            return 0.0;
        }
    }

    for (UNumber i = 1; i < item.NestMatch.Size; i++) {
        mb = &(item.NestMatch[i]);

        if (mb->Length != 0) {
            if (mb->NestMatch.Size != 0) {
                number2 = Calculate(mb->NestMatch, block, mb->Offset, mb->Length);
            } else {
                String::ToNumber(number2, block, mb->Offset, mb->Length);
            }

            switch (op_id) {
                case 3:
                case 4:
                    number1 = (number1 == number2) ? 1.0 : 0.0;
                    break;
                case 5:
                    number1 = (number1 != number2) ? 1.0 : 0.0;
                    break;
                case 6:
                    number1 = (number1 <= number2) ? 1.0 : 0.0;
                    break;
                case 7:
                    number1 = (number1 < number2) ? 1.0 : 0.0;
                    break;
                case 8:
                    number1 = (number1 >= number2) ? 1.0 : 0.0;
                    break;
                case 9:
                    number1 = (number1 > number2) ? 1.0 : 0.0;
                    break;
                default:
                    return 0.0;
            }
        }

        op_id = mb->Expr->ID;
    }

    return number1;
}

static double Add(wchar_t const *block, MatchBit const &item) noexcept {
    double number1 = 0.0;
    double number2 = 0.0;

    MatchBit const *mb    = &(item.NestMatch[0]);
    UShort          op_id = mb->Expr->ID;

    if (mb->NestMatch.Size != 0) {
        number1 = Calculate(mb->NestMatch, block, mb->Offset, mb->Length);
    } else {
        String::ToNumber(number1, block, mb->Offset, mb->Length);
    }

    for (UNumber i = 1; i < item.NestMatch.Size; i++) {
        mb = &(item.NestMatch[i]);

        if (mb->Length != 0) {

            if (mb->NestMatch.Size != 0) {
                number2 = Calculate(mb->NestMatch, block, mb->Offset, mb->Length);
            } else {
                String::ToNumber(number2, block, mb->Offset, mb->Length);
            }

            switch (op_id) {
                case 10:
                    number1 += number2;
                    break;
                case 11:
                    number1 -= number2;
                    break;
                default:
                    return 0.0;
            }

            op_id = mb->Expr->ID;
        } else if ((op_id == 11) && (op_id == mb->Expr->ID)) {
            op_id = 10; // Two --
        } else {
            op_id = mb->Expr->ID;
        }
    }

    return number1;
}

static double Multiply(wchar_t const *block, MatchBit const &item) noexcept {
    double number1 = 0.0;
    double number2 = 0.0;

    MatchBit const *mb    = &(item.NestMatch[0]);
    UShort          op_id = mb->Expr->ID;

    String::ToNumber(number1, block, mb->Offset, mb->Length);

    for (UNumber i = 1; i < item.NestMatch.Size; i++) {
        mb = &(item.NestMatch[i]);

        if (mb->Length != 0) {

            String::ToNumber(number2, block, mb->Offset, mb->Length);

            switch (op_id) {
                case 12: // *
                    number1 *= number2;
                    break;
                case 13: // /
                    if (number2 == 0) {
                        return 0.0;
                    }

                    number1 /= number2;
                    break;
                case 14: // ^
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
                case 15: // %
                    number1 = static_cast<double>(static_cast<UNumber>(number1) % static_cast<UNumber>(number2));
                    break;
                default:
                    return 0.0;
            }
        } else {
            return 0.0;
        }

        op_id = mb->Expr->ID;
    }

    return number1;
}

static double Calculate(Array<MatchBit> const &items, wchar_t const *content, UNumber offset, UNumber limit) noexcept {
    double num = 0.0;

    MatchBit const *item;
    for (UNumber id = 0; id < items.Size; id++) {
        item = &(items[id]);

        switch (item->Expr->ID) {
            case 1:
            case 2:
                num = Logic(content, *item);
                break;
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
                num = Equal(content, *item);
                break;
            case 10:
            case 11:
                num = Add(content, *item);
                break;
            case 12:
            case 13:
            case 14:
            case 15:
                num = Multiply(content, *item);
                break;
            default:
                break;
        }
    }

    return num;
}

static Expressions const &getMathExpres() noexcept {
    static constexpr UNumber flags_no_pop = Flags::SPLIT | Flags::GROUPED | Flags::TRIM;
    static constexpr UNumber flags_pop    = flags_no_pop | Flags::POP;

    static Expressions expres(2);

    if (expres.Size == 0) {
        static Expression math_mul;
        math_mul.SetKeyword(L"*");
        math_mul.ID   = 12;
        math_mul.Flag = flags_no_pop;

        static Expression math_div;
        math_div.SetKeyword(L"/");
        math_div.ID   = 13;
        math_div.Flag = flags_no_pop;

        static Expression math_exp;
        math_exp.SetKeyword(L"^");
        math_exp.ID   = 14;
        math_exp.Flag = flags_no_pop;

        static Expression math_rem;
        math_rem.SetKeyword(L"%");
        math_rem.ID   = 15;
        math_rem.Flag = flags_no_pop;
        ///////////////////////////////////////////
        static Expression math_add;
        math_add.SetKeyword(L"+");
        math_add.ID   = 10;
        math_add.Flag = flags_pop;
        math_add.NestExpres.SetCapacity(4);
        math_add.NestExpres.Add(&math_exp).Add(&math_rem).Add(&math_div).Add(&math_mul);

        static Expression math_sub;
        math_sub.SetKeyword(L"-");
        math_sub.ID         = 11;
        math_sub.Flag       = flags_no_pop;
        math_sub.NestExpres = math_add.NestExpres;
        ///////////////////////////////////////////
        static Expression logic_equ2;
        logic_equ2.SetKeyword(L"==");
        logic_equ2.ID   = 3;
        logic_equ2.Flag = flags_pop;
        logic_equ2.NestExpres.Add(&math_add).Add(&math_sub);

        static Expression logic_equ;
        logic_equ.SetKeyword(L"=");
        logic_equ.ID         = 4;
        logic_equ.Flag       = flags_pop;
        logic_equ.NestExpres = logic_equ2.NestExpres;

        static Expression logic_not_equ;
        logic_not_equ.SetKeyword(L"!=");
        logic_not_equ.ID         = 5;
        logic_not_equ.Flag       = flags_no_pop;
        logic_not_equ.NestExpres = logic_equ2.NestExpres;

        static Expression logic_less_equ;
        logic_less_equ.SetKeyword(L"<=");
        logic_less_equ.ID         = 6;
        logic_less_equ.Flag       = flags_no_pop;
        logic_less_equ.NestExpres = logic_equ2.NestExpres;

        static Expression logic_less;
        logic_less.SetKeyword(L"<");
        logic_less.ID         = 7;
        logic_less.Flag       = flags_no_pop;
        logic_less.NestExpres = logic_equ2.NestExpres;

        static Expression logic_big_equ;
        logic_big_equ.SetKeyword(L">=");
        logic_big_equ.ID         = 8;
        logic_big_equ.Flag       = flags_no_pop;
        logic_big_equ.NestExpres = logic_equ2.NestExpres;

        static Expression logic_big;
        logic_big.SetKeyword(L">");
        logic_big.ID         = 9;
        logic_big.Flag       = flags_no_pop;
        logic_big.NestExpres = logic_equ2.NestExpres;
        ///////////////////////////////////////////
        static Expression logic_and;
        logic_and.SetKeyword(L"&&");
        logic_and.ID   = 1;
        logic_and.Flag = flags_pop;
        logic_and.NestExpres.SetCapacity(7);
        logic_and.NestExpres.Add(&logic_equ2)
            .Add(&logic_equ)
            .Add(&logic_not_equ)
            .Add(&logic_less_equ)
            .Add(&logic_less)
            .Add(&logic_big_equ)
            .Add(&logic_big);

        static Expression logic_or;
        logic_or.SetKeyword(L"||");
        logic_or.ID         = 2;
        logic_or.Flag       = flags_no_pop;
        logic_or.NestExpres = logic_and.NestExpres;
        ///////////////////////////////////////////

        expres.Add(&logic_and).Add(&logic_or);
    }

    return expres;
}

// e.g. ( 4 + 3 ), ( 2 + ( 4 + ( 1 + 2 ) + 1 ) * 5 - 3 - 2 )
static String ParenthesisCallback(wchar_t const *block, MatchBit const &item, UNumber const length, void *other) noexcept {
    UNumber offset = 1;
    UNumber limit  = (length - 2);

    Array<MatchBit> items(Engine::Match(getMathExpres(), block, offset, limit));
    if (items.Size != 0) {
        return String::FromNumber(ALE::Calculate(items, block, offset, limit), 1, 0, 3);
    }

    String::SoftTrim(block, offset, limit);
    return String::Part(block, offset, limit);
}

static Expressions const &getParensExpres() noexcept {
    static Expressions expres(1);

    if (expres.Size == 0) {
        static Expression paren;
        static Expression paren_end;

        paren_end.SetKeyword(L")");
        paren_end.ID      = 20;
        paren_end.Flag    = Flags::BUBBLE;
        paren_end.ParseCB = &(ParenthesisCallback);
        paren_end.NestExpres.SetCapacity(1);
        paren_end.NestExpres.Add(&paren);

        paren.SetKeyword(L"(");
        paren.Connected = &paren_end;

        expres.Add(&paren);
    }

    return expres;
}

static double Evaluate(wchar_t const *content, UNumber offset, UNumber limit) noexcept {
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
    double num;

    Array<MatchBit> items(Engine::Match(getParensExpres(), content, offset, limit));

    if (items.Size != 0) {
        String n_content(Engine::Parse(items, content, offset, limit));
        num = ALE::Calculate(Engine::Match(getMathExpres(), n_content.Str, 0, n_content.Length), n_content.Str, 0, n_content.Length);
    } else if ((items = Engine::Match(getMathExpres(), content, offset, limit)).Size != 0) {
        num = ALE::Calculate(items, content, offset, limit);
    } else {
        String::SoftTrim(content, offset, limit);
        String::ToNumber(num, content, offset, limit);
    }

    return num;
}

} // namespace ALE
} // namespace Qentem

#endif
