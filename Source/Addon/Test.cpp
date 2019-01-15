
/**
 * Qentem Engine
 *
 * @brief     For testing Qentem Engine
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Addon/Test.hpp"

using Qentem::Test::TestBit;

void Qentem::Test::CleanBits(Array<TestBit> &bits) noexcept {
    for (size_t i = 0; i < bits.Size; i++) {
        for (size_t j = 0; j < bits[i].Exprslvl0.Size; j++) {
            delete bits[i].Exprslvl0[j];
        }
    }
}

Array<String> Qentem::Test::Extract(const String &content, const Array<Match> &items) noexcept {
    Array<String> matches = Array<String>(items.Size);
    for (size_t i = 0; i < items.Size; i++) {
        matches.Add(content.Part(items[i].Offset, items[i].Length));
    }

    return matches;
}

String Qentem::Test::DumbExpressions(const Expressions &expres, const String offset, size_t index,
                                     Qentem::Engine::Expression *expr) noexcept {
    if (expres.Size == 0) {
        return offset + L"No expressions!\n";
    }

    String tree      = offset + L"(" + String::ToString((float)expres.Size) + L") => [\n";
    String innoffset = L"    ";
    String l_offset  = offset + innoffset + innoffset;

    for (size_t i = index; i < expres.Size; i++) {

        if (expres[i] == expr) {
            tree += offset + innoffset + L"[" + String::ToString((float)i) + L"]: " + L"This.\n";
            continue;
        }

        tree += offset + innoffset + L"[" + String::ToString((float)i) + L"]: => {\n";

        tree += l_offset + L"Keyword: \"" + expres[i]->Keyword + L"\"\n";

        tree += l_offset + L"Flags: (" + String::ToString((float)(expres[i]->Flag)) + L")";

        if ((expres[i]->Flag & Flags::COMPACT) != 0) {
            tree += L" COMPACT";
        }

        if ((expres[i]->Flag & Flags::BUBBLE) != 0) {
            tree += L" BUBBLE";
        }

        if ((expres[i]->Flag & Flags::IGNORE) != 0) {
            tree += L" IGNORE";
        }

        if ((expres[i]->Flag & Flags::SPLIT) != 0) {
            tree += L" SPLIT";
        }

        if ((expres[i]->Flag & Flags::POP) != 0) {
            tree += L" POP";
        }
        tree += L"\n";

        tree += l_offset + L"Replace: \"" + expres[i]->Replace + L"\"\n";
        tree += l_offset + L"SearchCB: " +
                ((expres[i]->SearchCB != nullptr) ? String::ToString((float)((size_t)(expres[i]->SearchCB))) : L"N/A");
        tree += L"\n";
        tree += l_offset + L"ParseCB: " +
                ((expres[i]->ParseCB != nullptr) ? String::ToString((float)((size_t)(expres[i]->ParseCB))) : L"N/A");
        tree += L"\n";

        if (expres[i]->Tail != nullptr) {
            tree += l_offset + L"Tail:\n";
            tree += Test::DumbExpressions(Expressions().Add(expres[i]->Tail), innoffset + l_offset, 0, expres[i]);
        }

        if (expres[i]->NestExprs.Size != 0) {
            tree += l_offset + L"NestExprs:\n";
            tree += Test::DumbExpressions(expres[i]->NestExprs, innoffset + l_offset, 0, expres[i]);
        }

        if (expres[i]->SubExprs.Size != 0) {
            tree += l_offset + L"SubExprs:\n";
            tree += Test::DumbExpressions(expres[i]->SubExprs, innoffset + l_offset, 0, expres[i]);
        }

        tree += l_offset + L"}\n";
    }

    return tree + offset + L"]\n";
}

String Qentem::Test::DumbMatches(const String &content, const Array<Match> &matches, const String offset,
                                 size_t index) noexcept {
    if (matches.Size == 0) {
        return offset + L"No matches!\n";
    }

    Array<String> items = Test::Extract(content, matches);

    String innoffset = L"    ";
    String tree      = offset + L"(" + String::ToString((float)(matches.Size)) + L") => [\n";

    for (size_t i = index; i < items.Size; i++) {
        tree += innoffset + offset + L"[" + String::ToString((float)i) + L"]: \"" + items[i] + L"\"\n";

        if (matches[i].NestMatch.Size != 0) {
            tree += innoffset + offset + L"-NestMatch:\n";
            tree += Test::DumbMatches(content, matches[i].NestMatch, innoffset + innoffset + offset, 0);
        }

        if (matches[i].SubMatch.Size != 0) {
            tree += innoffset + offset + L"-SubMatch:\n";
            tree += Test::DumbMatches(content, matches[i].SubMatch, innoffset + innoffset + offset, 0);
        }
    }

    return tree + offset + L"]\n";
}

Array<TestBit> Qentem::Test::GetBits() noexcept {
    Array<TestBit> bits = Array<TestBit>();
    TestBit        bit;

    Expression *x1, *x2, *x3;
    Expression *y1, *y2, *y3;

    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;
    bit.Content.Add(L"").Add(L" ").Add(L"  ");
    bit.Expected.Add(L"").Add(L" ").Add(L"  ");

    x1 = new Expression(); // Empty

    bit.Exprs.Add(x1);
    bit.Exprslvl0.Add(x1);
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
    bit.Exprslvl0.Add(x1);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"(<1>)<2>");
    bit.Expected.Add(L"(<1>)m");

    x1          = new Expression();
    y1          = new Expression();
    x1->Flag    = Flags::IGNORE;
    x1->Tail    = y1;
    x1->Keyword = L"(";
    y1->Keyword = L")";

    x2          = new Expression();
    y2          = new Expression();
    x2->Tail    = y2;
    x2->Keyword = L"<";
    y2->Keyword = L">";
    x2->Replace = L"m";

    bit.Exprs.Add(x1);
    bit.Exprs.Add(x2);
    bit.Exprslvl0.Add(x1).Add(x2).Add(y1).Add(y2);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"(<1>)<2>");
    bit.Expected.Add(L"(<1>)<n>");

    x1          = new Expression();
    y1          = new Expression();
    x1->Flag    = Flags::IGNORE;
    x1->Tail    = y1;
    x1->Keyword = L"(";
    y1->Keyword = L")";

    x2          = new Expression();
    y2          = new Expression();
    x2->Flag    = Flags::COMPACT;
    x2->Tail    = y2;
    x2->Keyword = L"<";
    y2->Keyword = L">";
    x2->Replace = L"n";

    bit.Exprs.Add(x1);
    bit.Exprs.Add(x2);
    bit.Exprslvl0.Add(x1).Add(x2).Add(y1).Add(y2);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<").Add(L"<>").Add(L"<> ").Add(L" <>").Add(L" <> ").Add(L"  <>  ");
    bit.Expected.Add(L"<").Add(L"-").Add(L"- ").Add(L" -").Add(L" - ").Add(L"  -  ");
    bit.Content.Add(L"<a").Add(L"a<aa").Add(L"<aa> a").Add(L"a <a>").Add(L"a <a> a").Add(L"a  <aa>  a");
    bit.Expected.Add(L"<a").Add(L"a<aa").Add(L"- a").Add(L"a -").Add(L"a - a").Add(L"a  -  a");

    x1          = new Expression();
    y1          = new Expression();
    x1->Tail    = y1;
    x1->Keyword = L"<";
    y1->Keyword = L">";
    x1->Replace = L"-";

    bit.Exprs.Add(x1);
    bit.Exprslvl0.Add(x1).Add(y1);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<<").Add(L"<<>").Add(L"<<> ").Add(L" <<>").Add(L" <<> ").Add(L"  <<>  ");
    bit.Expected.Add(L"<<").Add(L"-").Add(L"- ").Add(L" -").Add(L" - ").Add(L"  -  ");
    bit.Content.Add(L"<<a").Add(L"a<<aa").Add(L"<<aa> a").Add(L"a <<a>").Add(L"a <<a> a").Add(L"a  <<aa>  a");
    bit.Expected.Add(L"<<a").Add(L"a<<aa").Add(L"- a").Add(L"a -").Add(L"a - a").Add(L"a  -  a");

    x1          = new Expression();
    y1          = new Expression();
    x1->Tail    = y1;
    x1->Keyword = L"<<";
    y1->Keyword = L">";
    x1->Replace = L"-";

    bit.Exprs.Add(x1);
    bit.Exprslvl0.Add(x1).Add(y1);
    bits.Add(bit);
    /////////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<").Add(L"<>>").Add(L"<>> ").Add(L" <>>").Add(L" <>> ").Add(L"  <>>  ");
    bit.Expected.Add(L"<").Add(L"-").Add(L"- ").Add(L" -").Add(L" - ").Add(L"  -  ");
    bit.Content.Add(L"<a").Add(L"a<aa").Add(L"<aa>> a").Add(L"a <a>>").Add(L"a <a>> a").Add(L"a  <aa>>  a");
    bit.Expected.Add(L"<a").Add(L"a<aa").Add(L"- a").Add(L"a -").Add(L"a - a").Add(L"a  -  a");

    x1          = new Expression();
    y1          = new Expression();
    x1->Tail    = y1;
    x1->Keyword = L"<";
    y1->Keyword = L">>";
    x1->Replace = L"-";

    bit.Exprs.Add(x1);
    bit.Exprslvl0.Add(x1).Add(y1);
    bits.Add(bit);
    /////////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<<").Add(L"<<>>").Add(L"<<>> ").Add(L" <<>>").Add(L" <<>> ").Add(L"  <<>>  ");
    bit.Expected.Add(L"<<").Add(L"-").Add(L"- ").Add(L" -").Add(L" - ").Add(L"  -  ");
    bit.Content.Add(L"<<a").Add(L"a<<aa").Add(L"<<aa>> a").Add(L"a <<a>>").Add(L"a <<a>> a").Add(L"a  <<aa>>  a");
    bit.Expected.Add(L"<<a").Add(L"a<<aa").Add(L"- a").Add(L"a -").Add(L"a - a").Add(L"a  -  a");

    x1          = new Expression();
    y1          = new Expression();
    x1->Tail    = y1;
    x1->Keyword = L"<<";
    y1->Keyword = L">>";
    x1->Replace = L"-";

    bit.Exprs.Add(x1);
    bit.Exprslvl0.Add(x1).Add(y1);
    bits.Add(bit);
    /////////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<").Add(L"->-").Add(L"<o<>").Add(L"<>o>").Add(L"<><>o>").Add(L"<><><>");
    bit.Expected.Add(L"<").Add(L"->-").Add(L"=").Add(L"=o>").Add(L"==o>").Add(L"===");

    x1          = new Expression();
    y1          = new Expression();
    x1->Tail    = y1;
    x1->Keyword = L"<";
    y1->Keyword = L">";
    x1->Replace = L"=";

    bit.Exprs.Add(x1);
    bit.Exprslvl0.Add(x1).Add(y1);
    bits.Add(bit);
    /////////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<o<>").Add(L"<>").Add(L"< < >>").Add(L"<0><<<1-0-0><1-0-1><1-0-2>><1-1>><2>");
    bit.Expected.Add(L"<o=").Add(L"=").Add(L"=").Add(L"===");
    bit.Content.Add(L"<0><1><<2-0><<2-1-0><2-1-1><<2-1-2-0><2-1-2-1><2-1-2-2><2-1-2-3>>><2-2><2-3>><3> ><");
    bit.Expected.Add(L"==== ><");

    x1          = new Expression();
    y1          = new Expression();
    x1->Tail    = y1;
    x1->Keyword = L"<";
    y1->Keyword = L">";
    x1->Replace = L"=";

    x1->NestExprs.Add(x1);
    bit.Exprs.Add(x1);
    bit.Exprslvl0.Add(x1).Add(y1);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<<o<<>").Add(L"<<>").Add(L"<< << >>").Add(L"<<0><<<<<<1-0-0><<1-0-1><<1-0-2>><<1-1>><<2>");
    bit.Expected.Add(L"<<o+").Add(L"+").Add(L"+").Add(L"+++");
    bit.Content.Add(
        L"<<0><<1><<<<2-0><<<<2-1-0><<2-1-1><<<<2-1-2-0><<2-1-2-1><<2-1-2-2><<2-1-2-3>>><<2-2><<2-3>><<3> ><<");
    bit.Expected.Add(L"++++ ><<");

    x1          = new Expression();
    y1          = new Expression();
    x1->Tail    = y1;
    x1->Keyword = L"<<";
    y1->Keyword = L">";
    x1->Replace = L"+";

    x1->NestExprs.Add(x1);
    bit.Exprs.Add(x1);
    bit.Exprslvl0.Add(x1).Add(y1);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<o<>>").Add(L"<>>").Add(L"< < >>>>").Add(L"<0>><<<1-0-0>><1-0-1>><1-0-2>>>><1-1>>>><2>>");
    bit.Expected.Add(L"<o_").Add(L"_").Add(L"_").Add(L"___");
    bit.Content.Add(
        L"<0>><1>><<2-0>><<2-1-0>><2-1-1>><<2-1-2-0>><2-1-2-1>><2-1-2-2>><2-1-2-3>>>>>><2-2>><2-3>>>><3>> >><");
    bit.Expected.Add(L"____ >><");

    x1          = new Expression();
    y1          = new Expression();
    x1->Tail    = y1;
    x1->Keyword = L"<";
    y1->Keyword = L">>";
    x1->Replace = L"_";

    x1->NestExprs.Add(x1);
    bit.Exprs.Add(x1);
    bit.Exprslvl0.Add(x1).Add(y1);
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

    x1          = new Expression();
    y1          = new Expression();
    x1->Tail    = y1;
    x1->Keyword = L"<<";
    y1->Keyword = L">>";
    x1->Replace = L"_";

    x1->NestExprs.Add(x1);
    bit.Exprs.Add(x1);
    bit.Exprslvl0.Add(x1).Add(y1);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<o<>").Add(L"<>").Add(L"< < >>").Add(L"<0><<<1-0-0><1-0-1><1-0-2>><1-1>><2>");
    bit.Expected.Add(L"<o()").Add(L"()").Add(L"( ( ))").Add(L"(0)(((1-0-0)(1-0-1)(1-0-2))(1-1))(2)");
    bit.Content.Add(L"<0><1><<2-0><<2-1-0><2-1-1><<2-1-2-0><2-1-2-1><2-1-2-2><2-1-2-3>>><2-2><2-3>><3> ><");
    bit.Expected.Add(L"(0)(1)((2-0)((2-1-0)(2-1-1)((2-1-2-0)(2-1-2-1)(2-1-2-2)(2-1-2-3)))(2-2)(2-3))(3) ><");

    x1          = new Expression();
    y1          = new Expression();
    x1->Flag    = Flags::BUBBLE;
    x1->Tail    = y1;
    x1->Keyword = L"<";
    y1->Keyword = L">";

    x1->NestExprs.Add(x1);
    bit.Exprs.Add(x1);
    bit.Exprslvl0.Add(x1).Add(y1);
    bits.Add(bit);

    x1->ParseCB = ([](const String &block, const Match &match) noexcept->String {
        String nc = L"(";
        nc += block.Part(match.OLength, block.Length - (match.OLength + match.CLength));
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

    x1          = new Expression();
    y1          = new Expression();
    x1->Flag    = Flags::BUBBLE;
    x1->Tail    = y1;
    x1->Keyword = L"(";
    y1->Keyword = L")";

    x2          = new Expression();
    y2          = new Expression();
    x2->Flag    = Flags::BUBBLE;
    x2->Tail    = y2;
    x2->Keyword = L"<";
    y2->Keyword = L">";

    x2->NestExprs.Add(x2); // Nest itself
    x1->NestExprs.Add(x2);
    bit.Exprs.Add(x1);
    bit.Exprslvl0.Add(x1).Add(x2).Add(y1).Add(y2);
    bits.Add(bit);

    x1->ParseCB = ([](const String &block, const Match &match) noexcept->String {
        String nc = L"=";
        nc += block;
        nc += L"=";
        return nc;
    });

    x2->ParseCB = ([](const String &block, const Match &match) noexcept->String {
        if (block == L"<3-U>") {
            return L"A";
        } else if (block == L"<2-A>") {
            return L"B";
        } else if (block == L"<1-B>") {
            return L"A+";
        }

        return L"err";
    });
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"[([x]),([y])--]==").Add(L"e[(]xx)] ").Add(L" [(x])] ");
    bit.Expected.Add(L"[]==").Add(L"e[] ").Add(L" [] ");

    x1          = new Expression();
    y1          = new Expression();
    x1->Flag    = Flags::COMPACT;
    x1->Tail    = y1;
    x1->Keyword = L"[";
    y1->Keyword = L"]";

    x2          = new Expression();
    y2          = new Expression();
    x2->Tail    = y2;
    x2->Keyword = L"(";
    y2->Keyword = L")";

    x1->NestExprs.Add(x2);
    bit.Exprs.Add(x1);
    bit.Exprslvl0.Add(x1).Add(x2).Add(y1).Add(y2);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;
    bit.Content.Add(L"[[[[((((x]]]])))),((((y))))--]]]]--");
    bit.Expected.Add(L"[[[[]]]]--");

    x1          = new Expression();
    y1          = new Expression();
    x1->Flag    = Flags::COMPACT;
    x1->Tail    = y1;
    x1->Keyword = L"[[[[";
    y1->Keyword = L"]]]]";

    x2          = new Expression();
    y2          = new Expression();
    x2->Tail    = y2;
    x2->Keyword = L"((((";
    y2->Keyword = L"))))";

    x1->NestExprs.Add(x2);
    bit.Exprs.Add(x1);
    bit.Exprslvl0.Add(x1).Add(x2).Add(y1).Add(y2);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"[xY]/").Add(L"    [     x     Y    ]     /  ").Add(L"[xY-yyyyyy]/");
    bit.Expected.Add(L"Y").Add(L"         Y      ").Add(L"Y-yyyyyy");

    x1          = new Expression();
    y1          = new Expression();
    x1->Tail    = y1;
    x1->Keyword = L"[";
    y1->Keyword = L"/";

    x2          = new Expression();
    y2          = new Expression();
    x2->Tail    = y2;
    x2->Keyword = L"x";
    y2->Keyword = L"]";

    x1->SubExprs.Add(x2);
    bit.Exprs.Add(x1);
    bit.Exprslvl0.Add(x1).Add(x2).Add(y1).Add(y2);
    bits.Add(bit);

    x1->ParseCB = &(Qentem::Test::SubMatchZero);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"[[xxxxxY]]]]]]]]]]///")
        .Add(L"    [[     xxxxx     Y    ]]]]]]]]]]     ///  ")
        .Add(L"[[xxxxxY-yyyyyy]]]]]]]]]]///");
    bit.Expected.Add(L"Y").Add(L"         Y      ").Add(L"Y-yyyyyy");

    x1          = new Expression();
    y1          = new Expression();
    x1->Tail    = y1;
    x1->Keyword = L"[[";
    y1->Keyword = L"///";

    x2          = new Expression();
    y2          = new Expression();
    x2->Tail    = y2;
    x2->Keyword = L"xxxxx";
    y2->Keyword = L"]]]]]]]]]]";

    x1->SubExprs.Add(x2);
    bit.Exprs.Add(x1);
    bit.Exprslvl0.Add(x1).Add(x2).Add(y1).Add(y2);
    bits.Add(bit);

    x1->ParseCB = &(Qentem::Test::SubMatchZero);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"#    [[ op1='one', op2='two' opx=']q       ]']second op is:/");
    bit.Expected.Add(L"#    second op is:two");

    x1          = new Expression();
    y1          = new Expression();
    x1->Tail    = y1;
    x1->Keyword = L"[[ ";
    y1->Keyword = L"/";

    x2          = new Expression();
    y2          = new Expression();
    x2->Tail    = y2;
    x2->Keyword = L"[[";
    y2->Keyword = L"]";

    x3          = new Expression();
    y3          = new Expression();
    x3->Tail    = y3;
    x3->Keyword = L"'";
    y3->Keyword = L"'";

    x2->NestExprs.Add(x3);
    x1->SubExprs.Add(x2);
    bit.Exprs.Add(x1);
    bit.Exprslvl0.Add(x1).Add(x2).Add(x3).Add(y1).Add(y2).Add(y3);
    bits.Add(bit);

    x1->ParseCB = &(Qentem::Test::SubMatchNestMatch);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"#    [[ op1=;'one';, op2=;'two'; opx=;'}}}q       }}}';}}}second op is:{//}");
    bit.Expected.Add(L"#    second op is:two");

    x1          = new Expression();
    y1          = new Expression();
    x1->Tail    = y1;
    x1->Keyword = L"[[ ";
    y1->Keyword = L"{//}";

    x2          = new Expression();
    y2          = new Expression();
    x2->Tail    = y2;
    x2->Keyword = L"[[";
    y2->Keyword = L"}}}";

    x3          = new Expression();
    y3          = new Expression();
    x3->Tail    = y3;
    x3->Keyword = L";'";
    y3->Keyword = L"';";

    x2->NestExprs.Add(x3);
    x1->SubExprs.Add(x2);
    bit.Exprs.Add(x1);
    bit.Exprslvl0.Add(x1).Add(x2).Add(x3).Add(y1).Add(y2).Add(y3);
    bits.Add(bit);

    x1->ParseCB = &(Qentem::Test::SubMatchNestMatch);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"#    [ op1='one', op2='two' opx=']q       ]']second op is:/");
    bit.Expected.Add(L"#    second op is:two");

    x1          = new Expression();
    y1          = new Expression();
    x1->Tail    = y1;
    x1->Keyword = L"[ ";
    y1->Keyword = L"/";

    x2          = new Expression();
    y2          = new Expression();
    x2->Tail    = y2;
    x2->Keyword = L"[";
    y2->Keyword = L"]";

    x3          = new Expression();
    y3          = new Expression();
    x3->Tail    = y3;
    x3->Keyword = L"'";
    y3->Keyword = L"'";

    x2->NestExprs.Add(x3);
    x1->SubExprs.Add(x2);
    bit.Exprs.Add(x1);
    bit.Exprslvl0.Add(x1).Add(x2).Add(x3).Add(y1).Add(y2).Add(y3);
    bits.Add(bit);

    x1->ParseCB = &(Qentem::Test::SubMatchNestMatch);
    /////////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"#    [[ op1=;'one';, op2=;'two'; opx=;'}}}q       }}}';}}}second op is:{//}");
    bit.Expected.Add(L"#    second op is:two");

    x1          = new Expression();
    y1          = new Expression();
    x1->Tail    = y1;
    x1->Keyword = L"[[ ";
    y1->Keyword = L"{//}";

    x2          = new Expression();
    y2          = new Expression();
    x2->Tail    = y2;
    x2->Keyword = L"[[";
    y2->Keyword = L"}}}";

    x3          = new Expression();
    y3          = new Expression();
    x3->Tail    = y3;
    x3->Keyword = L";'";
    y3->Keyword = L"';";

    x2->NestExprs.Add(x3);
    x1->SubExprs.Add(x2);
    bit.Exprs.Add(x1);
    bit.Exprslvl0.Add(x1).Add(x2).Add(x3).Add(y1).Add(y2).Add(y3);
    bits.Add(bit);

    x1->ParseCB = &(Qentem::Test::SubMatchNestMatch);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"1+2").Add(L" 1 + 2 ").Add(L"1+").Add(L"+1").Add(L"1+2+3").Add(L"4+1+2+9");
    bit.Expected.Add(L"3").Add(L"3").Add(L"0").Add(L"1").Add(L"6").Add(L"16");
    bit.Content.Add(L"11+222").Add(L" 111 + 22 ").Add(L"1111+").Add(L"+1111").Add(L"11+222+3333");
    bit.Expected.Add(L"233").Add(L"133").Add(L"0").Add(L"1111").Add(L"3566");

    x1          = new Expression();
    x1->Flag    = Flags::SPLIT;
    x1->Keyword = L"+";

    bit.Exprs.Add(x1);
    bit.Exprslvl0.Add(x1);
    bits.Add(bit);

    x1->ParseCB = ([](const String &block, const Match &match) noexcept->String {
        float number = 0.0f;

        if (match.NestMatch.Size > 0) {
            String r      = L"";
            float  temnum = 0.0f;
            size_t i      = 0;

            if (match.NestMatch[i].Length == 0) {
                // Plus sign at the biggening. Thats cool.
                i = 1;
            }

            Match *nm;
            for (; i < match.NestMatch.Size; i++) {
                nm = &(match.NestMatch[i]);
                r  = block.Part(nm->Offset, nm->Length);

                if ((r.Length == 0) || !String::ToNumber(r, temnum)) {
                    return L"0";
                }

                number += temnum;
            }
        }

        return String::ToString(number);
    });
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"2*3+4*5+7*8").Add(L"2*3+4*5").Add(L"1+2*3+4*5+1").Add(L"1+1*1+1");
    bit.Expected.Add(L"82").Add(L"26").Add(L"28").Add(L"3");

    x1          = new Expression();
    x1->Flag    = Flags::SPLIT;
    x1->Keyword = L"+";

    x2          = new Expression();
    x2->Flag    = Flags::SPLIT;
    x2->Keyword = L"*";

    x1->NestExprs.Add(x2);
    bit.Exprs.Add(x1);
    bit.Exprslvl0.Add(x1).Add(x2);
    bits.Add(bit);

    x1->ParseCB = ([](const String &block, const Match &match) noexcept->String {
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
    });

    x2->ParseCB = ([](const String &block, const Match &match) noexcept->String {
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
    });
    /////////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"1++2").Add(L" 1 ++ 2 ").Add(L"1++").Add(L"++1").Add(L"1++2++3");
    bit.Expected.Add(L"3").Add(L"3").Add(L"0").Add(L"0").Add(L"6");
    bit.Content.Add(L"11++222").Add(L" 111 ++ 22 ").Add(L"1111++").Add(L"++1111").Add(L"11++222++3333");
    bit.Expected.Add(L"233").Add(L"133").Add(L"0").Add(L"0").Add(L"3566");

    x1          = new Expression();
    x1->Flag    = Flags::SPLIT;
    x1->Keyword = L"++";

    bit.Exprs.Add(x1);
    bit.Exprslvl0.Add(x1);
    bits.Add(bit);

    x1->ParseCB = ([](const String &block, const Match &match) noexcept->String {
        float number = 0.0f;

        if (match.NestMatch.Size > 0) {
            String r      = L"";
            float  temnum = 0.0f;

            Match *nm;
            for (size_t i = 0; i < match.NestMatch.Size; i++) {
                nm = &(match.NestMatch[i]);
                r  = block.Part(nm->Offset, nm->Length);

                if ((r.Length == 0) || !String::ToNumber(r, temnum)) {
                    return L"0";
                }

                number += temnum;
            }
        }

        return String::ToString(number);
    });
    /////////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    // Note: Crazy mojo!
    bit.Content.Add(L"yx31xy").Add(L"x+1").Add(L"xx+1").Add(L"xx+1+2").Add(L"1+yy").Add(L"1+3+yy");
    bit.Content.Add(L"yx+1+xy+2+y").Add(L"yx4xy+xx8y+y1yyy");
    bit.Expected.Add(L"733137").Add(L"4").Add(L"34").Add(L"36").Add(L"78").Add(L"81");
    bit.Expected.Add(L"120").Add(L"148601");

    x1          = new Expression();
    x1->Flag    = Flags::SPLIT;
    x1->Keyword = L"+";

    x2          = new Expression();
    x2->Keyword = L"x";
    x2->ParseCB = ([](const String &block, const Match &match) noexcept->String {
        //
        return L"3";
    });

    x3          = new Expression();
    x3->Keyword = L"y";
    x3->ParseCB = ([](const String &block, const Match &match) noexcept->String {
        //
        return L"7";
    });

    bit.Exprs.Add(x1).Add(x2).Add(x3);
    bit.Exprslvl0.Add(x1).Add(x2).Add(x3);
    bits.Add(bit);

    x1->ParseCB = ([](const String &block, const Match &match) noexcept->String {
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
    });
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"[(<>)]").Add(L"(<>)").Add(L"<>");
    bit.Expected.Add(L"[(u)]").Add(L"(u)").Add(L"u");

    x1          = new Expression();
    y1          = new Expression();
    x1->Flag    = Flags::BUBBLE | Flags::POP;
    x1->Tail    = y1;
    x1->Keyword = L"[";
    y1->Keyword = L"]";
    x1->ParseCB = ([](const String &block, const Match &match) noexcept->String {
        //
        return block;
    });

    x2          = new Expression();
    y2          = new Expression();
    x2->Flag    = Flags::BUBBLE | Flags::POP;
    x2->Tail    = y2;
    x2->Keyword = L"(";
    y2->Keyword = L")";
    x2->ParseCB = ([](const String &block, const Match &match) noexcept->String {
        //
        return block;
    });

    x3          = new Expression();
    y3          = new Expression();
    x3->Tail    = y3;
    x3->Keyword = L"<";
    y3->Keyword = L">";
    x3->Replace = L"u";

    x2->NestExprs.Add(x3);
    x1->NestExprs.Add(x2);
    bit.Exprs.Add(x1);
    bit.Exprslvl0.Add(x1).Add(x2).Add(x3).Add(y1).Add(y2).Add(y3);
    bits.Add(bit);

    /////////////////////////////////////////////

    return bits;
}

String Qentem::Test::SubMatchNestMatch(const String &block, const Match &match) noexcept {
    String nc = L"";
    if (match.SubMatch.Size != 0) {
        Match *sm     = &(match.SubMatch[0]);
        size_t offset = sm->Offset + sm->Length;
        size_t length = (match.Length - (sm->Length + match.CLength));
        nc            = block.Part(offset, length);

        if (sm->NestMatch.Size != 0) {
            Match *nm = &(sm->NestMatch[1]);
            nc += block.Part((nm->Offset + nm->OLength), (nm->Length - (nm->CLength + nm->OLength)));
        }
    }

    // When bubbling; (when starts at 0)
    // if (match.SubMatch.Size != 0) {
    //     Match *sm     = &(match.SubMatch[0]);
    //     size_t offset = sm->Length;
    //     size_t length = (match.Length - (offset + match.CLength));
    //     nc            = block.Part(offset, length);

    //     if (sm->NestMatch.Size != 0) {
    //         Match *nm = &(sm->NestMatch[1]);
    //         nc +=
    //             block.Part(((nm->Offset + nm->OLength) - match.Offset), (nm->Length - (nm->CLength +
    //             nm->OLength)));
    //     }
    // }

    return nc;
}

String Qentem::Test::SubMatchZero(const String &block, const Match &match) noexcept {
    Match *sm = &(match.SubMatch[0]);
    return block.Part((sm->Offset + sm->OLength), (sm->Length - (sm->CLength + sm->OLength)));

    // When bubbling; (when starts at 0)
    // String nc = L"";
    // if (match.SubMatch.Size != 0) {
    //     Match *sm     = &(match.SubMatch[0]);
    //     size_t offset = (sm->Offset - match.Offset);

    //     if (sm->OLength > match.OLength) {
    //         offset += (sm->OLength - match.OLength);
    //     } else if (sm->OLength < match.OLength) {
    //         offset -= (match.OLength - sm->OLength);
    //     }

    //     size_t length = (sm->Length - (sm->OLength + sm->CLength));

    //     nc = block.Part(offset, length);
    // }

    // return nc;
}
