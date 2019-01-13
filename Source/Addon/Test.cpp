
/**
 * Qentem Engine
 *
 * @brief     For testing Qentem Engine
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Test.hpp"

using Qentem::Test::TestBit;

void Qentem::Test::CleanBits(Array<TestBit> &bits) noexcept {
    for (size_t i = 0; i < bits.Size(); i++) {
        for (size_t j = 0; j < bits[i].Exprslvl0.Size(); j++) {
            delete bits[i].Exprslvl0[j];
        }
    }
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
    x1->Tail    = y1;
    x1->Keyword = L"<";
    y1->Keyword = L">";
    x1->Flag    = Flags::BUBBLE;

    x1->NestExprs.Add(x1);
    bit.Exprs.Add(x1);
    bit.Exprslvl0.Add(x1).Add(y1);
    bits.Add(bit);

    x1->ParseCB = ([](const String &block, const Match &match) noexcept->String {
        String nc = L"(";
        nc += block;
        nc += L")";
        return nc;
    });
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;
    bit.Content.Add(L"(6<1-B>9)").Add(L"(6<1-B>9)(6<1-B>9)").Add(L"(6<1-<2-<3-U>>>9)");
    bit.Expected.Add(L"=6A+9=").Add(L"=6A+9==6A+9=").Add(L"=6A+9=");
    bit.Content.Add(L"(6<1-<2-<3-U>>>9)(6<1-<2-<3-U>>>9)");
    bit.Expected.Add(L"=6A+9==6A+9=");

    x1          = new Expression();
    y1          = new Expression();
    x1->Tail    = y1;
    x1->Keyword = L"(";
    y1->Keyword = L")";
    x1->Flag    = Flags::BUBBLE;

    x2          = new Expression();
    y2          = new Expression();
    x2->Tail    = y2;
    x2->Keyword = L"<";
    y2->Keyword = L">";
    x2->Flag    = Flags::BUBBLE;

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
        if (block == L"3-U") {
            return L"A";
        } else if (block == L"2-A") {
            return L"B";
        } else if (block == L"1-B") {
            return L"A+";
        }

        return L"err";
    });
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;
    bit.Content.Add(L"[([x]),([y])--]==").Add(L"e[(]xx)] ").Add(L" [(x])] ");
    bit.Expected.Add(L"==").Add(L"e ").Add(L"  ");

    x1          = new Expression();
    y1          = new Expression();
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
    bit.Expected.Add(L"--");

    x1          = new Expression();
    y1          = new Expression();
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
    x1->Flag    = Flags::COMPLETE;
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
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;
    bit.Content.Add(L"#    [[ op1=;'one';, op2=;'two'; opx=;'}}}q       }}}';}}}second op is:{//}");
    bit.Expected.Add(L"#    second op is:two");

    x1          = new Expression();
    y1          = new Expression();
    x1->Tail    = y1;
    x1->Flag    = Flags::COMPLETE;
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
    /////////////////////////////////////////////
    return bits;
}

String Qentem::Test::SubMatchNestMatch(const String &block, const Match &match) noexcept {
    String nc = L"";
    if (match.SubMatch.Size() != 0) {
        Match *sm     = &(match.SubMatch[0]);
        size_t offset = sm->Offset + sm->Length;
        size_t length = (match.Length - (sm->Length + match.CLength));
        nc            = block.Part(offset, length);

        if (sm->NestMatch.Size() != 0) {
            Match *nm = &(sm->NestMatch[1]);
            nc += block.Part((nm->Offset + nm->OLength), (nm->Length - (nm->CLength + nm->OLength)));
        }
    }

    // When bubbling;
    // if (match.SubMatch.Size() != 0) {
    //     Match *sm     = &(match.SubMatch[0]);
    //     size_t offset = sm->Length;
    //     size_t length = (match.Length - (offset + match.CLength));
    //     nc            = block.Part(offset, length);

    //     if (sm->NestMatch.Size() != 0) {
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

    // if BUBBLing
    // String nc = L"";
    // if (match.SubMatch.Size() != 0) {
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
