/**
 * Qentem Arithmetic & Logic Evaluator
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

static bool   Process(double &number, const Array<MatchBit> &items, const char *content, UNumber offset, UNumber limit) noexcept;
static double Evaluate(const char *content, UNumber offset, UNumber limit) noexcept;

static const Expressions &getMathExpres() noexcept {
    static constexpr UNumber flags_no_pop = Flags::SPLIT | Flags::TRIM;
    static constexpr UNumber flags_pop    = flags_no_pop | Flags::POP;

    static Expressions expres(3);

    if (expres.Size == 0) {
        static Expression i_paren;
        static Expression i_paren_end;

        i_paren_end.SetKeyword(")");
        i_paren_end.Flag = Flags::IGNORE;
        i_paren_end.NestExpres.SetCapacity(1);
        i_paren_end.NestExpres.Add(&i_paren);

        i_paren.SetKeyword("(");
        i_paren.Connected = &i_paren_end;
        ///////////////////////////////////////////
        static Expression math_mul;
        math_mul.SetKeyword("*");
        math_mul.ID   = 2;
        math_mul.Flag = flags_no_pop;

        static Expression math_div;
        math_div.SetKeyword("/");
        math_div.ID   = 3;
        math_div.Flag = flags_no_pop;

        static Expression math_exp;
        math_exp.SetKeyword("^");
        math_exp.ID   = 4;
        math_exp.Flag = flags_no_pop;

        static Expression math_rem;
        math_rem.SetKeyword("%");
        math_rem.ID   = 5;
        math_rem.Flag = flags_no_pop;
        ///////////////////////////////////////////
        static Expression math_add;
        math_add.SetKeyword("+");
        math_add.ID   = 6;
        math_add.Flag = flags_pop;
        math_add.NestExpres.SetCapacity(5);
        math_add.NestExpres.Add(&math_mul).Add(&i_paren).Add(&math_div).Add(&math_rem).Add(&math_exp);

        static Expression math_sub;
        math_sub.SetKeyword("-");
        math_sub.ID         = 7;
        math_sub.Flag       = flags_no_pop;
        math_sub.NestExpres = math_add.NestExpres;
        ///////////////////////////////////////////
        static Expression equ2;
        equ2.SetKeyword("==");
        equ2.ID   = 8;
        equ2.Flag = flags_pop;
        equ2.NestExpres.SetCapacity(3);
        equ2.NestExpres.Add(&math_add).Add(&i_paren).Add(&math_sub);

        static Expression equ;
        equ.Keyword    = equ2.Keyword;
        equ.Length     = 1;
        equ.ID         = 9;
        equ.Flag       = flags_pop;
        equ.NestExpres = equ2.NestExpres;

        static Expression not_equ;
        not_equ.SetKeyword("!=");
        not_equ.ID         = 10;
        not_equ.Flag       = flags_no_pop;
        not_equ.NestExpres = equ2.NestExpres;

        static Expression less_equ;
        less_equ.SetKeyword("<=");
        less_equ.ID         = 11;
        less_equ.Flag       = flags_no_pop;
        less_equ.NestExpres = equ2.NestExpres;

        static Expression less;
        less.Keyword    = less_equ.Keyword;
        less.Length     = 1;
        less.ID         = 12;
        less.Flag       = flags_no_pop;
        less.NestExpres = equ2.NestExpres;

        static Expression big_equ;
        big_equ.SetKeyword(">=");
        big_equ.ID         = 13;
        big_equ.Flag       = flags_no_pop;
        big_equ.NestExpres = equ2.NestExpres;

        static Expression big;
        big.Keyword    = big_equ.Keyword;
        big.Length     = 1;
        big.ID         = 14;
        big.Flag       = flags_no_pop;
        big.NestExpres = equ2.NestExpres;
        ///////////////////////////////////////////
        static Expression logic_and;
        logic_and.SetKeyword("&&");
        logic_and.ID   = 15;
        logic_and.Flag = flags_pop;
        logic_and.NestExpres.SetCapacity(8);
        logic_and.NestExpres.Add(&equ2).Add(&i_paren).Add(&equ).Add(&not_equ).Add(&less_equ).Add(&less).Add(&big_equ).Add(&big);

        static Expression logic_or;
        logic_or.SetKeyword("||");
        logic_or.ID         = 16;
        logic_or.Flag       = flags_no_pop;
        logic_or.NestExpres = logic_and.NestExpres;
        ///////////////////////////////////////////
        expres.Add(&logic_and).Add(&i_paren).Add(&logic_or);
    }

    return expres;
}

static double Multiply(const char *content, const Array<MatchBit> &items) noexcept {
    const MatchBit *mb    = &(items[0]);
    UShort          op_id = mb->Expr->ID;
    double          number1;
    double          number2;

    if (!Process(number1, mb->NestMatch, content, mb->Offset, mb->Length)) {
        return 0.0;
    }

    for (UNumber i = 1; i < items.Size; i++) {
        mb = &(items[i]);

        if (!Process(number2, mb->NestMatch, content, mb->Offset, mb->Length)) {
            return 0.0;
        }

        switch (op_id) {
            case 2: // *
                number1 *= number2;
                break;
            case 3: // /
                if (number2 == 0) {
                    return 0.0;
                }

                number1 /= number2;
                break;
            case 4: // ^
                if (number2 != 0.0) {
                    bool neg = (number2 < 0);

                    if (neg) {
                        number2 *= -1;
                    }

                    UNumber      times = static_cast<UNumber>(number2);
                    const double num   = number1;

                    while (--times > 0) {
                        number1 *= num;
                    }

                    if (neg) {
                        number1 = (1 / number1);
                    }

                    break;
                }

                number1 = 1;
                break;
            case 5: // %
                number1 = static_cast<double>(static_cast<UNumber>(number1) % static_cast<UNumber>(number2));
                break;
            default:
                return 0.0;
        }

        op_id = mb->Expr->ID;
    }

    return number1;
}

static double Add(const char *content, const Array<MatchBit> &items) noexcept {
    const MatchBit *mb      = &(items[0]);
    UShort          op_id   = mb->Expr->ID;
    double          number1 = 0.0;
    double          number2;

    Process(number1, mb->NestMatch, content, mb->Offset, mb->Length);

    for (UNumber i = 1; i < items.Size; i++) {
        mb = &(items[i]);

        if (mb->Length != 0) {
            if (!Process(number2, mb->NestMatch, content, mb->Offset, mb->Length)) {
                return 0.0;
            }

            switch (op_id) {
                case 6:
                    number1 += number2;
                    break;
                case 7:
                    number1 -= number2;
                    break;
                default:
                    return 0.0;
            }

            op_id = mb->Expr->ID;
        } else if ((op_id == 7) && (op_id == mb->Expr->ID)) {
            op_id = 6; // double (-)
        } else {
            op_id = mb->Expr->ID;
        }
    }

    return number1;
}

static double Equal(const char *content, const Array<MatchBit> &items) noexcept {
    double number1 = 0.0;
    double number2;

    const MatchBit *mb    = &(items[0]);
    UShort          op_id = mb->Expr->ID;

    const char c = content[mb->Offset];

    if (((c < 58) && (c > 47)) || ((c == '(') || (c == '+') || (c == '-'))) {
        if (!Process(number1, mb->NestMatch, content, mb->Offset, mb->Length)) {
            return 0.0;
        }
    } else if (items.Size == 2) { // String
        if (String::Compare(content, mb->Offset, mb->Length, content, items[1].Offset, items[1].Length)) {
            return 1.0;
        }

        return 0.0;
    }

    for (UNumber i = 1; i < items.Size; i++) {
        mb = &(items[i]);

        if (!Process(number2, mb->NestMatch, content, mb->Offset, mb->Length)) {
            return 0.0;
        }

        switch (op_id) {
            case 8:
            case 9:
                number1 = (number1 == number2) ? 1.0 : 0.0;
                break;
            case 10:
                number1 = (number1 != number2) ? 1.0 : 0.0;
                break;
            case 11:
                number1 = (number1 <= number2) ? 1.0 : 0.0;
                break;
            case 12:
                number1 = (number1 < number2) ? 1.0 : 0.0;
                break;
            case 13:
                number1 = (number1 >= number2) ? 1.0 : 0.0;
                break;
            case 14:
                number1 = (number1 > number2) ? 1.0 : 0.0;
                break;
            default:
                return 0.0;
        }

        op_id = mb->Expr->ID;
    }

    return number1;
}

static double LogicAnd(const char *content, const Array<MatchBit> &items) noexcept {
    const MatchBit *mb    = &(items[0]);
    UShort          op_id = mb->Expr->ID;
    double          number1;
    double          number2;

    if (!Process(number1, mb->NestMatch, content, mb->Offset, mb->Length)) {
        return 0.0;
    }

    for (UNumber i = 1; i < items.Size; i++) {
        mb = &(items[i]);

        if (!Process(number2, mb->NestMatch, content, mb->Offset, mb->Length)) {
            return 0.0;
        }

        switch (op_id) {
            case 15:
                number1 = ((number1 > 0) && (number2 > 0.0)) ? 1.0 : 0.0;
                break;
            case 16:
                number1 = ((number1 > 0) || (number2 > 0.0)) ? 1.0 : 0.0;
                break;
            default:
                return 0.0;
        }

        op_id = mb->Expr->ID;
    }

    return number1;
}

static bool Process(double &number, const Array<MatchBit> &items, const char *content, const UNumber offset, const UNumber limit) noexcept {
    if (items.Size == 0) {
        if (content[offset] == '(') {
            if (limit < 3) {
                return false;
            }

            number = Evaluate(content, (offset + 1), (limit - 2));
            return true;
        }

        return String::ToNumber(number, content, offset, limit);
    }

    switch (items[0].Expr->ID) {
        case 2:
        case 3:
        case 4:
        case 5:
            number = Multiply(content, items);
            return true;

        case 6:
        case 7:
            number = Add(content, items);
            return true;

        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
            number = Equal(content, items);
            return true;

        case 15:
        case 16:
            number = LogicAnd(content, items);
            return true;

        default:
            return false;
    }
}

static double Evaluate(const char *content, UNumber offset, UNumber limit) noexcept {
    /**
     * e.g. ((2* (1 * 3)) + 1 - 4) + ((10 - 5) - 6 + ((1 + 1) + (1 + 1))) * (8 / 4 + 1) - (1) - (-1) + 2 == 14
     * e.g. (6 + 1 - 4) + (5 - 6 + 4) * (8 / 4 + 1) - (1) - (-1) + 2 = 14
     * e.g. 3 + 3 * 3 - 1 + 1 + 2 = 14
     * e.g. 3 + 9 - 1 - -1 + 2 == 14
     * e.g. 14 = 14
     * 1 means true.
     */
    double num;

    Array<MatchBit> items(Engine::Match(getMathExpres(), content, offset, limit));

    if (items.Size == 0) {
        String::SoftTrim(content, offset, limit);
    }

    if (!Process(num, items, content, offset, limit)) {
        return 0.0;
    }

    return num;
}

} // namespace ALE
} // namespace Qentem

#endif
