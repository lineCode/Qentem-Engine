
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

using Qentem::String;
using Qentem::Engine::Flags;

Qentem::ALU::ALU() noexcept {
    ParensExpr.Keyword = L"(";
    ParensTail.Keyword = L")";
    ParensExpr.Tail    = &ParensTail;
    ParensExpr.Flag    = Flags::BUBBLE;
    ParensExpr.NestExprs.Add(&ParensExpr);
    ParensExpr.ParseCB = &(ALU::ParenthesisCallback);
    ParensExpr.Pocket  = &(this->MathExprs);

    this->ParensExprs.Add(&ParensExpr);

    MathMul.Keyword  = L"*|/";
    MathMul.Flag     = Flags::SPLIT;
    MathMul.ParseCB  = &(ALU::MultiplicationCallback);
    MathMul.SearchCB = &(ALU::SearchCallback);

    MathAdd.Keyword = L"+|-";
    MathAdd.Flag    = Flags::SPLIT | Flags::POP;
    MathAdd.NestExprs.Add(&MathMul);
    MathAdd.ParseCB  = &(ALU::AdditionCallback);
    MathAdd.SearchCB = &(ALU::SearchCallback);

    MathEqu.Keyword = L"==|=|!=";
    MathEqu.Flag    = Flags::SPLIT | Flags::POP;
    MathEqu.NestExprs.Add(&MathAdd);
    MathEqu.ParseCB  = &(ALU::EqualCallback);
    MathEqu.SearchCB = &(ALU::SearchCallback);

    this->MathExprs.Add(&MathEqu);

    // start with = < > !
    // pop to * /
    // (/d+[*\/]/d+)
    /// Pop to + -
}

// e.g. ( 4 + 3 ), ( 2 + ( 4 + ( 1 + 2 ) + 1 ) * 5 - 3 - 2 )
String Qentem::ALU::ParenthesisCallback(const String &block, const Match &match) noexcept {
    String result = block.Part(match.OLength, (block.Length - (match.OLength + match.CLength)));
    return Engine::Parse(result, Engine::Search(result, *(static_cast<Expressions *>(match.Expr->Pocket))));
}

size_t Qentem::ALU::SearchCallback(const String &content, const Expression &expr, size_t &started,
                                   size_t &ended) noexcept {
    size_t counter = 0;
    size_t tag     = 1;

    for (; counter < expr.Keyword.Length; counter++) {
        if (content.Str[started] == expr.Keyword.Str[counter]) {
            // Maintaining a copy of the original offset.
            ended = started;

            while ((++counter < expr.Keyword.Length) && (expr.Keyword.Str[counter] != L'|')) {
                if (content.Str[++ended] != expr.Keyword.Str[counter]) {
                    // Mismatch.
                    counter = 0;
                    break;
                }
            }

            if (counter != 0) {
                return tag;
            }
        }

        while (counter < expr.Keyword.Length) {
            if (expr.Keyword.Str[++counter] == L'|') {
                tag += 1;
                break;
            }
        }
    }

    return 0;
}

bool Qentem::ALU::NestNumber(const String &block, const Match &match, double &number) noexcept {
    String r = L"";

    if (match.NestMatch.Size != 0) {
        r = Engine::Parse(block, match.NestMatch, match.Offset, match.Offset + match.Length);
    } else {
        r = block.Part(match.Offset, match.Length);
    }

    if ((r.Length == 0) || !String::ToNumber(r, number)) {
        return false;
    }

    return true;
}

String Qentem::ALU::EqualCallback(const String &block, const Match &match) noexcept {
    bool   result = false;
    bool   is_str = false;
    size_t op;

    if (match.NestMatch.Size > 0) {
        Match *nm      = &(match.NestMatch[0]);
        op             = nm->Tag;
        String r       = block.Part(nm->Offset, nm->Length);
        double temnum1 = 0.0;
        double temnum2 = 0.0;
        String temstr1 = L"";

        if (r.Length == 0) {
            return L"0";
        }

        if (!String::ToNumber(r, temnum1)) {
            is_str  = true;
            temstr1 = r;
        }

        for (size_t i = 1; i < match.NestMatch.Size; i++) {
            nm = &(match.NestMatch[i]);
            r  = block.Part(nm->Offset, nm->Length);

            if (is_str) {
                result = (String::Trim(temstr1) == String::Trim(r));
            } else {
                if (!String::ToNumber(r, temnum2)) {
                    break;
                }

                if (op < 3) {
                    result = (temnum1 == temnum2);
                } else if (op == 3) {
                    result = (temnum1 != temnum2);
                }
            }

            if (!result) {
                break;
            }

            op = nm->Tag;
        }
    }

    if (result) {
        return L"1";
    }

    return L"0";
}

String Qentem::ALU::MultiplicationCallback(const String &block, const Match &match) noexcept {
    double number = 0.0;
    size_t op;
    if (!ALU::NestNumber(block, match.NestMatch[0], number)) {
        return L"0";
    }

    double temnum = 0.0;
    Match *nm     = &(match.NestMatch[0]);
    op            = nm->Tag;
    for (size_t i = 1; i < match.NestMatch.Size; i++) {
        nm = &(match.NestMatch[i]);

        if (!ALU::NestNumber(block, *nm, temnum)) {
            return L"0";
        }

        if (op == 1) {
            number *= temnum;
        } else {
            if (temnum == 0) {
                return L"NULL";
            }
            number /= temnum;
        }

        op = nm->Tag;
    }

    return String::ToString(number);
}

String Qentem::ALU::AdditionCallback(const String &block, const Match &match) noexcept {
    double number = 0.0;
    size_t op;
    if (!ALU::NestNumber(block, match.NestMatch[0], number)) {
        return L"0";
    }

    double temnum = 0.0;
    Match *nm     = &(match.NestMatch[0]);
    op            = nm->Tag;
    for (size_t i = 1; i < match.NestMatch.Size; i++) {
        nm = &(match.NestMatch[i]);

        if (!ALU::NestNumber(block, *nm, temnum)) {
            return L"0";
        }

        if (op == 1) {
            number += temnum;
        } else {
            number -= temnum;
        }

        op = nm->Tag;
    }

    return String::ToString(number);
}

double Qentem::ALU::Evaluate(String &content) noexcept {
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

    content = Engine::Parse(content, Engine::Search(content, this->ParensExprs));

    if ((content.Length == 0) || (content == L"0")) {
        return 0;
    }

    content = Engine::Parse(content, Engine::Search(content, this->MathExprs));

    double num = 0;
    if ((content.Length == 0) || (content == L"0") || !String::ToNumber(content, num)) {
        return 0;
    }

    return num;
}
