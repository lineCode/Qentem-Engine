
/**
 * Qentem ALU
 *
 * @brief     For arithmetic and logic evaluation.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Addon/QRegex.hpp"
#include "Addon/ALU.hpp"

using Qentem::String;
using Qentem::Engine::Flags;
using Qentem::QRegex::OR;

Qentem::ALU::ALU() noexcept {
    ParensExpr.Keyword   = L'(';
    ParensNext.Keyword   = L')';
    ParensExpr.Connected = &ParensNext;
    ParensNext.Flag      = Flags::BUBBLE | Flags::TRIM;
    ParensNext.NestExprs.Add(&ParensExpr);
    ParensNext.ParseCB = &(ALU::ParenthesisCallback);
    ParensNext.Pocket  = &(this->MathExprs);

    this->ParensExprs.Add(&ParensExpr);

    MathMul.Keyword  = L"*|/|%"; // ^ Needs it's own callback
    MathMul.Flag     = Flags::SPLIT | Flags::GROUPSPLIT | Flags::TRIM;
    MathMul.ParseCB  = &(ALU::MultiplicationCallback);
    MathMul.SearchCB = &(QRegex::OR);

    MathAdd.Keyword = L"+|-";
    MathAdd.Flag    = Flags::SPLIT | Flags::GROUPSPLIT | Flags::POP | Flags::TRIM;
    MathAdd.NestExprs.Add(&MathMul);
    MathAdd.ParseCB  = &(ALU::AdditionCallback);
    MathAdd.SearchCB = &(QRegex::OR);

    MathEqu.Keyword = L"==|=|!=|<|>|<=|>=";
    MathEqu.Flag    = Flags::SPLIT | Flags::GROUPSPLIT | Flags::POP | Flags::TRIM;
    MathEqu.NestExprs.Add(&MathAdd);
    MathEqu.ParseCB  = &(ALU::EqualCallback);
    MathEqu.SearchCB = &(QRegex::OR);

    this->MathExprs.Add(&MathEqu);
}

// e.g. ( 4 + 3 ), ( 2 + ( 4 + ( 1 + 2 ) + 1 ) * 5 - 3 - 2 )
String Qentem::ALU::ParenthesisCallback(const String &block, const Match &item) noexcept {
    String result = String::Part(block, item.OLength, (block.Length - (item.OLength + item.CLength)));
    return Engine::Parse(result, Engine::Search(result, *(static_cast<Expressions *>(item.Expr->Pocket))));
}

bool Qentem::ALU::NestNumber(const String &block, const Match &item, double &number) noexcept {
    if (item.NestMatch.Size != 0) {
        String r = Engine::Parse(block, item.NestMatch, item.Offset, item.Offset + item.Length);
        return ((r.Length != 0) && String::ToNumber(r, number));
    }

    if (item.Length != 0) {
        return String::ToNumber(block, number, item.Offset, item.Length);
    }

    return false;
}

String Qentem::ALU::EqualCallback(const String &block, const Match &item) noexcept {
    bool result = false;

    if (item.NestMatch.Size != 0) {
        bool    is_str  = false;
        double  temnum1 = 0.0;
        double  temnum2 = 0.0;
        String  str;
        Match * nm = &(item.NestMatch[0]);
        UNumber op = nm->Tag;

        if ((nm->Length - nm->Offset) == 0) {
            return L"0";
        }

        if (!ALU::NestNumber(block, *nm, temnum1)) {
            is_str = true;
            str    = String::Part(block, nm->Offset, nm->Length);
        }

        for (UNumber i = 1; i < item.NestMatch.Size; i++) {
            nm = &(item.NestMatch[i]);
            if (is_str) {
                result = (str == String::Part(block, nm->Offset, nm->Length)); // TODO: compare without copying block
                if (op > 3) {
                    result = !result;
                }
            } else {
                if (!ALU::NestNumber(block, *nm, temnum2)) {
                    break;
                }

                switch (op) {
                    case 1:
                    case 2:
                        result = (temnum1 == temnum2);
                        break;
                    case 3:
                        result = (temnum1 != temnum2);
                        break;
                    case 4:
                        result = (temnum1 < temnum2);
                        break;
                    case 5:
                        result = (temnum1 > temnum2);
                        break;
                    case 6:
                        result = (temnum1 <= temnum2);
                        break;
                    case 7:
                        result = (temnum1 >= temnum2);
                        break;
                    default:
                        break;
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

String Qentem::ALU::MultiplicationCallback(const String &block, const Match &item) noexcept {
    UNumber op;
    double  number = 0.0;
    if (!ALU::NestNumber(block, item.NestMatch[0], number)) {
        return L"0";
    }

    double temnum = 0.0;
    Match *nm     = &(item.NestMatch[0]);
    op            = nm->Tag;
    for (UNumber i = 1; i < item.NestMatch.Size; i++) {
        nm = &(item.NestMatch[i]);

        if (!ALU::NestNumber(block, *nm, temnum)) {
            return L"0";
        }

        switch (op) {
            case 1:
                number *= temnum;
                break;
            case 2:
                number /= temnum;
                break;
            case 3:
                number = static_cast<double>((static_cast<UNumber>(number) % static_cast<UNumber>(temnum)));
                break;
            default:
                return L"0";
        }

        op = nm->Tag;
    }

    return String::FromNumber(number);
}

String Qentem::ALU::AdditionCallback(const String &block, const Match &item) noexcept {
    double number = 0.0;
    ALU::NestNumber(block, item.NestMatch[0], number);

    double  temnum = 0.0;
    bool    neg    = false;
    Match * nm     = &(item.NestMatch[0]);
    UNumber op     = nm->Tag;

    for (UNumber i = 1; i < item.NestMatch.Size; i++) {
        nm = &(item.NestMatch[i]);

        if (!ALU::NestNumber(block, *nm, temnum) && (op == 2) && (nm->Length == 0)) {
            neg = true; // x (- -1) or something like that.
            continue;
        }

        if ((op == 1) || neg) {
            number += temnum;
        } else {
            number -= temnum;
        }

        op = nm->Tag;
    }

    return String::FromNumber(number);
}

double Qentem::ALU::Evaluate(String &content) const noexcept {
    /**
     *
     * e.g. ((2* (1 * 3)) + 1 - 4) + ((10 - 5) - 6 + ((1 + 1) + (1 + 1))) * (8 / 4 + 1) - (1) - (-1) +
     * 2 = 14 e.g. (6 + 1 - 4) + (5 - 6 + 4) * (8 / 4 + 1) - (1) - (-1) + 2 = 14 e.g. 3 + 3 * 3 - 1 +
     * 1 + 2 = 14 e.g. 3 + 9 - 1 - -1 + 2 = 14 e.g. 14 = 14 e.g. 1; means true.
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
    if ((content.Length == 0) || (content == L"0") || !String::ToNumber(content, num, 0, 0)) {
        return 0;
    }

    return num;
}
