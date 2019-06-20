
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

using Qentem::Engine::Expression;
using Qentem::Engine::Expressions;
using Qentem::Engine::Flags;
using Qentem::Engine::Match;

struct ALU {
    static const Expressions ParensExprs;
    static const Expressions MathExprs;

    static Expressions getMathExprs() noexcept {
        static Expression MathExp; // 1
        static Expression MathRem; // 2
        static Expression MathDiv; // 3
        static Expression MathMul; // 4

        static Expression MathAdd; // 1
        static Expression MathSub; // 2

        static Expression MathEqu2; // 1
        static Expression MathEqu;  // 2
        static Expression MathNEqu; // 3
        static Expression MathLEqu; // 4
        static Expression MathBEqu; // 5

        constexpr UNumber flags_ops    = Flags::SPLIT | Flags::GROUPSPLIT | Flags::POP | Flags::TRIM;
        constexpr UNumber flags_no_pop = Flags::SPLIT | Flags::GROUPSPLIT | Flags::TRIM;
        ///////////////////////////////////////////
        MathAdd.Keyword = L'+';
        MathAdd.ID      = 1;
        MathAdd.Flag    = flags_ops;
        MathAdd.ParseCB = &(AdditionCallback);
        MathAdd.NestExprs.Add(&MathExp).Add(&MathRem).Add(&MathDiv).Add(&MathMul);

        MathSub.Keyword = L'-';
        MathSub.ID      = 2;
        MathSub.Flag    = flags_no_pop;
        MathSub.ParseCB = &(AdditionCallback);
        ///////////////////////////////////////////
        MathExp.Keyword = L"^";
        MathExp.ID      = 1;
        MathExp.Flag    = flags_no_pop;
        MathExp.ParseCB = &(MultiplicationCallback);

        MathRem.Keyword = L'%';
        MathRem.ID      = 2;
        MathRem.Flag    = flags_no_pop;
        MathRem.ParseCB = &(MultiplicationCallback);

        MathDiv.Keyword = L'/';
        MathDiv.ID      = 3;
        MathDiv.Flag    = flags_no_pop;
        MathDiv.ParseCB = &(MultiplicationCallback);

        MathMul.Keyword = L"*";
        MathMul.ID      = 4;
        MathMul.Flag    = flags_no_pop;
        MathMul.ParseCB = &(MultiplicationCallback);
        ///////////////////////////////////////////

        ///////////////////////////////////////////
        MathEqu2.Keyword = L"==";
        MathEqu2.ID      = 1;
        MathEqu2.Flag    = flags_ops;
        MathEqu2.ParseCB = &(EqualCallback);
        MathEqu2.NestExprs.Add(&MathAdd).Add(&MathSub);

        MathEqu.Keyword = L"=";
        MathEqu.ID      = 2;
        MathEqu.Flag    = flags_no_pop;
        MathEqu.ParseCB = &(EqualCallback);

        MathNEqu.Keyword = L"!=";
        MathNEqu.ID      = 3;
        MathNEqu.Flag    = flags_no_pop;
        MathNEqu.ParseCB = &(EqualCallback);

        MathLEqu.Keyword = L"<=";
        MathLEqu.ID      = 4;
        MathLEqu.Flag    = flags_no_pop;
        MathLEqu.ParseCB = &(EqualCallback);

        MathBEqu.Keyword = L">=";
        MathBEqu.ID      = 5;
        MathBEqu.Flag    = flags_no_pop;
        MathBEqu.ParseCB = &(EqualCallback);

        return Expressions().Add(&MathEqu2).Add(&MathEqu).Add(&MathNEqu).Add(&MathLEqu).Add(&MathBEqu);

        // TODO: Implement && 'AND' 1=1&&1>3
    }

    static Expressions getParensExprs() noexcept {
        static Expression ParensExpr;
        static Expression ParensNext;

        ParensExpr.Keyword   = L'(';
        ParensNext.Keyword   = L')';
        ParensExpr.Connected = &ParensNext;
        ParensNext.Flag      = Flags::BUBBLE | Flags::TRIM;
        ParensNext.ParseCB   = &(ParenthesisCallback);
        ParensNext.NestExprs.Add(&ParensExpr);
        ///////////////////////////////////////////

        return Expressions().Add(&ParensExpr);
    }

    // e.g. ( 4 + 3 ), ( 2 + ( 4 + ( 1 + 2 ) + 1 ) * 5 - 3 - 2 )
    static String ParenthesisCallback(const String &block, const Match &item) noexcept {
        return Engine::Parse(block, Engine::Search(block, ALU::MathExprs, item.OLength, (block.Length - item.CLength)),
                             item.OLength, (block.Length - item.CLength));
    }

    static void NestNumber(const String &block, const Match &item, double &number) noexcept {
        String r = Engine::Parse(block, item.NestMatch, item.Offset, (item.Offset + item.Length));
        if (r.Length != 0) {
            String::ToNumber(r, number);
        }
    }

    static String EqualCallback(const String &block, const Match &item) noexcept {
        double number1 = 0.0;
        double number2 = 0.0;

        Match *m2;

        const Match *m1    = &(item.NestMatch.Storage[0]);
        UNumber      op_id = m1->Expr->ID;

        if (m1->Length != 0) {
            if (m1->NestMatch.Size != 0) {
                NestNumber(block, *m1, number1);
            } else {
                String::ToNumber(block, number1, m1->Offset, m1->Length);
            }
        }

        for (UNumber i = 1; i < item.NestMatch.Size; i++) {
            m2 = &(item.NestMatch.Storage[i]);

            if (m2->Length != 0) {

                if (m2->NestMatch.Size != 0) {
                    NestNumber(block, *m2, number2);
                } else {
                    String::ToNumber(block, number2, m2->Offset, m2->Length);
                }

                switch (op_id) {
                    case 3:
                        number1 = (number1 != number2) ? 1.0 : 0.0;
                        break;
                    case 4:
                        number1 = (number1 <= number2) ? 1.0 : 0.0;
                        break;
                    case 5:
                        number1 = (number1 >= number2) ? 1.0 : 0.0;
                        break;
                    default:
                        number1 = (number1 == number2) ? 1.0 : 0.0;
                        break;
                }
            }

            op_id = m2->Expr->ID;
        }

        return String::FromNumber(number1);
    }

    static String MultiplicationCallback(const String &block, const Match &item) noexcept {
        double number1 = 0.0;
        double number2 = 0.0;

        Match *m2;

        const Match *m1    = &(item.NestMatch.Storage[0]);
        UNumber      op_id = m1->Expr->ID;

        if (m1->Length != 0) {
            String::ToNumber(block, number1, m1->Offset, m1->Length);
        }

        for (UNumber i = 1; i < item.NestMatch.Size; i++) {
            m2 = &(item.NestMatch.Storage[i]);

            if (m2->Length != 0) {

                String::ToNumber(block, number2, m2->Offset, m2->Length);

                switch (op_id) {
                    case 1:
                        if (number2 != 0.0) {
                            // if (number2 > 1) {
                            const UNumber times = static_cast<UNumber>(number2);
                            for (UNumber k = 1; k < times; k++) {
                                number1 *= number1;
                                // }
                            }
                        } else {
                            number1 = 1;
                        }
                        break;
                    case 2:
                        number1 = static_cast<double>(static_cast<UNumber>(number1) % static_cast<UNumber>(number2));
                        break;
                    case 3:
                        if (number2 == 0) {
                            return L"0";
                        }

                        number1 /= number2;
                        break;
                    default:
                        number1 *= number2;
                        break;
                }
            } else {
                return L"0";
            }

            op_id = m2->Expr->ID;
        }

        return String::FromNumber(number1);
    }

    static String AdditionCallback(const String &block, const Match &item) noexcept {
        double number1 = 0.0;
        double number2 = 0.0;

        Match *m2;

        const Match *m1    = &(item.NestMatch.Storage[0]);
        UNumber      op_id = m1->Expr->ID;

        if (m1->Length != 0) {
            if (m1->NestMatch.Size != 0) {
                NestNumber(block, *m1, number1);
            } else {
                String::ToNumber(block, number1, m1->Offset, m1->Length);
            }
        }

        for (UNumber i = 1; i < item.NestMatch.Size; i++) {
            m2 = &(item.NestMatch.Storage[i]);

            if (m2->Length != 0) {

                if (m2->NestMatch.Size != 0) {
                    NestNumber(block, *m2, number2);
                } else {
                    String::ToNumber(block, number2, m2->Offset, m2->Length);
                }

                switch (op_id) {
                    case 2:
                        number1 -= number2;
                        break;
                    default:
                        number1 += number2;
                        break;
                }

                op_id = m2->Expr->ID;
            } else if ((op_id == 2) && (op_id == m2->Expr->ID)) {
                op_id = 1; // Two --
            } else {
                op_id = m2->Expr->ID;
            }
        }

        return String::FromNumber(number1);
    }

    static double Evaluate(String &content) noexcept {
        /**
         *
         * e.g. ((2* (1 * 3)) + 1 - 4) + ((10 - 5) - 6 + ((1 + 1) + (1 + 1))) * (8 / 4 + 1) - (1) - (-1) + 2 = 14
         * e.g. (6 + 1 - 4) + (5 - 6 + 4) * (8 / 4 + 1) - (1) - (-1) + 2 = 14 e.g. 3 + 3 * 3 - 1 + 1 + 2 = 14
         * e.g. 3 + 9 - 1 - -1 + 2 = 14 e.g. 14 = 14
         * 1 means true.
         *
         * Steps:
         * First: Look for parenthesis ( operation ora  number )
         * Second: Process opreations: * / % ^
         * Third: Process : + and -
         * Forth: Process logic ( = != > < ... )
         * Fifth: Return final value or 0;
         */

        // Stage one:
        content = Engine::Parse(content, Engine::Search(content, ParensExprs));
        if ((content.Length == 0) || (content == L'0')) {
            return 0.0;
        }

        // Stage two:
        double num = 0.0;
        content    = Engine::Parse(content, Engine::Search(content, MathExprs));
        if ((content.Length == 0) || (content == L'0') || !String::ToNumber(content, num)) {
            return 0.0;
        }

        return num;
    }
};

const Expressions ALU::ParensExprs = ALU::getParensExprs();
const Expressions ALU::MathExprs   = ALU::getMathExprs();

} // namespace Qentem

#endif
