
/**
 * Qentem Test
 *
 * @brief     For testing Qentem Engine
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

/////////////////////////////////////////////
#ifndef TESTENGINE_H
#define TESTENGINE_H

#include <Extension/ALU.hpp>
#include <Extension/Document.hpp>
#include <Extension/Template.hpp>

namespace Qentem {
namespace Test {

struct TestBit {
    UNumber       Line = 0;
    Expressions   Exprs;
    Expressions   Collect;
    Array<String> Content;
    Array<String> Expected;
};

/////////////////////////////////////////////
static String FlipSplit(String const &block, Match const &item) noexcept;

static void CleanBits(Array<TestBit> &bits) noexcept {
    for (UNumber i = 0; i < bits.Size; i++) {
        for (UNumber j = 0; j < bits[i].Collect.Size; j++) {
            Qentem::Memory<Expression>::DeallocateBit(&bits[i].Collect[j]);
        }
    }
}

static Array<String> Extract(String const &content, Array<Match> const &items) noexcept {
    Array<String> matches = Array<String>();
    matches.SetCapacity(items.Size);

    for (UNumber i = 0; i < items.Size; i++) {
        matches.Add(String::Part(content, items[i].Offset, items[i].Length) + L" -> O:" +
                    String::FromNumber(items[i].Offset) + L" L:" + String::FromNumber(items[i].Length) + L" OL:" +
                    String::FromNumber(items[i].OLength) + L" CL:" + String::FromNumber(items[i].CLength));
    }

    return matches;
}

static String ReplaceNewLine(String const &content) {
    static Expressions find_keys;
    static Expression  find_key;

    if (find_keys.Size == 0) { // Caching
        find_key.Keyword = L'\n';
        find_key.Replace = L"\\n";
        find_keys.Add(&find_key);
    }

    return Engine::Parse(content, Engine::Search(content, find_keys));
}

static String DumpExpressions(Expressions const &expres, String const &offset, UNumber index = 0,
                              Engine::Expression *expr = nullptr) noexcept {
    if (expres.Size == 0) {
        return offset + L"No expressions!\n";
    }

    StringStream ss;

    ss += L'(';
    ss += String::FromNumber(static_cast<double>(expres.Size)) + L") => [\n";

    String innoffset = L"    ";
    String l_offset  = offset + innoffset + innoffset;

    for (UNumber i = index; i < expres.Size; i++) {

        if (expres[i] == expr) {
            ss += offset;
            ss += innoffset + L'[';
            ss += String::FromNumber(static_cast<double>(i)) + L"]: " + L"This.\n";
            continue;
        }

        ss += offset + innoffset + L'[' + String::FromNumber(static_cast<double>(i)) + L"]: => {\n";

        ss += l_offset + L"Keyword: " + expres[i]->Keyword + '\n';

        ss += l_offset + L"Flags: (" + String::FromNumber(static_cast<double>(expres[i]->Flag)) + L')';

        if ((expres[i]->Flag & Flags::COMPACT) != 0) {
            ss += L" COMPACT";
        }

        if ((expres[i]->Flag & Flags::BUBBLE) != 0) {
            ss += L" BUBBLE";
        }

        if ((expres[i]->Flag & Flags::NOPARSE) != 0) {
            ss += L" NOPARSE";
        }

        if ((expres[i]->Flag & Flags::IGNORE) != 0) {
            ss += L" IGNORE";
        }

        if ((expres[i]->Flag & Flags::SPLIT) != 0) {
            ss += L" SPLIT";
        }

        if ((expres[i]->Flag & Flags::SPLITNEST) != 0) {
            ss += L" SPLITNEST";
        }

        if ((expres[i]->Flag & Flags::GROUPSPLIT) != 0) {
            ss += L" GROUPSPLIT";
        }

        if ((expres[i]->Flag & Flags::POP) != 0) {
            ss += L" POP";
        }

        if ((expres[i]->Flag & Flags::ONCE) != 0) {
            ss += L" ONCE";
        }
        ss += L'\n';

        if (expres[i]->ID != 0) {
            ss += l_offset + L"ID: ";
            ss += String::FromNumber(expres[i]->ID);
            ss += '\n';
        }

        if (expres[i]->Replace.Length != 0) {
            ss += l_offset + L"Replace: " + ReplaceNewLine(expres[i]->Replace) + '\n';
        }

        if (expres[i]->ParseCB != nullptr) {
            ss += l_offset + L"ParseCB: Yes";
        } else {
            ss += l_offset + L"ParseCB: No";
        }
        ss += '\n';

        if (expres[i]->Connected != nullptr) {
            ss += l_offset + L"Next: ";
            ss += Test::DumpExpressions(Expressions().Add(expres[i]->Connected), innoffset + l_offset, 0, expres[i]);
        }

        if (expres[i]->NestExprs.Size != 0) {
            ss += l_offset + L"NestExprs: ";
            ss += Test::DumpExpressions(expres[i]->NestExprs, innoffset + l_offset, 0, expres[i]);
        }

        ss += innoffset + offset + L"}\n";
    }

    ss += offset + L"]\n";
    return ss.Eject();
}

static String DumpMatches(String const &content, Array<Match> const &matches, String const &offset,
                          UNumber index = 0) noexcept {
    if (matches.Size == 0) {
        return offset + L"No matches!\n";
    }

    Array<String> items = Test::Extract(content, matches);

    String innoffset = L"    ";
    String _array    = offset + L'(' + String::FromNumber(static_cast<double>(matches.Size)) + L") => [\n";

    // It should be (matches.size) not (items.Size), but they should be the same size!
    for (UNumber i = index; i < items.Size; i++) {
        _array += innoffset + offset + L'[' + String::FromNumber(static_cast<double>(i)) + L"]: " +
                  ReplaceNewLine(items[i]) + L'\n';

        if (matches[i].NestMatch.Size != 0) {
            _array += innoffset + offset + L"-NestMatch:\n";
            _array += Test::DumpMatches(content, matches[i].NestMatch, innoffset + innoffset + offset, 0);
        }
    }

    return _array + offset + L"]\n";
}

static Array<TestBit> GetALUBits() noexcept {
    Array<TestBit> bits = Array<TestBit>();
    TestBit        bit;

    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"+1").Add(L" +1 ").Add(L"1+").Add(L" 1+ ").Add(L"1+ ").Add(L" 1+").Add(L"1+1").Add(L" 1 + 1 ");
    bit.Expected.Add(L"1").Add(L"1").Add(L"1").Add(L"1").Add(L"1").Add(L"1").Add(L"2").Add(L"2");

    bit.Content.Add(L" -1 ").Add(L"-1").Add(L"1-").Add(L" 1- ").Add(L"1- ").Add(L" 1-").Add(L"1-1").Add(L" 1 - 1 ");
    bit.Expected.Add(L"-1").Add(L"-1").Add(L"1").Add(L"1").Add(L"1").Add(L"1").Add(L"0").Add(L"0");

    bit.Content.Add(L"1-").Add(L"--1").Add(L"1--").Add(L"1--1").Add(L"1---1");
    bit.Expected.Add(L"1").Add(L"1").Add(L"1").Add(L"2").Add(L"0");

    bit.Content.Add(L"-1--1");
    bit.Expected.Add(L"0");

    bit.Content.Add(L"+1+1+2+11").Add(L"1+1+2+11+").Add(L"1-1-2-11").Add(L"-1-1-2-11");
    bit.Expected.Add(L"15").Add(L"15").Add(L"-13").Add(L"-15");

    bit.Content.Add(L"+1-2+3").Add(L"+1+2-3").Add(L"-1-2+3-").Add(L"-1+2-3-");
    bit.Expected.Add(L"2").Add(L"0").Add(L"0").Add(L"-2");

    ////
    bit.Content.Add(L"*1").Add(L" 1*1 ").Add(L"3*5").Add(L" 5*3**1*2 ").Add(L" 5*3*1*2 ");
    bit.Expected.Add(L"0").Add(L"1").Add(L"15").Add(L"0").Add(L"30");

    bit.Content.Add(L"/1").Add(L" 1/1 ").Add(L"100/5").Add(L" 5/3//1/2 ").Add(L" 32/2/2/2/2/2 ");
    bit.Expected.Add(L"0").Add(L"1").Add(L"20").Add(L"0").Add(L"1");

    ////
    bit.Content.Add(L"2*1+1*2").Add(L"1+1*2");
    bit.Expected.Add(L"4").Add(L"3");

    ////
    bit.Content.Add(L"4^0").Add(L"4^2").Add(L"8^2").Add(L"8^1");
    bit.Expected.Add(L"1").Add(L"16").Add(L"64").Add(L"8");

    ////
    bit.Content.Add(L"2=2").Add(L"2==2").Add(L"1=2").Add(L"2==1").Add(L"2==1+1").Add(L"2/2==1");
    bit.Expected.Add(L"1").Add(L"1").Add(L"0").Add(L"0").Add(L"1").Add(L"1");

    bit.Content.Add(L"2!=2").Add(L"1!=2").Add(L"1<=2").Add(L"2>=2").Add(L"2<=2").Add(L"2<=1").Add(L"1>=2").Add(L"2>=1");
    bit.Expected.Add(L"0").Add(L"1").Add(L"1").Add(L"1").Add(L"1").Add(L"0").Add(L"0").Add(L"1");

    bit.Content.Add(L"3 + 9 - 1 - -1 + 2 == 14");
    bit.Expected.Add(L"1");
    ////
    bit.Exprs = Qentem::ALU::MathExprs;
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"(2*(1+1))").Add(L"((1+10)+(5*5))");
    bit.Expected.Add(L"4").Add(L"36");

    bit.Content.Add(
        L"(((2* (1 * 3)) + 1 - 4) + (((10 - 5) - 6 + ((1 + 1) + (1 + 1))) * (8 / 4 + 1)) - (1) + (1) + 2 = 14)");
    bit.Expected.Add(L"1");

    bit.Exprs = Qentem::ALU::ParensExprs;
    bits.Add(bit);
    ///////////////////////////////////////////
    return bits;
}

static Array<TestBit> GetTemplateBits() noexcept {
    Array<TestBit> bits = Array<TestBit>();
    TestBit        bit;

    static Qentem::Document data = Qentem::Document();

    data[L"r1"] = L"Familly";
    data[L"e1"] = L"";
    data[L"e2"] = L" ";

    data[L"lvl2"] = Qentem::Document();

    data[L"lvl2"][L"r1"] = L"l2";
    data[L"lvl2"][L"e1"] = L"";
    data[L"lvl2"][L"e2"] = L" ";

    data[L"lvl2"][L"numbers"] = Array<double>().Add(1).Add(2);
    data[L"lvl2"][L"strings"] = Array<String>().Add(L"N1").Add(L"N2").Add(L"N3");

    Template::Data = &data;

    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"{v:r1}  ").Add(L" {v:r2}  ").Add(L"{v:e1} ").Add(L"{v:e2}");
    bit.Expected.Add(L"Familly  ").Add(L" r2  ").Add(L" ").Add(L" ");

    bit.Content.Add(L"{v:lvl2[r1]}").Add(L"{v:lvl2[r2]}").Add(L"{v:lvl2[e1]}").Add(L" {v:lvl2[e2]} ");
    bit.Expected.Add(L"l2").Add(L"lvl2[r2]").Add(L"").Add(L"   ");

    bit.Content.Add(L"{v:lvl2[numbers][0]}").Add(L"{v:lvl2[numbers][1]}").Add(L"{v:lvl2[numbers][3]}");
    bit.Expected.Add(L"1").Add(L"2").Add(L"lvl2[numbers][3]");

    bit.Content.Add(L"{v:lvl2[strings][0]}").Add(L"{v:lvl2[strings][1]}").Add(L"{v:lvl2[strings][3]}");
    bit.Expected.Add(L"N1").Add(L"N2").Add(L"lvl2[strings][3]");

    ////

    bit.Content.Add(L"{iif case=\"10\" true =\"{iif case =\"1\" true=\"5\"}\"}");
    bit.Expected.Add(L"5");

    bit.Content.Add(L"{iif case=\"1\" false=\"10\"}");
    bit.Expected.Add(L"");

    bit.Content.Add(L"{iif case = \"1\" true = \"1\" false = \"0\"}");
    bit.Expected.Add(L"1");

    bit.Content.Add(L"{iif case=\"0\" true=\"1\" false=\"0\"}");
    bit.Expected.Add(L"0");

    bit.Content.Add(
        L"{iif case=\"((2* (1 * 3)) + 1 - 4) + (((10 - 5) - 6 + ((1 + 1) + (1 + 1))) * (8 / 4 + 1)) - (1) + (1) + 2\" true =\"14\"}");
    bit.Expected.Add(L"14");

    bit.Content.Add(L"{iif case=\"0\" true=\"1\"}");
    bit.Expected.Add(L"");

    bit.Content.Add(L"{iif case=\"{v:lvl2[numbers][1]} = 2\" true=\"it's true! \" false =\"it's false\"}");
    bit.Expected.Add(L"it's true! ");

    bit.Content.Add(L"{iif case=\"0\" true=\"{iif case=\"1\" true=\"5\"}\"}");
    bit.Expected.Add(L"");

    bit.Content.Add(L"{iif case=\"0\" false=\"{iif case=\"1\" true=\"35.5\"}\"}");
    bit.Expected.Add(L"35.5");

    ////

    bit.Content.Add(L"<loop set =\"lvl2[numbers]\" var =\"id\">l-[id]): {v:lvl2[numbers][id]}\n </loop>");
    bit.Expected.Add(L"l-[0]): 1\n l-[1]): 2\n ");

    bit.Content.Add(
        L"<loop set=\"lvl2[numbers]\" var=\"id\"><loop set =\"lvl2[numbers]\" var =\"i2\">l-[id]-[i2]):{v:lvl2[numbers][i2]}\n </loop></loop>");
    bit.Expected.Add(L"l-[0]-[0]):1\n l-[0]-[1]):2\n l-[1]-[0]):1\n l-[1]-[1]):2\n ");

    bit.Content.Add("<loop set=\"lvl2[strings]\" var =\"sw\">l-[sw]): {v:lvl2[strings][sw]}\n</loop>");
    bit.Expected.Add(L"l-[0]): N1\nl-[1]): N2\nl-[2]): N3\n");

    bit.Content.Add(
        L"Space <loop set=\"lvl2[strings]\" var=\"sw\">l-[sw]): {v:lvl2[strings][sw]}\n</loop><loop set =\"lvl2[numbers]\" var=\"id\">l-[id]): {v:lvl2[numbers][id]}\n </loop>");
    bit.Expected.Add(L"Space l-[0]): N1\nl-[1]): N2\nl-[2]): N3\nl-[0]): 1\n l-[1]): 2\n ");

    ////

    bit.Content.Add(L" <if case=\"1\"> 5 </if> ")
        .Add(L"<if case=\"0\">5</if> ")
        .Add(L" <if case=\"100\"><if case=\"1\">6</if></if>")
        .Add(L"<if case=\"8\"><if case=\"88\"><if case=\"888\">7</if></if></if>")
        .Add(L"<if case=\"1\"><if case=\"1\"><if case=\"1\"><if case=\"1\">8</if></if></if></if>");
    bit.Expected.Add(L"  5  ").Add(L" ").Add(L" 6").Add(L"7").Add(L"8");

    bit.Content.Add(L"<if case=\"1\">4<else /> 6 </if>")
        .Add(L"<if case=\"0\"> 4 <else />6</if>")
        .Add(L"<if case=\"0\"> 4 <else /><if case=\"0\"> 4 <else /><if case=\"0\"> 4 <else />7</if></if></if>");
    bit.Expected.Add(L"4").Add(L"6").Add(L"7");

    bit.Content.Add(L"<if case=\"0\"><else />91</if>");
    bit.Expected.Add(L"91");

    bit.Content.Add(L"<if case=\"0\">4<elseif case=\"{v:lvl2[numbers][0]} = 1\" /> 6 </if>");
    bit.Expected.Add(L" 6 ");

    bit.Content.Add(L"<if case=\"1\">4<elseif case=\"{v:lvl2[numbers][0]} = 1\" /> 6 </if>");
    bit.Expected.Add(L"4");

    bit.Content.Add(L"<if case=\"0\">4<elseif case=\"0\" /> 6 <elseif case=\"1\" />9</if>");
    bit.Expected.Add(L"9");

    bit.Content.Add(L"<if case=\"0\">4<elseif case=\"0\" /> 6 <else />91</if>");
    bit.Expected.Add(L"91");

    ////

    bit.Exprs = Template::TagsAll;
    bits.Add(bit);
    ///////////////////////////////////////////

    return bits;
}

static Array<TestBit> GetEngineBits() noexcept {
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
    bit.Content.Add(L'-').Add(L" -- ").Add(L"- - - -");
    bit.Expected.Add(L'*').Add(L" ** ").Add(L"* * * *");

    Memory<Expression>::AllocateBit(&x1);
    x1->Keyword = L'-';
    x1->Replace = L'*';

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;
    bit.Content.Add(L"----");
    bit.Expected.Add(L"****");

    Memory<Expression>::AllocateBit(&x1);
    x1->Keyword = L'-';

    Memory<Expression>::AllocateBit(&x2);
    x2->Keyword = L"--";

    x1->ParseCB = ([](String const &block, Match const &item) noexcept->String { return L'*'; });

    bit.Exprs.Add(x1).Add(x2);
    bit.Collect.Add(x1).Add(x2);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;
    bit.Content.Add(L" - ").Add(L" -- ").Add(L" {{{9}} ");
    bit.Expected.Add(L" 0-0 ").Add(L" 0-0 ").Add(L" 3-2 ");

    Memory<Expression>::AllocateBit(&x1);
    x1->Keyword = L'-';

    Memory<Expression>::AllocateBit(&x2);
    x2->Keyword = L"--";

    Memory<Expression>::AllocateBit(&x3);
    x3->Keyword = L"{{{";

    Memory<Expression>::AllocateBit(&y3);
    y3->Keyword   = L"}}";
    x3->Connected = y3;

    x1->ParseCB = x2->ParseCB = y3->ParseCB = ([](String const &block, Match const &item) noexcept->String {
        return String::FromNumber(item.OLength) + L'-' + String::FromNumber(item.CLength);
    });

    bit.Exprs.Add(x2).Add(x1).Add(x3);
    bit.Collect.Add(x1).Add(x2).Add(x3).Add(y3);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"(<1>)<2>");
    bit.Expected.Add(L"(<1>)m");

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&y1);
    x1->Keyword   = L'(';
    y1->Keyword   = L')';
    x1->Connected = y1;
    y1->Flag      = Flags::NOPARSE;

    Memory<Expression>::AllocateBit(&x2);
    Memory<Expression>::AllocateBit(&y2);
    x2->Keyword   = L'<';
    y2->Keyword   = L'>';
    x2->Connected = y2;
    y2->Replace   = L'm';

    bit.Exprs.Add(x1);
    bit.Exprs.Add(x2);
    bit.Collect.Add(x1).Add(x2).Add(y1).Add(y2);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"(<1>)<2>");
    bit.Expected.Add(L"(<1>)<n>");

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&y1);
    x1->Keyword   = L'(';
    y1->Keyword   = L')';
    x1->Connected = y1;
    y1->Flag      = Flags::NOPARSE;

    Memory<Expression>::AllocateBit(&x2);
    Memory<Expression>::AllocateBit(&y2);
    x2->Keyword   = L'<';
    y2->Keyword   = L'>';
    x2->Connected = y2;
    y2->Flag      = Flags::COMPACT;
    y2->Replace   = L'n';

    bit.Exprs.Add(x1);
    bit.Exprs.Add(x2);
    bit.Collect.Add(x1).Add(x2).Add(y1).Add(y2);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L'<').Add(L"<>").Add(L"<> ").Add(L" <>").Add(L" <> ").Add(L"  <>  ");
    bit.Expected.Add(L'<').Add(L'-').Add(L"- ").Add(L" -").Add(L" - ").Add(L"  -  ");
    bit.Content.Add(L"<a").Add(L"a<aa").Add(L"<aa> a").Add(L"a <a>").Add(L"a <a> a").Add(L"a  <aa>  a");
    bit.Expected.Add(L"<a").Add(L"a<aa").Add(L"- a").Add(L"a -").Add(L"a - a").Add(L"a  -  a");

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&y1);
    x1->Keyword   = L'<';
    y1->Keyword   = L'>';
    x1->Connected = y1;
    y1->Replace   = L'-';

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(y1);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<<").Add(L"<<>").Add(L"<<> ").Add(L" <<>").Add(L" <<> ").Add(L"  <<>  ");
    bit.Expected.Add(L"<<").Add(L'-').Add(L"- ").Add(L" -").Add(L" - ").Add(L"  -  ");
    bit.Content.Add(L"<<a").Add(L"a<<aa").Add(L"<<aa> a").Add(L"a <<a>").Add(L"a <<a> a").Add(L"a  <<aa>  a");
    bit.Expected.Add(L"<<a").Add(L"a<<aa").Add(L"- a").Add(L"a -").Add(L"a - a").Add(L"a  -  a");

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&y1);
    x1->Keyword   = L"<<";
    y1->Keyword   = L'>';
    x1->Connected = y1;
    y1->Replace   = L'-';

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(y1);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L'<').Add(L"<>>").Add(L"<>> ").Add(L" <>>").Add(L" <>> ").Add(L"  <>>  ");
    bit.Expected.Add(L'<').Add(L'-').Add(L"- ").Add(L" -").Add(L" - ").Add(L"  -  ");
    bit.Content.Add(L"<a").Add(L"a<aa").Add(L"<aa>> a").Add(L"a <a>>").Add(L"a <a>> a").Add(L"a {8} <aa>>  a");
    bit.Expected.Add(L"<a").Add(L"a<aa").Add(L"- a").Add(L"a -").Add(L"a - a").Add(L"a {8} -  a");

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&y1);
    Memory<Expression>::AllocateBit(&x2);
    Memory<Expression>::AllocateBit(&y2);

    x1->Keyword   = L'<';
    y1->Keyword   = L">>";
    x1->Connected = y1;
    x2->Keyword   = L'{';
    y2->Keyword   = L'}';
    y2->Flag      = Flags::IGNORE;
    x2->Connected = y2;
    y1->Replace   = L'-';

    bit.Exprs.Add(x1).Add(x2);
    bit.Collect.Add(x1).Add(x2).Add(y1).Add(y2);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<<").Add(L"<<>>").Add(L"<<>> ").Add(L" <<>>").Add(L" <<>> ").Add(L"  <<>>  ");
    bit.Expected.Add(L"<<").Add(L'-').Add(L"- ").Add(L" -").Add(L" - ").Add(L"  -  ");
    bit.Content.Add(L"<<a").Add(L"a<<aa").Add(L"<<aa>> a").Add(L"a <<a>>").Add(L"a <<a>> a").Add(L"a  <<aa>>  a");
    bit.Expected.Add(L"<<a").Add(L"a<<aa").Add(L"- a").Add(L"a -").Add(L"a - a").Add(L"a  -  a");

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&y1);
    x1->Keyword   = L"<<";
    y1->Keyword   = L">>";
    x1->Connected = y1;
    y1->Replace   = L'-';

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(y1);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L'<').Add(L"->-").Add(L"<o<>").Add(L"<>o>").Add(L"<><>o>").Add(L"<><><>");
    bit.Expected.Add(L'<').Add(L"->-").Add(L'=').Add(L"=o>").Add(L"==o>").Add(L"===");

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&y1);
    x1->Keyword   = L'<';
    y1->Keyword   = L'>';
    x1->Connected = y1;
    y1->Replace   = L'=';

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(y1);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<>").Add(L"<><>").Add(L"<><><>");
    bit.Expected.Add(L'=').Add(L"=<>").Add(L"=<><>");

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&y1);
    x1->Keyword   = L'<';
    y1->Keyword   = L'>';
    x1->Connected = y1;
    y1->Flag      = Flags::ONCE;
    y1->Replace   = L'=';

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(y1);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<<> ").Add(L"<<<> ").Add(L"<>> ").Add(L" <>>> ");
    bit.Content.Add(L"<<>").Add(L"<<<>").Add(L"<>>").Add(L" <>>>");
    bit.Content.Add(L"<>").Add(L"<o<> ").Add(L" < < >>").Add(L"<0><<<1-0-0><1-0-1><1-0-2>><1-1>><2>");
    bit.Content.Add(L"<> ").Add(L"<o< > ").Add(L" < < > > ");

    bit.Expected.Add(L"<= ").Add(L"<<= ").Add(L"=> ").Add(L" =>> ");
    bit.Expected.Add(L"<=").Add(L"<<=").Add(L"=>").Add(L" =>>");
    bit.Expected.Add(L'=').Add(L"<o= ").Add(L" =").Add(L"===");
    bit.Expected.Add(L"= ").Add(L"<o= ").Add(L" = ");

    bit.Content.Add(L"<0><1><<2-0><<2-1-0><2-1-1><<2-1-2-0><2-1-2-1><2-1-2-2><2-1-2-3>>><2-2><2-3>><3> ><");
    bit.Expected.Add(L"==== ><");

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&y1);
    x1->Keyword   = L'<';
    y1->Keyword   = L'>';
    x1->Connected = y1;
    y1->Replace   = L'=';
    y1->NestExprs.Add(x1);

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(y1);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<0<X>").Add(L"<0<X> ").Add(L"<0<<X>");
    bit.Expected.Add(L"W").Add(L"W ").Add(L"W");

    bit.Content.Add(L"<0<xxxX>").Add(L"<0-<0-X>").Add(L"<0-<0X>").Add(L"<0<<0X>").Add(L"<0>X>").Add(L"<0X>X>X>");
    bit.Expected.Add(L"W").Add(L"<0-W").Add(L"<0-W").Add(L"<0<W").Add(L"W").Add(L"WX>X>");

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&y1);
    x1->Keyword   = L"<0";
    y1->Keyword   = L"X>";
    x1->Connected = y1;
    y1->Replace   = L'W';
    y1->NestExprs.Add(x1);

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(y1);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;
    bit.Content.Add(L"<0<0X> ").Add(L"<0<088888X> ").Add(L"<0<<0X> ").Add(L"<0>X> ").Add(L" <0X>X>X> ");
    bit.Expected.Add(L"<0W ").Add(L"<0W ").Add(L"<0<W ").Add(L"W ").Add(L" WX>X> ");

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&y1);
    x1->Keyword   = L"<0";
    y1->Keyword   = L"X>";
    x1->Connected = y1;
    y1->Replace   = L'W';
    y1->NestExprs.Add(x1);

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(y1);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<<<> ");
    bit.Content.Add(L"<<>>").Add(L" <<>> ").Add(L"<<> ").Add(L"<<o<<> ");
    bit.Content.Add(L"<< << >>").Add(L"<<0><<<<<<1-0-0><<1-0-1><<1-0-2>><<1-1>><<2>");
    bit.Expected.Add(L"+ ").Add(L"+>").Add(L" +> ").Add(L"+ ").Add(L"<<o+ ").Add(L'+').Add(L"+++");
    bit.Content.Add(
        L"<<0><<1><<<<2-0><<<<2-1-0><<2-1-1><<<<2-1-2-0><<2-1-2-1><<2-1-2-2><<2-1-2-3>>><<2-2><<2-3>><<3> ><<");
    bit.Expected.Add(L"++++ ><<");

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&y1);
    x1->Keyword   = L"<<";
    y1->Keyword   = L'>';
    x1->Connected = y1;
    y1->Replace   = L'+';
    y1->NestExprs.Add(x1);

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(y1);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<o<>>").Add(L"<>>").Add(L"< < >>>>").Add(L"<0>><<<1-0-0>><1-0-1>><1-0-2>>>><1-1>>>><2>>");
    bit.Expected.Add(L"<o_").Add(L'_').Add(L'_').Add(L"___");
    bit.Content.Add(
        L"<0>><1>><<2-0>><<2-1-0>><2-1-1>><<2-1-2-0>><2-1-2-1>><2-1-2-2>><2-1-2-3>>>>>><2-2>><2-3>>>><3>> >><");
    bit.Expected.Add(L"____ >><");

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&y1);
    x1->Keyword   = L'<';
    y1->Keyword   = L">>";
    x1->Connected = y1;
    y1->Replace   = L'_';
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
    bit.Expected.Add(L"<<o_").Add(L'_').Add(L'_').Add(L"___");
    bit.Content.Add(
        L"<<0>><<1>><<<<2-0>><<<<2-1-0>><<2-1-1>><<<<2-1-2-0>><<2-1-2-1>><<2-1-2-2>><<2-1-2-3>>>>>><<2-2>><<2-3>>>><<3>> >><<");
    bit.Expected.Add(L"____ >><<");

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&y1);
    x1->Keyword   = L"<<";
    y1->Keyword   = L">>";
    x1->Connected = y1;
    y1->Replace   = L'_';
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

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&y1);
    x1->Keyword   = L'<';
    y1->Keyword   = L'>';
    x1->Connected = y1;
    y1->Flag      = Flags::BUBBLE;
    y1->NestExprs.Add(x1);

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(y1);
    bits.Add(bit);

    y1->ParseCB = ([](String const &block, Match const &item) noexcept->String {
        String nc = L'(';
        nc += String::Part(block, item.OLength, block.Length - (item.OLength + item.CLength));
        nc += L')';
        return nc;
    });
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"(6<1-B>9)").Add(L"(6<1-B>9)(6<1-B>9)").Add(L"(6<1-<2-<3-U>>>9)");
    bit.Expected.Add(L"=(6A+9)=").Add(L"=(6A+9)==(6A+9)=").Add(L"=(6A+9)=");
    bit.Content.Add(L"(6<1-<2-<3-U>>>9)(6<1-<2-<3-U>>>9)");
    bit.Expected.Add(L"=(6A+9)==(6A+9)=");

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&y1);
    x1->Keyword   = L'(';
    y1->Keyword   = L')';
    x1->Connected = y1;
    y1->Flag      = Flags::BUBBLE;

    Memory<Expression>::AllocateBit(&x2);
    Memory<Expression>::AllocateBit(&y2);
    x2->Keyword   = L'<';
    y2->Keyword   = L'>';
    x2->Connected = y2;
    y2->Flag      = Flags::BUBBLE;
    y2->NestExprs.Add(x2); // Nest itself

    y1->NestExprs.Add(x2); // Nested by x2

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(x2).Add(y1).Add(y2);
    bits.Add(bit);

    y1->ParseCB = ([](String const &block, Match const &item) noexcept->String {
        String nc = L'=';
        nc += block;
        nc += L'=';
        return nc;
    });

    y2->ParseCB = ([](String const &block, Match const &item) noexcept->String {
        if (block == L"<3-U>") {
            return L'A';
        }

        if (block == L"<2-A>") {
            return L'B';
        }

        if (block == L"<1-B>") {
            return L"A+";
        }

        return L"err";
    });
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"[([x]),([y])--]==").Add(L"e[(]xx)] ").Add(L" [(x])] ");
    bit.Expected.Add(L"[]==").Add(L"e[] ").Add(L" [] ");

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&y1);
    x1->Keyword   = L'[';
    y1->Keyword   = L']';
    x1->Connected = y1;
    y1->Flag      = Flags::COMPACT;

    Memory<Expression>::AllocateBit(&x2);
    Memory<Expression>::AllocateBit(&y2);
    x2->Keyword   = L'(';
    y2->Keyword   = L')';
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

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&y1);
    x1->Keyword   = L"[[[[";
    y1->Keyword   = L"]]]]";
    x1->Connected = y1;
    y1->Flag      = Flags::COMPACT;

    Memory<Expression>::AllocateBit(&x2);
    Memory<Expression>::AllocateBit(&y2);
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

    bit.Content.Add(L"(1+2)").Add(L"( 1 + 2 )").Add(L"(1+)").Add(L"(+1)").Add(L"(1+2+3)").Add(L"(4+1+2+9)");
    bit.Expected.Add(L'3').Add(L'3').Add(L'0').Add(L'1').Add(L'6').Add(L"16");
    bit.Content.Add(L"(11+222)").Add(L"( 111 + 22 )").Add(L"(1111+)").Add(L"(+1111)").Add(L"(11+222+3333)");
    bit.Expected.Add(L"233").Add(L"133").Add(L'0').Add(L"1111").Add(L"3566");

    Memory<Expression>::AllocateBit(&x1);
    x1->Keyword = L'+';
    x1->Flag    = Flags::SPLIT | Flags::TRIM;

    Memory<Expression>::AllocateBit(&y1);
    Memory<Expression>::AllocateBit(&y2);
    y1->Keyword = L'(';
    y2->Keyword = L')';
    y2->NestExprs.Add(x1);
    y1->Connected = y2;
    y2->Flag      = Flags::SPLITNEST;

    bit.Exprs.Add(y1);
    bit.Collect.Add(x1).Add(y1).Add(y2);
    bits.Add(bit);

    y2->ParseCB = ([](String const &block, Match const &item) noexcept->String {
        double number = 0.0;

        if (item.NestMatch.Size != 0) {
            String  r      = L"";
            double  temnum = 0.0;
            UNumber i      = 0;

            if (item.NestMatch[i].Length == 0) {
                // Plus sign at the biggening. Thats cool.
                i = 1;
            }

            Match *nm;
            for (; i < item.NestMatch.Size; i++) {
                nm = &(item.NestMatch[i]);
                if ((nm->Length == 0) || !String::ToNumber(block, temnum, nm->Offset, nm->Length)) {
                    return L'0';
                }

                number += temnum;
            }

            return String::FromNumber(number);
        }

        return L'0';
    });
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"2*3+4*5+7*8").Add(L"2*3+4*5").Add(L"1+2*3+4*5+1").Add(L"1+1*1+1");
    bit.Expected.Add(L"82").Add(L"26").Add(L"28").Add(L'3');

    Memory<Expression>::AllocateBit(&x1);
    x1->Keyword = L'+';
    x1->Flag    = Flags::SPLIT | Flags::GROUPSPLIT;

    Memory<Expression>::AllocateBit(&x2);
    x2->Keyword = L'*';
    x2->Flag    = Flags::SPLIT | Flags::GROUPSPLIT;

    x1->NestExprs.Add(x2);
    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(x2);
    bits.Add(bit);

    x1->ParseCB = ([](String const &block, Match const &item) noexcept->String {
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
                return L'0';
            }

            number += temnum;
        }
        return String::FromNumber(number);
    });

    x2->ParseCB = ([](String const &block, Match const &item) noexcept->String {
        double number = 1.0;
        double temnum = 1.0;
        String r      = L"";

        Match *nm;
        for (UNumber i = 0; i < item.NestMatch.Size; i++) {
            nm = &(item.NestMatch[i]);
            if ((nm->Length == 0) || !String::ToNumber(block, temnum, nm->Offset, nm->Length)) {
                return L'0';
            }

            number *= temnum;
        }
        return String::FromNumber(number);
    });
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"(1++2)").Add(L"( 1 ++ 2 )").Add(L"(1++)").Add(L"(++1)").Add(L"(1++2++3)");
    bit.Expected.Add(L'3').Add(L'3').Add(L'0').Add(L'0').Add(L'6');
    bit.Content.Add(L"(11++222)").Add(L"( 111 ++ 22 )").Add(L"(1111++)").Add(L"(++1111)").Add(L"(11++222++3333)");
    bit.Expected.Add(L"233").Add(L"133").Add(L'0').Add(L'0').Add(L"3566");

    Memory<Expression>::AllocateBit(&x1);
    x1->Keyword = L"++";
    x1->Flag    = Flags::SPLIT | Flags::TRIM;

    Memory<Expression>::AllocateBit(&y1);
    Memory<Expression>::AllocateBit(&y2);
    y1->Keyword = L'(';
    y2->Keyword = L')';
    y2->NestExprs.Add(x1);
    y1->Connected = y2;
    y2->Flag      = Flags::SPLITNEST;

    bit.Exprs.Add(y1);
    bit.Collect.Add(x1).Add(y1).Add(y2);
    bits.Add(bit);

    y2->ParseCB = ([](String const &block, Match const &item) noexcept->String {
        double number = 0.0;
        double temnum = 0.0;
        String r      = L"";

        Match *nm;
        for (UNumber i = 0; i < item.NestMatch.Size; i++) {
            nm = &(item.NestMatch[i]);
            if ((nm->Length == 0) || !String::ToNumber(block, temnum, nm->Offset, nm->Length)) {
                return L'0';
            }

            number += temnum;
        }
        return String::FromNumber(number);
    });
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    // Note: Crazy mojo!
    bit.Content.Add(L"yx31xy").Add(L"x+1").Add(L"xx+1").Add(L"xx+1+2").Add(L"1+xx").Add(L"1+yy").Add(L"1+3+yy");
    bit.Expected.Add(L"733137").Add(L'4').Add(L"34").Add(L"36").Add(L"34").Add(L"78").Add(L"81");
    bit.Content.Add(L"yx+1+xy+2+y").Add(L"yx4xy+xx8y+y1yyy");
    bit.Expected.Add(L"120").Add(L"148601");

    Memory<Expression>::AllocateBit(&x1);
    x1->Keyword = L'+';
    x1->Flag    = Flags::SPLIT | Flags::GROUPSPLIT;

    Memory<Expression>::AllocateBit(&x2);
    x2->Keyword = L'x';
    x2->ParseCB = ([](String const &block, Match const &item) noexcept->String {
        //
        return L'3';
    });

    Memory<Expression>::AllocateBit(&x3);
    x3->Keyword = L'y';
    x3->ParseCB = ([](String const &block, Match const &item) noexcept->String {
        //
        return L'7';
    });

    bit.Exprs.Add(x1).Add(x2).Add(x3);
    bit.Collect.Add(x1).Add(x2).Add(x3);
    bits.Add(bit);

    x1->ParseCB = ([](String const &block, Match const &item) noexcept->String {
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
                return L'0';
            }

            number += temnum;
        }
        return String::FromNumber(number);
    });
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"[(<>)]").Add(L"(<>)").Add(L"<>");
    bit.Expected.Add(L"[(u)]").Add(L"(u)").Add(L'u');

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&y1);
    x1->Keyword   = L'[';
    y1->Keyword   = L']';
    x1->Connected = y1;
    x1->Flag      = Flags::POP;
    y1->Flag      = Flags::BUBBLE;
    y1->ParseCB   = ([](String const &block, Match const &item) noexcept->String {
        //
        return block;
    });

    Memory<Expression>::AllocateBit(&x2);
    Memory<Expression>::AllocateBit(&y2);
    x2->Keyword   = L'(';
    y2->Keyword   = L')';
    x2->Flag      = Flags::POP;
    y2->Flag      = Flags::BUBBLE;
    x2->Connected = y2;
    y2->ParseCB   = ([](String const &block, Match const &item) noexcept->String {
        //
        return block;
    });

    Memory<Expression>::AllocateBit(&x3);
    Memory<Expression>::AllocateBit(&y3);
    x3->Keyword   = L'<';
    y3->Keyword   = L'>';
    x3->Connected = y3;
    y3->Replace   = L'u';

    x1->NestExprs.Add(x2);
    y1->NestExprs.Add(x2);
    x2->NestExprs.Add(x3);
    y2->NestExprs.Add(x3);

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(x2).Add(x3).Add(y1).Add(y2).Add(y3);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"{AAxx     yyyBBxx  yyyCC}");
    bit.Expected.Add(L"AABBCC");

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&y1);
    x1->Keyword   = L'{';
    y1->Keyword   = L'}';
    y1->Flag      = Flags::SPLITNEST;
    x1->Connected = y1;

    Memory<Expression>::AllocateBit(&x2);
    Memory<Expression>::AllocateBit(&y2);
    x2->Keyword   = L"xx";
    y2->Keyword   = L"yyy";
    y2->Flag      = Flags::SPLIT;
    x2->Connected = y2;

    y1->NestExprs.Add(x2);
    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(x2).Add(y1).Add(y2);
    bits.Add(bit);

    y1->ParseCB = ([](String const &block, Match const &item) noexcept->String {
        String nc = L"";

        for (UNumber i = 0; i < item.NestMatch.Size; i++) {
            nc += String::Part(block, item.NestMatch[i].Offset, item.NestMatch[i].Length);
        }

        return nc;
    });
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"ff11-22ss33-44ttt55-777e");
    bit.Expected.Add(L"HHHH");

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&x3);
    Memory<Expression>::AllocateBit(&x2);
    Memory<Expression>::AllocateBit(&y1);
    Memory<Expression>::AllocateBit(&y2);

    x1->Keyword = L"ff";
    x2->Keyword = L"ss";
    x3->Keyword = L"ttt";
    y2->Keyword = L'e';
    y1->Keyword = L'-';

    x1->Connected = x2;
    x2->Connected = x3;
    x3->Connected = y2;

    y1->Flag = Flags::SPLIT;
    y2->Flag = Flags::SPLITNEST;
    y2->NestExprs.Add(y1);

    y2->ParseCB = ([](String const &block, Match const &item) noexcept->String {
        String s = L"";

        for (UNumber i = 0; i < item.NestMatch.Size; i++) {
            s += L'H';
        }

        return s;
    });

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(x2).Add(x3).Add(y1).Add(y2);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"< zz (  2  ) hh >");
    bit.Expected.Add(L"xxx");

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&x3);
    Memory<Expression>::AllocateBit(&x2);
    Memory<Expression>::AllocateBit(&y1);
    Memory<Expression>::AllocateBit(&y3);

    x1->Keyword = L'<';
    x2->Keyword = L'>';
    x3->Keyword = L'(';
    y3->Keyword = L')';

    x2->Replace = L"xxx";

    x1->Connected = x3;
    x3->Connected = y3;
    y3->Connected = x2;

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(x2).Add(x3).Add(y1).Add(y3);
    bits.Add(bit);
    ///////////////////////////////////////////
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

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&y1);
    x1->Keyword   = L"{-";
    y1->Keyword   = L"-}";
    y1->Flag      = Flags::SPLITNEST;
    x1->Connected = y1;

    Memory<Expression>::AllocateBit(&x2);
    x2->Flag    = Flags::SPLIT;
    x2->Keyword = L'#';

    y1->ParseCB = &(FlipSplit);

    y1->NestExprs.Add(x1).Add(x2);
    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(x2).Add(y1);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<br><s><br>");
    bit.Expected.Add(L"TR<wr>");

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&y1);
    Memory<Expression>::AllocateBit(&x2);
    Memory<Expression>::AllocateBit(&y2);
    Memory<Expression>::AllocateBit(&x3);

    x1->Keyword = L'<';
    y1->Keyword = L'>';
    x2->Keyword = L'<';
    y2->Keyword = L'>';
    y2->Replace = L"TR";

    x3->Keyword = L"br";
    x3->Replace = L"wr";

    x1->Connected = y1;
    y1->Connected = x2;
    x2->Connected = y2;

    bit.Exprs.Add(x1).Add(x3);
    bit.Collect.Add(x1).Add(x2).Add(x3).Add(y1).Add(y2);
    bits.Add(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"{}")
        .Add(L"{  }")
        .Add(L"{ x }")
        .Add(L"{{ {  { x }}  }}")
        .Add(L"  &&&")
        .Add(L"  a & b & c & & d  &");
    bit.Expected.Add(L"{}").Add(L"{  }").Add(L'x').Add(L'x').Add(L"  &&&").Add(L"abcd");

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&y1);
    Memory<Expression>::AllocateBit(&x2);

    x1->Keyword   = L'{';
    y1->Keyword   = L'}';
    x1->Connected = y1;
    y1->Flag      = Flags::BUBBLE | Flags::TRIM | Flags::DROPEMPTY;
    y1->NestExprs.Add(x1);

    x2->Keyword = L'&';
    x2->Flag    = Flags::SPLIT | Flags::GROUPSPLIT | Flags::TRIM | Flags::DROPEMPTY;

    bit.Exprs.Add(x1).Add(x2);
    bit.Collect.Add(x1).Add(x2).Add(y1);
    bits.Add(bit);

    y1->ParseCB = ([](String const &block, Match const &item) noexcept->String {
        if (block.Length > (item.OLength + item.CLength)) {
            return String::Part(block, item.OLength, block.Length - (item.OLength + item.CLength));
        }

        return L'0';
    });

    x2->ParseCB = ([](String const &block, Match const &item) noexcept->String {
        String nc = L"";

        for (UNumber i = 0; i < item.NestMatch.Size; i++) {
            nc += String::Part(block, item.NestMatch[i].Offset, item.NestMatch[i].Length);
        }

        return nc;
    });
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add(L"<div>ae<div>str0</div></diva>");
    bit.Expected.Add(L"<div>ae(str0)</diva>");

    Memory<Expression>::AllocateBit(&x1);
    Memory<Expression>::AllocateBit(&y1);

    y1->Flag = Flags::BUBBLE;

    x1->Keyword = L"<div>";
    y1->Keyword = L"</div>";

    x1->Connected = y1;
    y1->NestExprs.Add(x1);

    bit.Exprs.Add(x1);
    bit.Collect.Add(x1).Add(y1);
    bits.Add(bit);

    y1->ParseCB = ([](String const &block, Match const &item) noexcept->String {
        String nc = L'(';
        nc += String::Part(block, item.OLength, block.Length - (item.OLength + item.CLength));
        nc += L')';
        return nc;
    });
    ///////////////////////////////////////////
    return bits;
}

static String FlipSplit(String const &block, Match const &item) noexcept {
    String nc = L"";

    if (item.NestMatch.Size != 0) {
        for (UNumber i = 0; i < item.NestMatch.Size; i++) {
            nc = Test::FlipSplit(block, item.NestMatch[i]) + nc;
        }

        if (item.NestMatch.Size > 1) {
            nc = String(L'[') + nc + L']';
        }
    } else {
        nc = String::Part(block, item.Offset, item.Length);
    }

    return nc;
}

} // namespace Test
} // namespace Qentem

#endif