
/**
 * Qentem Test
 *
 * @brief     For testing Qentem Engine
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Addon/Test.hpp"
#include "Addon/QRegex.hpp"

using Qentem::Array;
using Qentem::String;
using Qentem::Engine::Expression;
using Qentem::Engine::Expressions;
using Qentem::Engine::Flags;
using Qentem::Test::TestBit;

void Qentem::Test::CleanBits(Array<TestBit> &bits) noexcept {
    for (UNumber i = 0; i < bits.Size; i++) {
        for (UNumber j = 0; j < bits[i].Collect.Size; j++) {
            delete bits[i].Collect[j];
        }
    }
}

Array<String> Qentem::Test::Extract(const String &content, const Array<Match> &items) noexcept {
    Array<String> matches = Array<String>();
    matches.SetCapacity(items.Size);

    // for (UNumber i = 0; i < items.Size; i++) {
    //     matches.Add(String::Part(content, items[i].Offset, items[i].Length));
    // }

    String match;
    for (UNumber i = 0; i < items.Size; i++) {
        match = String::Part(content, items[i].Offset, items[i].Length) + L" -> O:" +
                String::FromNumber(items[i].Offset) + L" L:" + String::FromNumber(items[i].Length) + L" OL:" +
                String::FromNumber(items[i].OLength) + L" CL:" + String::FromNumber(items[i].CLength);
        matches.Add(match);
    }

    return matches;
}

String Qentem::Test::DumbExpressions(const Expressions &expres, const String &offset, UNumber index,
                                     Qentem::Engine::Expression *expr) noexcept {
    if (expres.Size == 0) {
        return offset + L"No expressions!\n";
    }

    String _array    = offset + L"(" + String::FromNumber(static_cast<double>(expres.Size)) + L") => [\n";
    String innoffset = L"    ";
    String l_offset  = offset + innoffset + innoffset;

    for (UNumber i = index; i < expres.Size; i++) {

        if (expres[i] == expr) {
            _array += offset + innoffset + L"[" + String::FromNumber(static_cast<double>(i)) + L"]: " + L"This.\n";
            continue;
        }

        _array += offset + innoffset + L"[" + String::FromNumber(static_cast<double>(i)) + L"]: => {\n";

        _array += l_offset + L"Keyword: \"" + expres[i]->Keyword + L"\"\n";

        _array += l_offset + L"Flags: (" + String::FromNumber(static_cast<double>(expres[i]->Flag)) + L")";

        if ((expres[i]->Flag & Flags::COMPACT) != 0) {
            _array += L" COMPACT";
        }

        if ((expres[i]->Flag & Flags::BUBBLE) != 0) {
            _array += L" BUBBLE";
        }

        if ((expres[i]->Flag & Flags::NOPARSE) != 0) {
            _array += L" NOPARSE";
        }

        if ((expres[i]->Flag & Flags::IGNORE) != 0) {
            _array += L" IGNORE";
        }

        if ((expres[i]->Flag & Flags::SPLIT) != 0) {
            _array += L" SPLIT";
        }

        if ((expres[i]->Flag & Flags::SPLITNEST) != 0) {
            _array += L" SPLITNEST";
        }

        if ((expres[i]->Flag & Flags::GROUPSPLIT) != 0) {
            _array += L" GROUPSPLIT";
        }

        if ((expres[i]->Flag & Flags::POP) != 0) {
            _array += L" POP";
        }

        if ((expres[i]->Flag & Flags::ONCE) != 0) {
            _array += L" ONCE";
        }
        _array += L"\n";

        _array += l_offset + L"Replace: \"" + expres[i]->Replace + L"\"\n";

        if (expres[i]->SearchCB != nullptr) {
            _array += l_offset + L"SearchCB: Yes";

        } else {
            _array += l_offset + L"SearchCB: No";
        }
        _array += L"\n";

        if (expres[i]->ParseCB != nullptr) {
            _array += l_offset + L"ParseCB: Yes";

        } else {
            _array += l_offset + L"ParseCB: No";
        }
        _array += L"\n";

        if (expres[i]->Connected != nullptr) {
            _array += l_offset + L"Next:\n";
            _array += Test::DumbExpressions(Expressions().Add(expres[i]->Connected), innoffset + l_offset, 0, expres[i]);
        }

        if (expres[i]->NestExprs.Size != 0) {
            _array += l_offset + L"NestExprs:\n";
            _array += Test::DumbExpressions(expres[i]->NestExprs, innoffset + l_offset, 0, expres[i]);
        }

        if (expres[i]->SubExprs.Size != 0) {
            _array += l_offset + L"SubExprs:\n";
            _array += Test::DumbExpressions(expres[i]->SubExprs, innoffset + l_offset, 0, expres[i]);
        }

        _array += l_offset + L"}\n";
    }

    return _array + offset + L"]\n";
}

String Qentem::Test::DumbMatches(const String &content, const Array<Match> &matches, const String &offset,
                                 UNumber index) noexcept {
    if (matches.Size == 0) {
        return offset + L"No matches!\n";
    }

    Array<String> items = Test::Extract(content, matches);

    String innoffset = L"    ";
    String _array    = offset + L"(" + String::FromNumber(static_cast<double>(matches.Size)) + L") => [\n";

    // It should be (matches.size) not (items.Size), but they should be the same size!
    for (UNumber i = index; i < items.Size; i++) {
        _array += innoffset + offset + L"[" + String::FromNumber(static_cast<double>(i)) + L"]: " +
                  QRegex::Replace(items[i], L"\n", L"\\n") + L"\n";

        if (matches[i].NestMatch.Size != 0) {
            _array += innoffset + offset + L"-NestMatch:\n";
            _array += Test::DumbMatches(content, matches[i].NestMatch, innoffset + innoffset + offset, 0);
        }

        if (matches[i].SubMatch.Size != 0) {
            _array += innoffset + offset + L"-SubMatch:\n";
            _array += Test::DumbMatches(content, matches[i].SubMatch, innoffset + innoffset + offset, 0);
        }
    }

    return _array + offset + L"]\n";
}

Array<TestBit> Qentem::Test::GetBits() noexcept {
    Array<TestBit> bits = Array<TestBit>();
    TestBit        bit;

    Expression *x1;
    Expression *x2;
    Expression *x3;
    Expression *y1;
    Expression *y2;
    Expression *y3;

    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;
    bit.Content.Add(L"").Add(L" ").Add(L"  ");
    bit.Expected.Add(L"").Add(L" ").Add(L"  ");

    x1 = new Expression(); // Empty

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;
    bit.Content.Add(L"_").Add(L"-").Add(L" -- ").Add(L"- - - -");
    bit.Expected.Add(L"_").Add(L"_").Add(L" __ ").Add(L"_ _ _ _");

    x1          = new Expression();
    x1->Keyword = L"-";
    x1->Replace = L"_";

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;
    bit.Content.Add(L"----");
    bit.Expected.Add(L"****");

    x1          = new Expression();
    x1->Keyword = L"-";

    x2          = new Expression();
    x2->Keyword = L"--";

    x1->ParseCB = ([](const String &block, const Match &item) noexcept->String { return L"*"; });

    bit.Exprs.Add(x1).Add(x2);
    bit.Collect.Add(x1).Add(x2);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;
    bit.Content.Add(L" - ").Add(L" -- ").Add(L" {{{9}} ");
    bit.Expected.Add(L" 0-0 ").Add(L" 0-0 ").Add(L" 3-2 ");

    x1          = new Expression();
    x1->Keyword = L"-";

    x2          = new Expression();
    x2->Keyword = L"--";

    x3          = new Expression();
    x3->Keyword = L"{{{";

    y3            = new Expression();
    y3->Keyword   = L"}}";
    x3->Connected = y3;

    x1->ParseCB = x2->ParseCB = y3->ParseCB = ([](const String &block, const Match &item) noexcept->String {
        String nc = String::FromNumber(item.OLength) + L"-";
        nc += String::FromNumber(item.CLength);
        return nc;
    });

    bit.Exprs.Add(x2).Add(x1).Add(x3);
    bit.Collect.Add(x1).Add(x2).Add(x3).Add(y3);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"(<1>)<2>");
    bit.Expected.Add(L"(<1>)m");

    x1            = new Expression();
    y1            = new Expression();
    x1->Keyword   = L"(";
    y1->Keyword   = L")";
    x1->Connected = y1;
    y1->Flag      = Flags::NOPARSE;

    x2            = new Expression();
    y2            = new Expression();
    x2->Keyword   = L"<";
    y2->Keyword   = L">";
    x2->Connected = y2;
    y2->Replace   = L"m";

    bit.Exprs.Add(x1);
    bit.Exprs.Add(x2);
    bit.Collect.Add(x1).Add(x2).Add(y1).Add(y2);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"(<1>)<2>");
    bit.Expected.Add(L"(<1>)<n>");

    x1            = new Expression();
    y1            = new Expression();
    x1->Keyword   = L"(";
    y1->Keyword   = L")";
    x1->Connected = y1;
    y1->Flag      = Flags::NOPARSE;

    x2            = new Expression();
    y2            = new Expression();
    x2->Keyword   = L"<";
    y2->Keyword   = L">";
    x2->Connected = y2;
    y2->Flag      = Flags::COMPACT;
    y2->Replace   = L"n";

    bit.Exprs.Add(x1);
    bit.Exprs.Add(x2);
    bit.Collect.Add(x1).Add(x2).Add(y1).Add(y2);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<").Add(L"<>").Add(L"<> ").Add(L" <>").Add(L" <> ").Add(L"  <>  ");
    bit.Expected.Add(L"<").Add(L"-").Add(L"- ").Add(L" -").Add(L" - ").Add(L"  -  ");
    bit.Content.Add(L"<a").Add(L"a<aa").Add(L"<aa> a").Add(L"a <a>").Add(L"a <a> a").Add(L"a  <aa>  a");
    bit.Expected.Add(L"<a").Add(L"a<aa").Add(L"- a").Add(L"a -").Add(L"a - a").Add(L"a  -  a");

    x1            = new Expression();
    y1            = new Expression();
    x1->Keyword   = L"<";
    y1->Keyword   = L">";
    x1->Connected = y1;
    y1->Replace   = L"-";

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(y1);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<<").Add(L"<<>").Add(L"<<> ").Add(L" <<>").Add(L" <<> ").Add(L"  <<>  ");
    bit.Expected.Add(L"<<").Add(L"-").Add(L"- ").Add(L" -").Add(L" - ").Add(L"  -  ");
    bit.Content.Add(L"<<a").Add(L"a<<aa").Add(L"<<aa> a").Add(L"a <<a>").Add(L"a <<a> a").Add(L"a  <<aa>  a");
    bit.Expected.Add(L"<<a").Add(L"a<<aa").Add(L"- a").Add(L"a -").Add(L"a - a").Add(L"a  -  a");

    x1            = new Expression();
    y1            = new Expression();
    x1->Keyword   = L"<<";
    y1->Keyword   = L">";
    x1->Connected = y1;
    y1->Replace   = L"-";

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(y1);
    bits.Add(bit);
    /////////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<").Add(L"<>>").Add(L"<>> ").Add(L" <>>").Add(L" <>> ").Add(L"  <>>  ");
    bit.Expected.Add(L"<").Add(L"-").Add(L"- ").Add(L" -").Add(L" - ").Add(L"  -  ");
    bit.Content.Add(L"<a").Add(L"a<aa").Add(L"<aa>> a").Add(L"a <a>>").Add(L"a <a>> a").Add(L"a {8} <aa>>  a");
    bit.Expected.Add(L"<a").Add(L"a<aa").Add(L"- a").Add(L"a -").Add(L"a - a").Add(L"a {8} -  a");

    x1 = new Expression();
    y1 = new Expression();
    x2 = new Expression();
    y2 = new Expression();

    x1->Keyword   = L"<";
    y1->Keyword   = L">>";
    x1->Connected = y1;
    x2->Keyword   = L"{";
    y2->Keyword   = L"}";
    y2->Flag      = Flags::IGNORE;
    x2->Connected = y2;
    y1->Replace   = L"-";

    bit.Exprs.Add(x1).Add(x2);
    bit.Collect.Add(x1).Add(x2).Add(y1).Add(y2);
    bits.Add(bit);
    /////////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<<").Add(L"<<>>").Add(L"<<>> ").Add(L" <<>>").Add(L" <<>> ").Add(L"  <<>>  ");
    bit.Expected.Add(L"<<").Add(L"-").Add(L"- ").Add(L" -").Add(L" - ").Add(L"  -  ");
    bit.Content.Add(L"<<a").Add(L"a<<aa").Add(L"<<aa>> a").Add(L"a <<a>>").Add(L"a <<a>> a").Add(L"a  <<aa>>  a");
    bit.Expected.Add(L"<<a").Add(L"a<<aa").Add(L"- a").Add(L"a -").Add(L"a - a").Add(L"a  -  a");

    x1            = new Expression();
    y1            = new Expression();
    x1->Keyword   = L"<<";
    y1->Keyword   = L">>";
    x1->Connected = y1;
    y1->Replace   = L"-";

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(y1);
    bits.Add(bit);
    /////////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<").Add(L"->-").Add(L"<o<>").Add(L"<>o>").Add(L"<><>o>").Add(L"<><><>");
    bit.Expected.Add(L"<").Add(L"->-").Add(L"=").Add(L"=o>").Add(L"==o>").Add(L"===");

    x1            = new Expression();
    y1            = new Expression();
    x1->Keyword   = L"<";
    y1->Keyword   = L">";
    x1->Connected = y1;
    y1->Replace   = L"=";

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(y1);
    bits.Add(bit);
    /////////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<>").Add(L"<><>").Add(L"<><><>");
    bit.Expected.Add(L"=").Add(L"=<>").Add(L"=<><>");

    x1            = new Expression();
    y1            = new Expression();
    x1->Keyword   = L"<";
    y1->Keyword   = L">";
    x1->Connected = y1;
    y1->Flag      = Flags::ONCE;
    y1->Replace   = L"=";

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(y1);
    bits.Add(bit);
    /////////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<o<>").Add(L"<>").Add(L"< < >>").Add(L"<0><<<1-0-0><1-0-1><1-0-2>><1-1>><2>");
    bit.Expected.Add(L"<o=").Add(L"=").Add(L"=").Add(L"===");
    bit.Content.Add(L"<0><1><<2-0><<2-1-0><2-1-1><<2-1-2-0><2-1-2-1><2-1-2-2><2-1-2-3>>><2-2><2-3>><3> ><");
    bit.Expected.Add(L"==== ><");

    x1            = new Expression();
    y1            = new Expression();
    x1->Keyword   = L"<";
    y1->Keyword   = L">";
    x1->Connected = y1;
    y1->Replace   = L"=";
    y1->NestExprs.Add(x1);

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(y1);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<<o<<>").Add(L"<<>").Add(L"<< << >>").Add(L"<<0><<<<<<1-0-0><<1-0-1><<1-0-2>><<1-1>><<2>");
    bit.Expected.Add(L"<<o+").Add(L"+").Add(L"+").Add(L"+++");
    bit.Content.Add(
        L"<<0><<1><<<<2-0><<<<2-1-0><<2-1-1><<<<2-1-2-0><<2-1-2-1><<2-1-2-2><<2-1-2-3>>><<2-2><<2-3>><<3> ><<");
    bit.Expected.Add(L"++++ ><<");

    x1            = new Expression();
    y1            = new Expression();
    x1->Keyword   = L"<<";
    y1->Keyword   = L">";
    x1->Connected = y1;
    y1->Replace   = L"+";
    y1->NestExprs.Add(x1);

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(y1);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<o<>>").Add(L"<>>").Add(L"< < >>>>").Add(L"<0>><<<1-0-0>><1-0-1>><1-0-2>>>><1-1>>>><2>>");
    bit.Expected.Add(L"<o_").Add(L"_").Add(L"_").Add(L"___");
    bit.Content.Add(
        L"<0>><1>><<2-0>><<2-1-0>><2-1-1>><<2-1-2-0>><2-1-2-1>><2-1-2-2>><2-1-2-3>>>>>><2-2>><2-3>>>><3>> >><");
    bit.Expected.Add(L"____ >><");

    x1            = new Expression();
    y1            = new Expression();
    x1->Keyword   = L"<";
    y1->Keyword   = L">>";
    x1->Connected = y1;
    y1->Replace   = L"_";
    y1->NestExprs.Add(x1);

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(y1);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<<o<<>>")
        .Add(L"<<>>")
        .Add(L"<< << >>>>")
        .Add(L"<<0>><<<<<<1-0-0>><<1-0-1>><<1-0-2>>>><<1-1>>>><<2>>");
    bit.Expected.Add(L"<<o_").Add(L"_").Add(L"_").Add(L"___");
    bit.Content.Add(
        L"<<0>><<1>><<<<2-0>><<<<2-1-0>><<2-1-1>><<<<2-1-2-0>><<2-1-2-1>><<2-1-2-2>><<2-1-2-3>>>>>><<2-2>><<2-3>>>><<3>> >><<");
    bit.Expected.Add(L"____ >><<");

    x1            = new Expression();
    y1            = new Expression();
    x1->Keyword   = L"<<";
    y1->Keyword   = L">>";
    x1->Connected = y1;
    y1->Replace   = L"_";
    y1->NestExprs.Add(x1);

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(y1);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<o<>").Add(L"<>").Add(L"< < >>").Add(L"<0><<<1-0-0><1-0-1><1-0-2>><1-1>><2>");
    bit.Expected.Add(L"<o()").Add(L"()").Add(L"( ( ))").Add(L"(0)(((1-0-0)(1-0-1)(1-0-2))(1-1))(2)");
    bit.Content.Add(L"<0><1><<2-0><<2-1-0><2-1-1><<2-1-2-0><2-1-2-1><2-1-2-2><2-1-2-3>>><2-2><2-3>><3> ><");
    bit.Expected.Add(L"(0)(1)((2-0)((2-1-0)(2-1-1)((2-1-2-0)(2-1-2-1)(2-1-2-2)(2-1-2-3)))(2-2)(2-3))(3) ><");

    x1            = new Expression();
    y1            = new Expression();
    x1->Keyword   = L"<";
    y1->Keyword   = L">";
    x1->Connected = y1;
    y1->Flag      = Flags::BUBBLE;
    y1->NestExprs.Add(x1);

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(y1);
    bits.Add(bit);

    y1->ParseCB = ([](const String &block, const Match &item) noexcept->String {
        String nc = L"(";
        nc += String::Part(block, item.OLength, block.Length - (item.OLength + item.CLength));
        nc += L")";
        return nc;
    });
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"(6<1-B>9)").Add(L"(6<1-B>9)(6<1-B>9)").Add(L"(6<1-<2-<3-U>>>9)");
    bit.Expected.Add(L"=(6A+9)=").Add(L"=(6A+9)==(6A+9)=").Add(L"=(6A+9)=");
    bit.Content.Add(L"(6<1-<2-<3-U>>>9)(6<1-<2-<3-U>>>9)");
    bit.Expected.Add(L"=(6A+9)==(6A+9)=");

    x1            = new Expression();
    y1            = new Expression();
    x1->Keyword   = L"(";
    y1->Keyword   = L")";
    x1->Connected = y1;
    y1->Flag      = Flags::BUBBLE;

    x2            = new Expression();
    y2            = new Expression();
    x2->Keyword   = L"<";
    y2->Keyword   = L">";
    x2->Connected = y2;
    y2->Flag      = Flags::BUBBLE;
    y2->NestExprs.Add(x2); // Nest itself

    y1->NestExprs.Add(x2); // Nested by x2

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(x2).Add(y1).Add(y2);
    bits.Add(bit);

    y1->ParseCB = ([](const String &block, const Match &item) noexcept->String {
        String nc = L"=";
        nc += block;
        nc += L"=";
        return nc;
    });

    y2->ParseCB = ([](const String &block, const Match &item) noexcept->String {
        if (block == L"<3-U>") {
            return L"A";
        }

        if (block == L"<2-A>") {
            return L"B";
        }

        if (block == L"<1-B>") {
            return L"A+";
        }

        return L"err";
    });
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"ssy<i1i>TTT-Z<i2i>XUU-U<i3i>ds");
    bit.Expected.Add(L"123");

    x1            = new Expression();
    y1            = new Expression();
    x1->Keyword   = L"<";
    y1->Keyword   = L">";
    x1->Connected = y1;
    y1->Flag      = Flags::SPLIT | Flags::GROUPSPLIT;

    x2            = new Expression();
    y2            = new Expression();
    x2->Keyword   = L"i";
    y2->Keyword   = L"i";
    x2->Connected = y2;

    y1->SubExprs.Add(x2);

    y1->ParseCB = ([](const String &block, const Match &item) noexcept->String {
        String nc = L"";

        if (item.SubMatch.Size != 0) {
            return L"SubMatch should not be here!";
        }

        for (UNumber i = 0; i < item.NestMatch.Size; i++) {
            Match *nm = &(item.NestMatch[i]);
            if (nm->SubMatch.Size != 0) {
                nc += String::Part(block, (nm->SubMatch[0].Offset + nm->SubMatch[0].OLength),
                                   (nm->SubMatch[0].Length - (nm->SubMatch[0].OLength + nm->SubMatch[0].CLength)));
            }
        }
        return nc;
    });

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(x2).Add(y1).Add(y2);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"[([x]),([y])--]==").Add(L"e[(]xx)] ").Add(L" [(x])] ");
    bit.Expected.Add(L"[]==").Add(L"e[] ").Add(L" [] ");

    x1            = new Expression();
    y1            = new Expression();
    x1->Keyword   = L"[";
    y1->Keyword   = L"]";
    x1->Connected = y1;
    y1->Flag      = Flags::COMPACT;

    x2            = new Expression();
    y2            = new Expression();
    x2->Keyword   = L"(";
    y2->Keyword   = L")";
    x2->Connected = y2;

    y1->NestExprs.Add(x2);

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(x2).Add(y1).Add(y2);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;
    bit.Content.Add(L"[[[[((((x]]]])))),((((y))))--]]]]--");
    bit.Expected.Add(L"[[[[]]]]--");

    x1            = new Expression();
    y1            = new Expression();
    x1->Keyword   = L"[[[[";
    y1->Keyword   = L"]]]]";
    x1->Connected = y1;
    y1->Flag      = Flags::COMPACT;

    x2            = new Expression();
    y2            = new Expression();
    x2->Keyword   = L"((((";
    y2->Keyword   = L"))))";
    x2->Connected = y2;

    y1->NestExprs.Add(x2);

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(x2).Add(y1).Add(y2);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"[xY]/").Add(L"    [     x     Y    ]     /  ").Add(L"[xY-yyyyyy]/");
    bit.Expected.Add(L"Y").Add(L"         Y      ").Add(L"Y-yyyyyy");

    x1            = new Expression();
    y1            = new Expression();
    x1->Keyword   = L"[";
    y1->Keyword   = L"/";
    x1->Connected = y1;

    x2            = new Expression();
    y2            = new Expression();
    x2->Keyword   = L"x";
    y2->Keyword   = L"]";
    x2->Connected = y2;

    y1->SubExprs.Add(x2);

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(x2).Add(y1).Add(y2);
    bits.Add(bit);

    y1->ParseCB = &(Qentem::Test::SubMatchZero);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"[[xxxxxY]]]]]]]]]]///")
        .Add(L"    [[     xxxxx     Y    ]]]]]]]]]]     ///  ")
        .Add(L"[[xxxxxY-yyyyyy]]]]]]]]]]///");
    bit.Expected.Add(L"Y").Add(L"         Y      ").Add(L"Y-yyyyyy");

    x1            = new Expression();
    y1            = new Expression();
    x1->Keyword   = L"[[";
    y1->Keyword   = L"///";
    x1->Connected = y1;

    x2            = new Expression();
    y2            = new Expression();
    x2->Keyword   = L"xxxxx";
    y2->Keyword   = L"]]]]]]]]]]";
    x2->Connected = y2;

    y1->SubExprs.Add(x2);

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(x2).Add(y1).Add(y2);
    bits.Add(bit);

    y1->ParseCB = &(Qentem::Test::SubMatchZero);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"#    [[ op1='one', op2='two' opx=']q       ]']second op is:/");
    bit.Expected.Add(L"#    second op is:two");

    x1            = new Expression();
    y1            = new Expression();
    x1->Keyword   = L"[[ ";
    y1->Keyword   = L"/";
    x1->Connected = y1;

    x2            = new Expression();
    y2            = new Expression();
    x2->Keyword   = L"[[";
    y2->Keyword   = L"]";
    x2->Connected = y2;

    x3            = new Expression();
    y3            = new Expression();
    x3->Keyword   = L"'";
    y3->Keyword   = L"'";
    x3->Connected = y3;

    y2->NestExprs.Add(x3);
    y1->SubExprs.Add(x2);
    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(x2).Add(x3).Add(y1).Add(y2).Add(y3);
    bits.Add(bit);

    y1->ParseCB = &(Qentem::Test::SubMatchNestMatch);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"#    [[ op1=;'one';, op2=;'two'; opx=;'}}}q       }}}';}}}second op is:{//}");
    bit.Expected.Add(L"#    second op is:two");

    x1            = new Expression();
    y1            = new Expression();
    x1->Keyword   = L"[[ ";
    y1->Keyword   = L"{//}";
    x1->Connected = y1;

    x2            = new Expression();
    y2            = new Expression();
    x2->Keyword   = L"[[";
    y2->Keyword   = L"}}}";
    x2->Connected = y2;

    x3            = new Expression();
    y3            = new Expression();
    x3->Keyword   = L";'";
    y3->Keyword   = L"';";
    x3->Connected = y3;

    y2->NestExprs.Add(x3);
    y1->SubExprs.Add(x2);
    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(x2).Add(x3).Add(y1).Add(y2).Add(y3);
    bits.Add(bit);

    y1->ParseCB = &(Qentem::Test::SubMatchNestMatch);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"#    [ op1='one', op2='two' opx=']q       ]']second op is:/");
    bit.Expected.Add(L"#    second op is:two");

    x1            = new Expression();
    y1            = new Expression();
    x1->Keyword   = L"[ ";
    y1->Keyword   = L"/";
    x1->Connected = y1;

    x2            = new Expression();
    y2            = new Expression();
    x2->Keyword   = L"[";
    y2->Keyword   = L"]";
    x2->Connected = y2;

    x3            = new Expression();
    y3            = new Expression();
    x3->Keyword   = L"'";
    y3->Keyword   = L"'";
    x3->Connected = y3;

    y2->NestExprs.Add(x3);
    y1->SubExprs.Add(x2);
    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(x2).Add(x3).Add(y1).Add(y2).Add(y3);
    bits.Add(bit);

    y1->ParseCB = &(Qentem::Test::SubMatchNestMatch);
    /////////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"#    [[ op1=;'one';, op2=;'two'; opx=;'}}}q       }}}';}}}second op is:{//}");
    bit.Expected.Add(L"#    second op is:two");

    x1            = new Expression();
    y1            = new Expression();
    x1->Keyword   = L"[[ ";
    y1->Keyword   = L"{//}";
    x1->Connected = y1;

    x2            = new Expression();
    y2            = new Expression();
    x2->Keyword   = L"[[";
    y2->Keyword   = L"}}}";
    x2->Connected = y2;

    x3            = new Expression();
    y3            = new Expression();
    x3->Keyword   = L";'";
    y3->Keyword   = L"';";
    x3->Connected = y3;

    y1->SubExprs.Add(x2);
    y2->NestExprs.Add(x3);
    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(x2).Add(x3).Add(y1).Add(y2).Add(y3);
    bits.Add(bit);

    y1->ParseCB = &(Qentem::Test::SubMatchNestMatch);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"(1+2)").Add(L"( 1 + 2 )").Add(L"(1+)").Add(L"(+1)").Add(L"(1+2+3)").Add(L"(4+1+2+9)");
    bit.Expected.Add(L"3").Add(L"3").Add(L"0").Add(L"1").Add(L"6").Add(L"16");
    bit.Content.Add(L"(11+222)").Add(L"( 111 + 22 )").Add(L"(1111+)").Add(L"(+1111)").Add(L"(11+222+3333)");
    bit.Expected.Add(L"233").Add(L"133").Add(L"0").Add(L"1111").Add(L"3566");

    x1          = new Expression();
    x1->Keyword = L"+";
    x1->Flag    = Flags::SPLIT | Flags::TRIM;

    y1          = new Expression();
    y2          = new Expression();
    y1->Keyword = L"(";
    y2->Keyword = L")";
    y2->NestExprs.Add(x1);
    y1->Connected = y2;
    y2->Flag      = Flags::SPLITNEST;

    bit.Exprs.Add(y1);
    bit.Collect.Add(x1).Add(y1).Add(y2);
    bits.Add(bit);

    y2->ParseCB = ([](const String &block, const Match &item) noexcept->String {
        double number = 0.0;

        if (item.NestMatch.Size != 0) {
            String  r      = L"";
            UNumber temnum = 0;
            UNumber i      = 0;

            if (item.NestMatch[i].Length == 0) {
                // Plus sign at the biggening. Thats cool.
                i = 1;
            }

            Match *nm;
            for (; i < item.NestMatch.Size; i++) {
                nm = &(item.NestMatch[i]);
                if ((nm->Length == 0) || !String::ToNumber(block, temnum, nm->Offset, nm->Length)) {
                    return L"0";
                }

                number += temnum;
            }

            return String::FromNumber(number);
        }

        return L"0";
    });
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"2*3+4*5+7*8").Add(L"2*3+4*5").Add(L"1+2*3+4*5+1").Add(L"1+1*1+1");
    bit.Expected.Add(L"82").Add(L"26").Add(L"28").Add(L"3");

    x1          = new Expression();
    x1->Keyword = L"+";
    x1->Flag    = Flags::SPLIT | Flags::GROUPSPLIT;

    x2          = new Expression();
    x2->Keyword = L"*";
    x2->Flag    = Flags::SPLIT | Flags::GROUPSPLIT;

    x1->NestExprs.Add(x2);
    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(x2);
    bits.Add(bit);

    x1->ParseCB = ([](const String &block, const Match &item) noexcept->String {
        double number = 0.0;
        double temnum = 0.0;
        String r      = L"";

        Match *nm;
        for (UNumber i = 0; i < item.NestMatch.Size; i++) {
            nm = &(item.NestMatch[i]);

            if (nm->NestMatch.Size != 0) {
                r = Engine::Parse(block, nm->NestMatch, nm->Offset, nm->Offset + nm->Length);
            } else {
                r = String::Part(block, nm->Offset, nm->Length);
            }

            if ((r.Length == 0) || !String::ToNumber(r, temnum, 0, 0)) {
                return L"0";
            }

            number += temnum;
        }
        return String::FromNumber(number);
    });

    x2->ParseCB = ([](const String &block, const Match &item) noexcept->String {
        double number = 1.0;
        double temnum = 1.0;
        String r      = L"";

        Match *nm;
        for (UNumber i = 0; i < item.NestMatch.Size; i++) {
            nm = &(item.NestMatch[i]);
            if ((nm->Length == 0) || !String::ToNumber(block, temnum, nm->Offset, nm->Length)) {
                return L"0";
            }

            number *= temnum;
        }
        return String::FromNumber(number);
    });
    /////////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"(1++2)").Add(L"( 1 ++ 2 )").Add(L"(1++)").Add(L"(++1)").Add(L"(1++2++3)");
    bit.Expected.Add(L"3").Add(L"3").Add(L"0").Add(L"0").Add(L"6");
    bit.Content.Add(L"(11++222)").Add(L"( 111 ++ 22 )").Add(L"(1111++)").Add(L"(++1111)").Add(L"(11++222++3333)");
    bit.Expected.Add(L"233").Add(L"133").Add(L"0").Add(L"0").Add(L"3566");

    x1          = new Expression();
    x1->Keyword = L"++";
    x1->Flag    = Flags::SPLIT | Flags::TRIM;

    y1          = new Expression();
    y2          = new Expression();
    y1->Keyword = L"(";
    y2->Keyword = L")";
    y2->NestExprs.Add(x1);
    y1->Connected = y2;
    y2->Flag      = Flags::SPLITNEST;

    bit.Exprs.Add(y1);
    bit.Collect.Add(x1).Add(y1).Add(y2);
    bits.Add(bit);

    y2->ParseCB = ([](const String &block, const Match &item) noexcept->String {
        double number = 0.0;
        double temnum = 0.0;
        String r      = L"";

        Match *nm;
        for (UNumber i = 0; i < item.NestMatch.Size; i++) {
            nm = &(item.NestMatch[i]);
            if ((nm->Length == 0) || !String::ToNumber(block, temnum, nm->Offset, nm->Length)) {
                return L"0";
            }

            number += temnum;
        }
        return String::FromNumber(number);
    });
    /////////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    // Note: Crazy mojo!
    bit.Content.Add(L"yx31xy").Add(L"x+1").Add(L"xx+1").Add(L"xx+1+2").Add(L"1+xx").Add(L"1+yy").Add(L"1+3+yy");
    bit.Expected.Add(L"733137").Add(L"4").Add(L"34").Add(L"36").Add(L"34").Add(L"78").Add(L"81");
    bit.Content.Add(L"yx+1+xy+2+y").Add(L"yx4xy+xx8y+y1yyy");
    bit.Expected.Add(L"120").Add(L"148601");

    x1          = new Expression();
    x1->Keyword = L"+";
    x1->Flag    = Flags::SPLIT | Flags::GROUPSPLIT;

    x2          = new Expression();
    x2->Keyword = L"x";
    x2->ParseCB = ([](const String &block, const Match &item) noexcept->String {
        //
        return L"3";
    });

    x3          = new Expression();
    x3->Keyword = L"y";
    x3->ParseCB = ([](const String &block, const Match &item) noexcept->String {
        //
        return L"7";
    });

    bit.Exprs.Add(x1).Add(x2).Add(x3);
    bit.Collect.Add(x1).Add(x2).Add(x3);
    bits.Add(bit);

    x1->ParseCB = ([](const String &block, const Match &item) noexcept->String {
        double number = 0.0;
        double temnum = 0.0;
        String r      = L"";

        Match *nm;
        for (UNumber i = 0; i < item.NestMatch.Size; i++) {
            nm = &(item.NestMatch[i]);

            if (nm->NestMatch.Size != 0) {
                r = Engine::Parse(block, nm->NestMatch, nm->Offset, nm->Offset + nm->Length);
            } else {
                r = String::Part(block, nm->Offset, nm->Length);
            }

            if ((r.Length == 0) || !String::ToNumber(String::Trim(r), temnum, 0, 0)) {
                return L"0";
            }

            number += temnum;
        }
        return String::FromNumber(number);
    });
    /////////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"[(<>)]").Add(L"(<>)").Add(L"<>");
    bit.Expected.Add(L"[(u)]").Add(L"(u)").Add(L"u");

    x1            = new Expression();
    y1            = new Expression();
    x1->Keyword   = L"[";
    y1->Keyword   = L"]";
    x1->Connected = y1;
    x1->Flag      = Flags::POP;
    y1->Flag      = Flags::BUBBLE;
    y1->ParseCB   = ([](const String &block, const Match &item) noexcept->String {
        //
        return block;
    });

    x2            = new Expression();
    y2            = new Expression();
    x2->Keyword   = L"(";
    y2->Keyword   = L")";
    x2->Flag      = Flags::POP;
    y2->Flag      = Flags::BUBBLE;
    x2->Connected = y2;
    y2->ParseCB   = ([](const String &block, const Match &item) noexcept->String {
        //
        return block;
    });

    x3            = new Expression();
    y3            = new Expression();
    x3->Keyword   = L"<";
    y3->Keyword   = L">";
    x3->Connected = y3;
    y3->Replace   = L"u";

    x1->NestExprs.Add(x2);
    y1->NestExprs.Add(x2);
    x2->NestExprs.Add(x3);
    y2->NestExprs.Add(x3);

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(x2).Add(x3).Add(y1).Add(y2).Add(y3);
    bits.Add(bit);
    /////////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"{AAxx     yyyBBxx  yyyCC}");
    bit.Expected.Add(L"AABBCC");

    x1            = new Expression();
    y1            = new Expression();
    x1->Keyword   = L"{";
    y1->Keyword   = L"}";
    y1->Flag      = Flags::SPLITNEST;
    x1->Connected = y1;

    x2            = new Expression();
    y2            = new Expression();
    x2->Keyword   = L"xx";
    y2->Keyword   = L"yyy";
    y2->Flag      = Flags::SPLIT;
    x2->Connected = y2;

    y1->NestExprs.Add(x2);
    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(x2).Add(y1).Add(y2);
    bits.Add(bit);

    y1->ParseCB = ([](const String &block, const Match &item) noexcept->String {
        String nc = L"";

        for (UNumber i = 0; i < item.NestMatch.Size; i++) {
            nc += String::Part(block, item.NestMatch[i].Offset, item.NestMatch[i].Length);
        }

        return nc;
    });
    /////////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"1=1").Add(L"1==1").Add(L"1!=1");
    bit.Expected.Add(L"2").Add(L"1").Add(L"3");

    x1           = new Expression();
    x1->SearchCB = &(QRegex::OR);
    x1->Keyword  = L"==|=|!=";
    x1->Flag     = Flags::SPLIT | Flags::GROUPSPLIT;

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1);
    bits.Add(bit);

    x1->ParseCB = ([](const String &block, const Match &item) noexcept->String {
        //
        return String::FromNumber(item.Tag);
    });
    /////////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"ff11-22ss33-44ttt55-777e");
    bit.Expected.Add(L"HHHH");

    x1 = new Expression();
    x3 = new Expression();
    x2 = new Expression();
    y1 = new Expression();
    y2 = new Expression();

    x1->Keyword = L"ff";
    x2->Keyword = L"ss";
    x3->Keyword = L"ttt";
    y2->Keyword = L"e";
    y1->Keyword = L"-";

    x1->Connected = x2;
    x2->Connected = x3;
    x3->Connected = y2;

    y1->Flag = Flags::SPLIT;
    y2->Flag = Flags::SPLITNEST;
    y2->NestExprs.Add(y1);

    y2->ParseCB = ([](const String &block, const Match &item) noexcept->String {
        String s = L"";

        for (UNumber i = 0; i < item.NestMatch.Size; i++) {
            s += L"H";
        }

        return s;
    });

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(x2).Add(x3).Add(y1).Add(y2);
    bits.Add(bit);
    /////////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"< zz (  2  ) hh >");
    bit.Expected.Add(L"xxx");

    x1 = new Expression();
    x3 = new Expression();
    x2 = new Expression();
    y1 = new Expression();
    y3 = new Expression();

    x1->Keyword = L"<";
    x2->Keyword = L">";
    x3->Keyword = L"(";
    y3->Keyword = L")";

    x2->Replace = L"xxx";

    x1->Connected = x3;
    x3->Connected = y3;
    y3->Connected = x2;

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(x2).Add(x3).Add(y1).Add(y3);
    bits.Add(bit);
    /////////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"{-{-g{-g#j-}j-}-}");
    bit.Expected.Add(L"[jg]");
    bit.Content.Add(L"{-g#j-}BB{-y#i-}").Add(L"{-g#j#y#i-}").Add(L"{-AA{-g#j-}BB{-y#i-}CC-}");
    bit.Expected.Add(L"[jg]BB[iy]").Add(L"[iyjg]").Add(L"[[iy][jg]]");
    bit.Content.Add(L"{-{-x#o-}#{-x#o-}#{-x#o-}-}");
    bit.Expected.Add(L"[[ox][ox][ox]]");
    bit.Content.Add(L"{-w#{-d#{-x#o-}#{-r#e#t-}#b-}t#g{-c#{-x#o-}-}-}");
    bit.Expected.Add(L"[[[ox]c][b[ter][ox]d]w]");

    x1            = new Expression();
    y1            = new Expression();
    x1->Keyword   = L"{-";
    y1->Keyword   = L"-}";
    y1->Flag      = Flags::SPLITNEST;
    x1->Connected = y1;

    x2          = new Expression();
    x2->Flag    = Flags::SPLIT;
    x2->Keyword = L"#";

    y1->ParseCB = &(Qentem::Test::FlipSplit);

    y1->NestExprs.Add(x1).Add(x2);
    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(x2).Add(y1);
    bits.Add(bit);
    /////////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<br><s><br>");
    bit.Expected.Add(L"TR<wr>");

    x1 = new Expression();
    y1 = new Expression();
    x2 = new Expression();
    y2 = new Expression();

    x3 = new Expression();

    x1->Keyword = L"<";
    y1->Keyword = L">";
    x2->Keyword = L"<";
    y2->Keyword = L">";
    y2->Replace = L"TR";

    x3->Keyword = L"br";
    x3->Replace = L"wr";

    x1->Connected = y1;
    y1->Connected = x2;
    x2->Connected = y2;

    bit.Exprs.Add(x1).Add(x3);
    bit.Collect.Add(x1).Add(x2).Add(x3).Add(y1).Add(y2);
    bits.Add(bit);
    /////////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"{}")
        .Add(L"{  }")
        .Add(L"{ x }")
        .Add(L"{{ {  { x }}  }}")
        .Add(L"  &&&")
        .Add(L"  a & b & c & & d  &");
    bit.Expected.Add(L"{}").Add(L"{  }").Add(L"x").Add(L"x").Add(L"  &&&").Add(L"abcd");

    x1 = new Expression();
    y1 = new Expression();

    x2 = new Expression();

    x1->Keyword   = L"{";
    y1->Keyword   = L"}";
    x1->Connected = y1;
    y1->Flag      = Flags::BUBBLE | Flags::TRIM | Flags::DROPEMPTY;
    y1->NestExprs.Add(x1);

    x2->Keyword = L"&";
    x2->Flag    = Flags::SPLIT | Flags::GROUPSPLIT | Flags::TRIM | Flags::DROPEMPTY;

    bit.Exprs.Add(x1).Add(x2);
    bit.Collect.Add(x1).Add(x2).Add(y1);
    bits.Add(bit);

    y1->ParseCB = ([](const String &block, const Match &item) noexcept->String {
        if (block.Length > (item.OLength + item.CLength)) {
            return String::Part(block, item.OLength, block.Length - (item.OLength + item.CLength));
        }

        return L"0";
    });

    x2->ParseCB = ([](const String &block, const Match &item) noexcept->String {
        String nc = L"";

        for (UNumber i = 0; i < item.NestMatch.Size; i++) {
            nc += String::Part(block, item.NestMatch[i].Offset, item.NestMatch[i].Length);
        }

        return nc;
    });
    /////////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<div>ae<div>str0</div></diva>");
    bit.Expected.Add(L"<div>ae(str0)</diva>");

    x1 = new Expression();
    y1 = new Expression();

    y1->Flag = Flags::BUBBLE;

    x1->Keyword = L"<div>";
    y1->Keyword = L"</div>";

    x1->Connected = y1;
    y1->NestExprs.Add(x1);

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(y1);
    bits.Add(bit);

    y1->ParseCB = ([](const String &block, const Match &item) noexcept->String {
        String nc = L"(";
        nc += String::Part(block, item.OLength, block.Length - (item.OLength + item.CLength));
        nc += L")";
        return nc;
    });
    /////////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    // HTML/XML

    bit.Content.Add(L"<#>ae<#^>str0</#^></#>");
    bit.Expected.Add(L"(ae(str0))");

    bit.Content.Add(L"<div>ae<span>str0</span></div>");
    bit.Expected.Add(L"(ae(str0))");

    bit.Content.Add(L"<span>ae<span>str0</span></spans>");
    bit.Expected.Add(L"<span>ae(str0)</spans>");

    bit.Content.Add(L"<div>ae<span>str0</span></div>");
    bit.Expected.Add(L"(ae(str0))");

    bit.Content.Add(L" <   div id=1  >xx< / div > ");
    bit.Expected.Add(L" (xx) ");

    bit.Content.Add(L" <  div  >< div ><   span>str0</span   ></   div><   /div   > ");
    bit.Expected.Add(L" (((str0))) ");

    bit.Content.Add(L"<div>ae<span>str0</span></div><span>str1</span>");
    bit.Expected.Add(L"(ae(str0))(str1)");

    x1 = new Expression();
    y1 = new Expression();

    y1->Flag = Flags::BUBBLE;

    x1->Keyword = L"<#>";
    y1->Keyword = L"</#>";

    x1->Connected = y1;
    y1->NestExprs.Add(x1);

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(y1);
    bits.Add(bit);

    y1->ParseCB = ([](const String &block, const Match &item) noexcept->String {
        String nc = L"(";
        nc += String::Part(block, item.OLength, block.Length - (item.OLength + item.CLength));
        nc += L")";
        return nc;
    });

    x1->SearchCB = ([](const String &content, const Expression &expr, Match *item, UNumber &index, UNumber &ends,
                       UNumber limit) noexcept->UNumber {
        UNumber original_index = index;
        while ((index < limit) && content.Str[index] != L'<') {
            index++;
        }
        item->Tag = index;

        // Trim start.
        while (content.Str[++item->Tag] == L' ') {
        }

        ends = item->Tag;
        while ((ends < limit) && content.Str[++ends] != '>') {
            if (content.Str[ends] == L'/') {
                index = original_index;
                return 0;
            }
        }
        if (ends == limit) {
            index = original_index;
            return 0;
        }

        item->ID = item->Tag;
        while ((item->ID < ends) && (content.Str[item->ID] != L' ')) {
            if (content.Str[item->ID] == L'>') {
                break;
            }
            item->ID++;
        }

        return 1;
    });

    y1->SearchCB = ([](const String &content, const Expression &expr, Match *item, UNumber &index, UNumber &ends,
                       UNumber limit) noexcept->UNumber {
        UNumber original_index = index;
        UNumber tmp_index      = index;
        bool    slash          = false;

        while ((index < limit) && content.Str[index] != L'<') {
            index++;
        }

        tmp_index = index + 1;
        ends      = tmp_index;
        while ((ends < limit) && content.Str[ends] != '>') {
            if (content.Str[ends] == L'/') {
                tmp_index = ends;
                while (content.Str[++tmp_index] == L' ') {
                }
                ends  = tmp_index;
                slash = true;
            }

            ends++;
        }

        if ((ends == limit) || !slash) {
            index = original_index;
            return 0;
        }

        UNumber o_offset = item->Tag;
        while (o_offset < item->ID) {
            if (content.Str[o_offset++] != content.Str[tmp_index++]) {
                index = original_index;
                return 0;
            }
        }

        if ((content.Str[tmp_index] != L'>') && (content.Str[tmp_index] != L' ')) {
            index = original_index;
            return 0;
        }

        return 1;
    });
    ///////////////////////////////////////////

    return bits;
}

String Qentem::Test::FlipSplit(const String &block, const Match &item) noexcept {
    String nc = L"";

    if (item.NestMatch.Size != 0) {
        for (UNumber i = 0; i < item.NestMatch.Size; i++) {
            nc = Test::FlipSplit(block, item.NestMatch[i]) + nc;
        }

        if (item.NestMatch.Size > 1) {
            nc = String(L"[") + nc + L"]";
        }
    } else {
        nc = String::Part(block, item.Offset, item.Length);
    }

    return nc;
}

String Qentem::Test::SubMatchNestMatch(const String &block, const Match &item) noexcept {
    String nc = L"";
    if (item.SubMatch.Size != 0) {
        Match * sm     = &(item.SubMatch[0]);
        UNumber offset = sm->Offset + sm->Length;
        UNumber length = (item.Length - (sm->Length + item.CLength));
        nc             = String::Part(block, offset, length);

        if (sm->NestMatch.Size != 0) {
            Match *nm = &(sm->NestMatch[1]);
            nc += String::Part(block, (nm->Offset + nm->OLength), (nm->Length - (nm->CLength + nm->OLength)));
        }
    }

    // When bubbling; (when starts at 0)
    // if (item.SubMatch.Size != 0) {
    //     Match *sm     = &(item.SubMatch[0]);
    //     UNumber offset = sm->Length;
    //     UNumber length = (item.Length - (offset + item.CLength));
    //     nc            = String::Part(block,offset, length);

    //     if (sm->NestMatch.Size != 0) {
    //         Match *nm = &(sm->NestMatch[1]);
    //         nc +=
    //             String::Part(block,((nm->Offset + nm->OLength) - item.Offset), (nm->Length - (nm->CLength +
    //             nm->OLength)));
    //     }
    // }

    return nc;
}

String Qentem::Test::SubMatchZero(const String &block, const Match &item) noexcept {
    Match *sm = &(item.SubMatch[0]);
    return String::Part(block, (sm->Offset + sm->OLength), (sm->Length - (sm->CLength + sm->OLength)));

    // When bubbling; (when starts at 0)
    // String nc = L"";
    // if (item.SubMatch.Size != 0) {
    //     Match *sm     = &(item.SubMatch[0]);
    //     UNumber offset = (sm->Offset - item.Offset);

    //     if (sm->OLength > item.OLength) {
    //         offset += (sm->OLength - item.OLength);
    //     } else if (sm->OLength < item.OLength) {
    //         offset -= (item.OLength - sm->OLength);
    //     }

    //     UNumber length = (sm->Length - (sm->OLength + sm->CLength));

    //     nc = String::Part(block,offset, length);
    // }

    // return nc;
}
