/**
 * Qentem Test
 *
 * @brief     For testing Qentem Engine
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include <Extension/Template.hpp>

#ifndef TESTENGINE_H
#define TESTENGINE_H

namespace Qentem {
namespace Test {

using Qentem::Engine::Expression;
using Qentem::Engine::Expressions;
using Qentem::Engine::Flags;
using Qentem::Engine::MatchBit;

struct TestBit {
    UNumber             Line = 0;
    Expressions         Expres;
    Expressions         Collect;
    Array<const char *> Content;
    Array<const char *> Expected;
};

/////////////////////////////////////////////
static String FlipSplit(const char *block, const MatchBit &item, UNumber length, void *ptr) noexcept;

static void CleanBits(Array<TestBit> &bits) noexcept {
    for (UNumber i = 0; i < bits.Size; i++) {
        for (UNumber j = 0; j < bits[i].Collect.Size; j++) {
            Qentem::Memory::DeallocateBit<Expression>(&bits[i].Collect[j]);
        }
    }
}

static String Replace(const char *content, UNumber length, const char *_find, const char *_replace) {
    static Expression find_key;

    find_key.SetHead(_find);
    find_key.SetReplace(_replace);

    return Engine::Parse(Engine::Match(Expressions().Add(&find_key), content, 0, length), content, 0, length);
}

static String ReplaceNewLine(const char *content, UNumber length, const char *_replace) {

    static Expressions find_keys;
    static Expression  find_key1;
    static Expression  find_key2;
    static Expression  find_key3;
    static Expression  find_key4;

    find_key1.SetReplace(_replace);
    find_key2.SetReplace(_replace);
    find_key3.SetReplace(_replace);
    find_key4.SetReplace(_replace);

    if (find_keys.Size == 0) { // Caching
        find_key1.SetHead("\n");
        find_key2.SetHead("\r");
        find_key3.SetHead("\t");
        find_key4.SetHead("    ");
        find_keys.Add(&find_key1).Add(&find_key2).Add(&find_key3).Add(&find_key4);
    }

    return Engine::Parse(Engine::Match(find_keys, content, 0, length), content, 0, length);
}

static Array<String> Extract(const Array<MatchBit> &items, const char *content) noexcept {
    Array<String> matches(items.Size);

    for (UNumber i = 0; i < items.Size; i++) {
        matches.Add(String::Part(content, items[i].Offset, items[i].Length) + " -> O:" + String::FromNumber(items[i].Offset) +
                    " L:" + String::FromNumber(items[i].Length));
    }

    return matches;
}

static String DumpMatches(const Array<MatchBit> &matches, const char *content, const String &offset, UNumber index = 0) noexcept {
    if (matches.Size == 0) {
        return offset + "No matches!\n";
    }
    StringStream ss;
    String       innoffset = "    ";

    Array<String> items = Extract(matches, content);

    ss += offset + "(" + String::FromNumber(static_cast<double>(matches.Size)) + ") => [\n";

    for (UNumber i = index; i < matches.Size; i++) {
        ss += innoffset + offset + "[" + String::FromNumber(static_cast<double>(i)) + "]: " + items[i] + "\n";

        if (matches[i].NestMatch.Size != 0) {
            ss += (innoffset + offset + "-NestMatch:\n");
            ss += DumpMatches(matches[i].NestMatch, content, (innoffset + innoffset + offset), 0);
        }
    }

    ss += offset + "]\n";
    return ss.ToString();
}

static Array<TestBit> GetALEBits() noexcept {
    Array<TestBit> bits;
    TestBit        bit;

    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("  1  ").Add("+1").Add(" +1 ").Add("1+").Add(" 1+ ").Add("1+ ").Add(" 1+").Add("1+1").Add(" 1 + 1 ");
    bit.Expected.Add("1").Add("1").Add("1").Add("1").Add("1").Add("1").Add("1").Add("2").Add("2");

    bit.Content.Add(" -1 ").Add("-1").Add("1-").Add(" 1- ").Add("1- ").Add(" 1-").Add("1-1").Add(" 1 - 1 ");
    bit.Expected.Add("-1").Add("-1").Add("1").Add("1").Add("1").Add("1").Add("0").Add("0");

    bit.Content.Add("1-").Add("--1").Add("1--").Add("1--1").Add("1---1");
    bit.Expected.Add("1").Add("1").Add("1").Add("2").Add("0");

    bit.Content.Add("-1--1");
    bit.Expected.Add("0");

    bit.Content.Add("+1+1+2+11").Add("1+1+2+11+").Add("1-1-2-11").Add("-1-1-2-11");
    bit.Expected.Add("15").Add("15").Add("-13").Add("-15");

    bit.Content.Add("+1-2+3").Add("+1+2-3").Add("-1-2+3-").Add("-1+2-3-");
    bit.Expected.Add("2").Add("0").Add("0").Add("-2");

    ////
    bit.Content.Add("*1").Add(" 1*1 ").Add("3*5").Add(" 5*3**1*2 ").Add(" 5*3*1*2 ");
    bit.Expected.Add("0").Add("1").Add("15").Add("0").Add("30");

    bit.Content.Add("/1").Add(" 1/1 ").Add("100/5").Add(" 5/3//1/2 ").Add(" 32/2/2/2/2/2 ");
    bit.Expected.Add("0").Add("1").Add("20").Add("0").Add("1");

    ////
    bit.Content.Add("2*1+1*2").Add("1+1*2").Add("5^5");
    bit.Expected.Add("4").Add("3").Add("3125");

    ////
    bit.Content.Add("4^0").Add("8^(-2)").Add("8^2").Add("8^1");
    bit.Expected.Add("1").Add("0.016").Add("64").Add("8");

    ////
    bit.Content.Add("2=2").Add("2==2").Add("1=2").Add("2==1").Add("2==1+1").Add("2/2==1");
    bit.Expected.Add("1").Add("1").Add("0").Add("0").Add("1").Add("1");

    bit.Content.Add("2!=2").Add("1!=2").Add("1<=2").Add("2>=2").Add("2<=2").Add("2<=1").Add("1>=2").Add("2>=1");
    bit.Expected.Add("0").Add("1").Add("1").Add("1").Add("1").Add("0").Add("0").Add("1");

    bit.Content.Add("3 + 9 - 1 - -1 + 2 == 14");
    bit.Expected.Add("1");

    ////
    bit.Content.Add("(   5   )").Add("  2 * (1+ 1  )  ").Add(" (  1  +1)  +  5 ").Add("  2 * (1+1) - 1 ");
    bit.Expected.Add("5").Add("4").Add("7").Add("3");

    bit.Content.Add("(5)+(6)").Add("  (  5   )  +  (  6  )  ").Add(" ( 5 ) + ( 6 ) + ( 6 ) ").Add(" ( 5 ) * ( 6 ) + ( 6 ) + (1)");
    bit.Expected.Add("11").Add("11").Add("17").Add("37");

    bit.Content.Add("2*(5)+(6)+1").Add("2*(5)+2*2+(6)+1");
    bit.Expected.Add("17").Add("21");

    bit.Content.Add(" (1+   1  ) / 2 + ((  2  *  4   ) - 1) / (2)  ");
    bit.Expected.Add("4.5");

    bit.Content.Add("(2*(1+1))").Add("((1+10)+(5*5))");
    bit.Expected.Add("4").Add("36");

    bit.Content.Add("(((2* (1 * 3)) + 1 - 4) + (((10 - 5) - 6 + ((1 + 1) + (1 + 1))) * (8 / 4 + 1)) - (1) + (1) + 2 = 14)");
    bit.Expected.Add("1");

    ////
    bit.Expres = Qentem::ALE::getMathExpres();
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    return bits;
}

static Array<TestBit> GetTemplateBits(Document &data) noexcept {
    Array<TestBit> bits;
    TestBit        bit;

    data["foo"]  = "FOO";
    data["r1"]   = "Familly";
    data["e1"]   = "";
    data["e2"]   = " ";
    data["m"]    = "  ((5^2) * 2) + 13 ";
    data["abc1"] = Array<String>().Add("a").Add("b").Add("c");

    data["lvl2"] = Document();

    data["lvl2"]["r1"] = "l2";

    data["lvl2"]["r1"] = "l2";
    data["lvl2"]["e1"] = "";
    data["lvl2"]["e2"] = " ";
    data["lvl2"]["e3"] = 5;

    data["lvl2"]["numbers"] = Array<double>().Add(1);

    Document n2;
    n2["the_rest"] = Array<double>().Add(2);
    data["lvl2"]["numbers"] += n2["the_rest"]; // Coping

    Document strings = R"({"strings": ["N1"]})";
    Document n3      = Array<String>().Add("N2").Add("N3");

    data["lvl2"] += strings;
    data["lvl2"]["strings"] += static_cast<Document &&>(n3); // Moving

    data.Rehash(11, true);

    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("{v:r1}  ").Add(" {v:r2}  ").Add("{v:e1} ").Add("{v:e2}");
    bit.Expected.Add("Familly  ").Add(" r2  ").Add(" ").Add(" ");

    bit.Content.Add("{v:lvl2[e3]}");
    bit.Expected.Add("5");

    bit.Content.Add("{v:lvl2[r1]}").Add("{v:lvl2[r2]}").Add("{v:lvl2[e1]}").Add(" {v:lvl2[e2]} ");
    bit.Expected.Add("l2").Add("lvl2[r2]").Add("").Add("   ");

    bit.Content.Add("{v:lvl2[numbers][0]}").Add("{v:lvl2[numbers][1]}").Add("{v:lvl2[numbers][3]}");
    bit.Expected.Add("1").Add("2").Add("lvl2[numbers][3]");

    bit.Content.Add("{v:lvl2[strings][0]}").Add("{v:lvl2[strings][1]}").Add("{v:lvl2[strings][3]}");
    bit.Expected.Add("N1").Add("N2").Add("lvl2[strings][3]");

    ////

    bit.Content.Add("{math:  ((5^2) * 2) + 3  }");
    bit.Expected.Add("53");
    bit.Content.Add("{math: {v:m}  }");
    bit.Expected.Add("63");
    ////

    bit.Content.Add(R"({iif case="987" true ="5"})");
    bit.Expected.Add("5");

    bit.Content.Add(R"({iif case="1" false="10"})");
    bit.Expected.Add("");

    bit.Content.Add(R"({iif case = "1" true = "1" false = "0"})");
    bit.Expected.Add("1");

    bit.Content.Add(R"({iif case="0" true="1" false="0"})");
    bit.Expected.Add("0");

    bit.Content.Add(
        R"({iif case="((2* (1 * 3)) + 1 - 4) + (((10 - 5) - 6 + ((1 + 1) + (1 + 1))) * (8 / 4 + 1)) - (1) + (1) + 2" true ="14"})");
    bit.Expected.Add("14");

    bit.Content.Add(R"({iif case="0" true="1"})");
    bit.Expected.Add("");

    bit.Content.Add(R"({iif case="{v:lvl2[numbers][1]} = 2" true="it's true! " false ="it's false"})");
    bit.Expected.Add("it's true! ");

    bit.Content.Add(R"({iif case="      0    " true="5"})");
    bit.Expected.Add("");

    bit.Content.Add(R"({iif case="      1    " false="35.5"})");
    bit.Expected.Add("");

    bit.Content.Add(R"({iif case = "5<1" true = "1" false = "0"})");
    bit.Expected.Add("0");

    bit.Content.Add(R"({iif case="1<2" false="No"})");
    bit.Expected.Add("");

    bit.Content.Add(R"({iif case="5<10" true="1" false="0"})");
    bit.Expected.Add("1");

    bit.Content.Add(R"({iif case="0&&3" true="1" false="0"})");
    bit.Expected.Add("0");

    bit.Content.Add(R"({iif case="5&&10" true="1" false="0"})");
    bit.Expected.Add("1");

    bit.Content.Add(R"({iif case="7 && 0" true="1" false="0"})");
    bit.Expected.Add("0");

    bit.Content.Add(R"({iif case="0||10" true="1" false="0"})");
    bit.Expected.Add("1");

    bit.Content.Add(R"({iif case="10  || 0" true="1" false="0"})");
    bit.Expected.Add("1");

    bit.Content.Add(R"({iif case="0||0" true="1" false="0"})");
    bit.Expected.Add("0");

    // Text compareing
    bit.Content.Add(R"({iif case="  {v:foo} == FOO  " true ="It's {v:foo}." false="Not {v:foo}!"})");
    bit.Expected.Add("It's FOO.");

    ////
    bit.Content.Add(R"(<loop set="abc1" value="v">v</loop>)");
    bit.Expected.Add("abc");

    bit.Content.Add(R"(<loop set ="lvl2[numbers]" value ="val" key="id">l-id): {v:lvl2[numbers][id]}# </loop>)");
    bit.Expected.Add("l-0): 1# l-1): 2# ");

    bit.Content.Add(
        R"(<loop set="lvl2[numbers]" value ="val" key="id"><loop set ="lvl2[numbers]" value ="val2" key  ="id2">l-id-id2):val2# </loop></loop>)");
    bit.Expected.Add("l-0-0):1# l-0-1):2# l-1-0):1# l-1-1):2# ");

    bit.Content.Add(R"(<loop set="lvl2[strings]" value="val" key="id">l-id): val#</loop>)");
    bit.Expected.Add("l-0): N1#l-1): N2#l-2): N3#");

    bit.Content.Add(
        R"(Space <loop set="lvl2[strings]" value="val" key="id">l-id): val#</loop><loop set ="lvl2[numbers]" value="val2" key="id2">l-id2): val2# </loop>)");
    bit.Expected.Add("Space l-0): N1#l-1): N2#l-2): N3#l-0): 1# l-1): 2# ");

    ////

    bit.Content.Add(R"( <if case="1"> 5 </if> )")
        .Add(R"(<if case="0">5</if> )")
        .Add(R"( <if case="100"><if case="1">6</if></if>)")
        .Add(R"(<if case="8"><if case="88"><if case="888">7</if></if></if>)")
        .Add(R"(<if case="1"><if case="1"><if case="1"><if case="1">8</if></if></if></if>)");
    bit.Expected.Add("  5  ").Add(" ").Add(" 6").Add("7").Add("8");

    bit.Content.Add(R"(<if case="1">4<else /> 6 </if>)")
        .Add(R"(<if case="0"> 4 <else />6</if>)")
        .Add(R"(<if case="0"><span> 1 <else /><if case="0"> <span> 2 <else /><if case="0"> <span> 3 <else />7</if></if></if>)");
    bit.Expected.Add("4").Add("6").Add("7");

    bit.Content.Add(R"(<if case="0"><else />91</if>)");
    bit.Expected.Add("91");

    bit.Content.Add(R"(<if case="0">4<elseif case="{v:lvl2[numbers][0]} = 1" /> 6 </if>)");
    bit.Expected.Add(" 6 ");

    bit.Content.Add(R"(<if case="1">4<elseif case="{v:lvl2[numbers][0]} = 1" /> 6 </if>)");
    bit.Expected.Add("4");

    bit.Content.Add(R"(<if case="0"><span> 4<elseif case="0" /><span> 6 <elseif case="1" />9</if>)");
    bit.Expected.Add("9");

    bit.Content.Add(R"(<if case="0">4<elseif case="0" /> 6 <else />91</if>)");
    bit.Expected.Add("91");

    ////

    bit.Expres = Qentem::Template::getExpres();
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////

    return bits;
}

static Array<TestBit> GetEngineBits() noexcept {
    Array<TestBit> bits;
    TestBit        bit;

    Expression *x1;
    Expression *x2;
    Expression *x3;
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;
    bit.Content.Add("-").Add(" -- ").Add("- - - -");
    bit.Expected.Add("*").Add(" ** ").Add("* * * *");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("-");
    x1->SetReplace("*");

    bit.Expres.Add(x1);
    bit.Collect.Add(x1);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;
    bit.Content.Add("----");
    bit.Expected.Add("****");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("-");
    x1->ParseCB = ([](const char *block, const MatchBit &item, const UNumber length, void *ptr) noexcept -> String { return "*"; });

    Memory::AllocateBit<Expression>(&x2);
    x2->SetHead("--");

    bit.Expres.Add(x1).Add(x2);
    bit.Collect.Add(x1).Add(x2);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;
    bit.Content.Add(" - ").Add(" -- ").Add(" {{{9}} ");
    bit.Expected.Add(" 1-1 ").Add(" 1-2 ").Add(" 1-6 ");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("-");

    Memory::AllocateBit<Expression>(&x2);
    x2->SetHead("--");

    Memory::AllocateBit<Expression>(&x3);
    x3->SetHead("{{{");
    x3->SetTail("}}");

    x1->ParseCB = x2->ParseCB = x3->ParseCB =
        ([](const char *block, const MatchBit &item, const UNumber length, void *ptr) noexcept -> String {
            return String::FromNumber(item.Offset) + "-" + String::FromNumber(item.Length);
        });

    bit.Expres.Add(x2).Add(x1).Add(x3);
    bit.Collect.Add(x1).Add(x2).Add(x3);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("<").Add("<>").Add("<> ").Add(" <>").Add(" <> ").Add("  <>  ");
    bit.Expected.Add("<").Add("-").Add("- ").Add(" -").Add(" - ").Add("  -  ");
    bit.Content.Add("<a").Add("a<aa").Add("<aa> a").Add("a <a>").Add("a <a> a").Add("a  <aa>  a");
    bit.Expected.Add("<a").Add("a<aa").Add("- a").Add("a -").Add("a - a").Add("a  -  a");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("<");
    x1->SetTail(">");
    x1->SetReplace("-");

    bit.Expres.Add(x1);
    bit.Collect.Add(x1);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("<<").Add("<<>").Add("<<> ").Add(" <<>").Add(" <<> ").Add("  <<>  ");
    bit.Expected.Add("<<").Add("-").Add("- ").Add(" -").Add(" - ").Add("  -  ");
    bit.Content.Add("<<a").Add("a<<aa").Add("<<aa> a").Add("a <<a>").Add("a <<a> a").Add("a  <<aa>  a");
    bit.Expected.Add("<<a").Add("a<<aa").Add("- a").Add("a -").Add("a - a").Add("a  -  a");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("<<");
    x1->SetTail(">");
    x1->SetReplace("-");

    bit.Expres.Add(x1);
    bit.Collect.Add(x1);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("<").Add("<>>").Add("<>> ").Add(" <>>").Add(" <>> ").Add("  <>>  ");
    bit.Expected.Add("<").Add("-").Add("- ").Add(" -").Add(" - ").Add("  -  ");
    bit.Content.Add("<a").Add("a<aa").Add("<aa>> a").Add("a <a>>").Add("a <a>> a").Add("a {8} <aa>>  a");
    bit.Expected.Add("<a").Add("a<aa").Add("- a").Add("a -").Add("a - a").Add("a {8} -  a");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("<");
    x1->SetTail(">>");
    x1->SetReplace("-");

    Memory::AllocateBit<Expression>(&x2);
    x2->SetHead("{");
    x2->SetTail("}");
    x2->Flag = Flags::IGNORE;

    bit.Expres.Add(x1).Add(x2);
    bit.Collect.Add(x1).Add(x2);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("<<").Add("<<>>").Add("<<>> ").Add(" <<>>").Add(" <<>> ").Add("  <<>>  ");
    bit.Expected.Add("<<").Add("-").Add("- ").Add(" -").Add(" - ").Add("  -  ");
    bit.Content.Add("<<a").Add("a<<aa").Add("<<aa>> a").Add("a <<a>>").Add("a <<a>> a").Add("a  <<aa>>  a");
    bit.Expected.Add("<<a").Add("a<<aa").Add("- a").Add("a -").Add("a - a").Add("a  -  a");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("<<");
    x1->SetTail(">>");
    x1->SetReplace("-");

    bit.Expres.Add(x1);
    bit.Collect.Add(x1);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("<").Add("->-").Add("<o<>").Add("<>o>").Add("<><>o>").Add("<><><>");
    bit.Expected.Add("<").Add("->-").Add("=").Add("=o>").Add("==o>").Add("===");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("<");
    x1->SetTail(">");
    x1->SetReplace("=");

    bit.Expres.Add(x1);
    bit.Collect.Add(x1);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("<>").Add("<><>").Add("<><><>");
    bit.Expected.Add("=").Add("=<>").Add("=<><>");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("<");
    x1->SetTail(">");
    x1->SetReplace("=");
    x1->Flag = Flags::ONCE;

    bit.Expres.Add(x1);
    bit.Collect.Add(x1);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("<<> ").Add("<<<> ").Add("<>> ").Add(" <>>> ");
    bit.Content.Add("<<>").Add("<<<>").Add("<>>").Add(" <>>>");
    bit.Content.Add("<>").Add("<o<> ").Add(" < < >>").Add("<0><<<1-0-0><1-0-1><1-0-2>><1-1>><2>");
    bit.Content.Add("<> ").Add("<o< > ").Add(" < < > > ");

    bit.Expected.Add("<= ").Add("<<= ").Add("=> ").Add(" =>> ");
    bit.Expected.Add("<=").Add("<<=").Add("=>").Add(" =>>");
    bit.Expected.Add("=").Add("<o= ").Add(" =").Add("===");
    bit.Expected.Add("= ").Add("<o= ").Add(" = ");

    bit.Content.Add("<0><1><<2-0><<2-1-0><2-1-1><<2-1-2-0><2-1-2-1><2-1-2-2><2-1-2-3>>><2-2><2-3>><3> ><");
    bit.Expected.Add("==== ><");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("<");
    x1->SetTail(">");
    x1->SetReplace("=");
    x1->NestExpres.Add(x1);

    bit.Expres.Add(x1);
    bit.Collect.Add(x1);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("<0<X>").Add("<0<X> ").Add("<0<<X>");
    bit.Expected.Add("W").Add("W ").Add("W");

    bit.Content.Add("<0<xxxX>").Add("<0-<0-X>").Add("<0-<0X>").Add("<0<<0X>").Add("<0>X>").Add("<0X>X>X>");
    bit.Expected.Add("W").Add("<0-W").Add("<0-W").Add("<0<W").Add("W").Add("WX>X>");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("<0");
    x1->SetTail("X>");
    x1->SetReplace("W");
    x1->NestExpres.Add(x1);

    bit.Expres.Add(x1);
    bit.Collect.Add(x1);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;
    bit.Content.Add("<0<0X> ").Add("<0<088888X> ").Add("<0<<0X> ").Add("<0>X> ").Add(" <0X>X>X> ");
    bit.Expected.Add("<0W ").Add("<0W ").Add("<0<W ").Add("W ").Add(" WX>X> ");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("<0");
    x1->SetTail("X>");
    x1->SetReplace("W");
    x1->NestExpres.Add(x1);

    bit.Expres.Add(x1);
    bit.Collect.Add(x1);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("<<<> ");
    bit.Content.Add("<<>>").Add(" <<>> ").Add("<<> ").Add("<<o<<> ");
    bit.Content.Add("<< << >>").Add("<<0><<<<<<1-0-0><<1-0-1><<1-0-2>><<1-1>><<2>");
    bit.Expected.Add("+ ").Add("+>").Add(" +> ").Add("+ ").Add("<<o+ ").Add("+").Add("+++");
    bit.Content.Add("<<0><<1><<<<2-0><<<<2-1-0><<2-1-1><<<<2-1-2-0><<2-1-2-1><<2-1-2-2><<2-1-2-3>>><<2-2><<2-3>><<3> ><<");
    bit.Expected.Add("++++ ><<");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("<<");
    x1->SetTail(">");
    x1->SetReplace("+");
    x1->NestExpres.Add(x1);

    bit.Expres.Add(x1);
    bit.Collect.Add(x1);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("<o<>>").Add("<>>").Add("< < >>>>").Add("<0>><<<1-0-0>><1-0-1>><1-0-2>>>><1-1>>>><2>>");
    bit.Expected.Add("<o_").Add("_").Add("_").Add("___");
    bit.Content.Add("<0>><1>><<2-0>><<2-1-0>><2-1-1>><<2-1-2-0>><2-1-2-1>><2-1-2-2>><2-1-2-3>>>>>><2-2>><2-3>>>><3>> >><");
    bit.Expected.Add("____ >><");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("<");
    x1->SetTail(">>");
    x1->SetReplace("_");
    x1->NestExpres.Add(x1);

    bit.Expres.Add(x1);
    bit.Collect.Add(x1);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("<<o<<>>").Add("<<>>").Add("<< << >>>>").Add("<<0>><<<<<<1-0-0>><<1-0-1>><<1-0-2>>>><<1-1>>>><<2>>");
    bit.Expected.Add("<<o_").Add("_").Add("_").Add("___");
    bit.Content.Add("<<0>><<1>><<<<2-0>><<<<2-1-0>><<2-1-1>><<<<2-1-2-0>><<2-1-2-1>><<2-1-2-2>><<2-1-2-3>>>>>><<2-2>><<2-3>>>><<3>> >><<");
    bit.Expected.Add("____ >><<");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("<<");
    x1->SetTail(">>");
    x1->SetReplace("_");
    x1->NestExpres.Add(x1);

    bit.Expres.Add(x1);
    bit.Collect.Add(x1);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("<o<>").Add("<>").Add("< < >>").Add("<0><<<1-0-0><1-0-1><1-0-2>><1-1>><2>");
    bit.Expected.Add("<o()").Add("()").Add("( ( ))").Add("(0)(((1-0-0)(1-0-1)(1-0-2))(1-1))(2)");
    bit.Content.Add("<0><1><<2-0><<2-1-0><2-1-1><<2-1-2-0><2-1-2-1><2-1-2-2><2-1-2-3>>><2-2><2-3>><3> ><");
    bit.Expected.Add("(0)(1)((2-0)((2-1-0)(2-1-1)((2-1-2-0)(2-1-2-1)(2-1-2-2)(2-1-2-3)))(2-2)(2-3))(3) ><");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("<");
    x1->SetTail(">");
    x1->Flag = Flags::BUBBLE;
    x1->NestExpres.Add(x1);
    x1->ParseCB = ([](const char *block, const MatchBit &item, const UNumber length, void *ptr) noexcept -> String {
        String nc = "(";
        nc += String::Part(block, 1, (length - 2));
        nc += ")";
        return nc;
    });

    bit.Expres.Add(x1);
    bit.Collect.Add(x1);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("(6<1-B>9)").Add("(6<1-B>9)(6<1-B>9)").Add("(6<1-<2-<3-U>>>9)");
    bit.Expected.Add("=(6A+9)=").Add("=(6A+9)==(6A+9)=").Add("=(6A+9)=");
    bit.Content.Add("(6<1-<2-<3-U>>>9)(6<1-<2-<3-U>>>9)");
    bit.Expected.Add("=(6A+9)==(6A+9)=");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("(");
    x1->SetTail(")");
    x1->Flag    = Flags::BUBBLE;
    x1->ParseCB = ([](const char *block, const MatchBit &item, const UNumber length, void *ptr) noexcept -> String {
        String nc = "=";
        nc += block;
        nc += "=";
        return nc;
    });

    Memory::AllocateBit<Expression>(&x2);
    x2->SetHead("<");
    x2->SetTail(">");
    x2->Flag = Flags::BUBBLE;
    x2->NestExpres.Add(x2); // Nest itself
    x2->ParseCB = ([](const char *block, const MatchBit &item, const UNumber length, void *ptr) noexcept -> String {
        if (String::Compare(block, "<3-U>")) {
            return "A";
        }

        if (String::Compare(block, "<2-A>")) {
            return "B";
        }

        if (String::Compare(block, "<1-B>")) {
            return "A+";
        }

        return "err";
    });

    x1->NestExpres.Add(x2); // Nested by x2

    bit.Expres.Add(x1);
    bit.Collect.Add(x1).Add(x2);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("[(<>)]").Add("(<>)").Add("<>");
    bit.Expected.Add("[(u)]").Add("(u)").Add("u");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("[");
    x1->SetTail("]");
    x1->Flag    = Flags::POP | Flags::BUBBLE;
    x1->ParseCB = ([](const char *block, const MatchBit &item, const UNumber length, void *ptr) noexcept -> String { return block; });

    Memory::AllocateBit<Expression>(&x2);
    x2->SetHead("(");
    x2->SetTail(")");
    x2->Flag    = Flags::POP | Flags::BUBBLE;
    x2->ParseCB = ([](const char *block, const MatchBit &item, const UNumber length, void *ptr) noexcept -> String { return block; });

    Memory::AllocateBit<Expression>(&x3);
    x3->SetHead("<");
    x3->SetTail(">");
    x3->SetReplace("u");

    x1->NestExpres.Add(x2);
    x2->NestExpres.Add(x3);

    bit.Expres.Add(x1);
    bit.Collect.Add(x1).Add(x2).Add(x3);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("<><><>").Add("111<>222").Add("1111<>2222<>3333").Add("1111<>2222<>3333<>4444");
    bit.Expected.Add("0<>2<>4<>").Add("3<>8").Add("4<>10<>16").Add("4<>10<>16<>22");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("<>");
    x1->Flag    = Flags::SPLIT;
    x1->ParseCB = ([](const char *block, const MatchBit &item, const UNumber length, void *ptr) noexcept -> String {
        return String::FromNumber(item.Offset + item.Length);
    });

    bit.Expres.Add(x1);
    bit.Collect.Add(x1);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("(1+2)").Add("( 1 + 2 )").Add("(1+)").Add("(+1)").Add("(1+2+3)").Add("(4+1+2+9)");
    bit.Expected.Add("3").Add("3").Add("0").Add("1").Add("6").Add("16");
    bit.Content.Add("(11+222)").Add("( 111 + 22 )").Add("(1111+)").Add("(+1111)").Add("(11+222+3333)");
    bit.Expected.Add("233").Add("133").Add("0").Add("1111").Add("3566");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("+");
    x1->Flag = Flags::SPLIT | Flags::TRIM;

    Memory::AllocateBit<Expression>(&x2);
    x2->SetHead("(");
    x2->SetTail(")");
    x2->NestExpres.Add(x1);
    x2->ParseCB = ([](const char *block, const MatchBit &item, const UNumber length, void *ptr) noexcept -> String {
        double number = 0.0;

        if (item.NestMatch.Size != 0) {
            double  temnum = 0.0;
            UNumber i      = 0;

            if (item.NestMatch[i].Length == 0) {
                // Plus sign at the biggening. Thats cool.
                i = 1;
            }

            MatchBit *nm;
            for (; i < item.NestMatch.Size; i++) {
                nm = &(item.NestMatch[i]);
                if ((nm->Length == 0) || !String::ToNumber(temnum, block, nm->Offset, nm->Length)) {
                    return "0";
                }

                number += temnum;
            }

            return String::FromNumber(number);
        }

        return "0";
    });

    bit.Expres.Add(x2);
    bit.Collect.Add(x1).Add(x2);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("2*3+4*5+7*8").Add("2*3+4*5").Add("1+2*3+4*5+1").Add("1+1*1+1");
    bit.Expected.Add("82").Add("26").Add("28").Add("3");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("+");
    x1->Flag    = Flags::SPLIT | Flags::GROUPED;
    x1->ParseCB = ([](const char *block, const MatchBit &item, const UNumber length, void *ptr) noexcept -> String {
        double number = 0.0;
        double temnum = 0.0;
        String r;

        MatchBit *nm;
        for (UNumber i = 0; i < item.NestMatch.Size; i++) {
            nm = &(item.NestMatch[i]);

            if (nm->NestMatch.Size != 0) {
                r = Engine::Parse(nm->NestMatch, block, nm->Offset, nm->Length);
            } else {
                r = String::Part(block, nm->Offset, nm->Length);
            }

            if ((r.Length == 0) || !String::ToNumber(temnum, r.Str, 0, r.Length)) {
                return "0";
            }

            number += temnum;
        }
        return String::FromNumber(number);
    });

    Memory::AllocateBit<Expression>(&x2);
    x2->SetHead("*");
    x2->Flag    = Flags::SPLIT | Flags::GROUPED;
    x2->ParseCB = ([](const char *block, const MatchBit &item, const UNumber length, void *ptr) noexcept -> String {
        double number = 1.0;
        double temnum = 1.0;

        MatchBit *nm;
        for (UNumber i = 0; i < item.NestMatch.Size; i++) {
            nm = &(item.NestMatch[i]);
            if ((nm->Length == 0) || !String::ToNumber(temnum, block, nm->Offset, nm->Length)) {
                return "0";
            }

            number *= temnum;
        }
        return String::FromNumber(number);
    });

    x1->NestExpres.Add(x2);
    bit.Expres.Add(x1);
    bit.Collect.Add(x1).Add(x2);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("(1++2)").Add("( 1 ++ 2 )").Add("(1++)").Add("(++1)").Add("(1++2++3)");
    bit.Expected.Add("3").Add("3").Add("0").Add("0").Add("6");
    bit.Content.Add("(11++222)").Add("( 111 ++ 22 )").Add("(1111++)").Add("(++1111)").Add("(11++222++3333)");
    bit.Expected.Add("233").Add("133").Add("0").Add("0").Add("3566");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("++");
    x1->Flag = Flags::SPLIT | Flags::TRIM;

    Memory::AllocateBit<Expression>(&x2);
    x2->SetHead("(");
    x2->SetTail(")");
    x2->NestExpres.Add(x1);
    x2->ParseCB = ([](const char *block, const MatchBit &item, const UNumber length, void *ptr) noexcept -> String {
        double number = 0.0;
        double temnum = 0.0;

        MatchBit *nm;
        for (UNumber i = 0; i < item.NestMatch.Size; i++) {
            nm = &(item.NestMatch[i]);
            if ((nm->Length == 0) || !String::ToNumber(temnum, block, nm->Offset, nm->Length)) {
                return "0";
            }

            number += temnum;
        }
        return String::FromNumber(number);
    });

    bit.Expres.Add(x2);
    bit.Collect.Add(x1).Add(x2);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    // Note: Crazy mojo!
    bit.Content.Add("yx31xy").Add("x+1").Add("xx+1").Add("xx+1+2").Add("1+x").Add("1+xx").Add("1+yy").Add("1+3+yy");
    bit.Expected.Add("733137").Add("4").Add("34").Add("36").Add("4").Add("34").Add("78").Add("81");
    bit.Content.Add("yx+1+xy+2+y").Add("yx4xy+xx8y+y1yyy");
    bit.Expected.Add("120").Add("148601");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("+");
    x1->Flag    = Flags::SPLIT | Flags::GROUPED;
    x1->ParseCB = ([](const char *block, const MatchBit &item, const UNumber length, void *ptr) noexcept -> String {
        double number = 0.0;
        double temnum = 0.0;
        String r      = "";

        MatchBit *nm;
        for (UNumber i = 0; i < item.NestMatch.Size; i++) {
            nm = &(item.NestMatch[i]);

            if (nm->NestMatch.Size != 0) {
                r = Engine::Parse(nm->NestMatch, block, nm->Offset, nm->Length);
            } else {
                r = String::Part(block, nm->Offset, nm->Length);
            }

            r = String::Trim(r);
            if ((r.Length == 0) || !String::ToNumber(temnum, r.Str, 0, r.Length)) {
                return "0";
            }

            number += temnum;
        }
        return String::FromNumber(number);
    });

    Memory::AllocateBit<Expression>(&x2);
    x2->SetHead("x");
    x2->ParseCB = ([](const char *block, const MatchBit &item, const UNumber length, void *ptr) noexcept -> String { return "3"; });

    Memory::AllocateBit<Expression>(&x3);
    x3->SetHead("y");
    x3->ParseCB = ([](const char *block, const MatchBit &item, const UNumber length, void *ptr) noexcept -> String { return "7"; });

    bit.Expres.Add(x1).Add(x2).Add(x3);
    bit.Collect.Add(x1).Add(x2).Add(x3);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("{AAxx     yyyBBxx  yyyCC}");
    bit.Expected.Add("AABBCC");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("{");
    x1->SetTail("}");
    x1->ParseCB = ([](const char *block, const MatchBit &item, const UNumber length, void *ptr) noexcept -> String {
        String nc = "";

        for (UNumber i = 0; i < item.NestMatch.Size; i++) {
            nc += String::Part(block, item.NestMatch[i].Offset, item.NestMatch[i].Length);
        }

        return nc;
    });

    Memory::AllocateBit<Expression>(&x2);
    x2->SetHead("xx");
    x2->SetTail("yyy");
    x2->Flag = Flags::SPLIT;
    x1->NestExpres.Add(x2);

    bit.Expres.Add(x1);
    bit.Collect.Add(x1).Add(x2);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("{-{-g{-g#j-}j-}-}");
    bit.Expected.Add("[jg]");
    bit.Content.Add("{-g#j-}BB{-y#i-}").Add("{-g#j#y#i-}").Add("{-AA{-g#j-}BB{-y#i-}CC-}");
    bit.Expected.Add("[jg]BB[iy]").Add("[iyjg]").Add("[[iy][jg]]");
    bit.Content.Add("{-{-x#o-}#{-x#o-}#{-x#o-}-}");
    bit.Expected.Add("[[ox][ox][ox]]");
    bit.Content.Add("{-w#{-d#{-x#o-}#{-r#e#t-}#b-}t#g{-c#{-x#o-}-}-}");
    bit.Expected.Add("[[[ox]c][b[ter][ox]d]w]");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("{-");
    x1->SetTail("-}");
    x1->ParseCB = &(FlipSplit);

    Memory::AllocateBit<Expression>(&x2);
    x2->Flag = Flags::SPLIT;
    x2->SetHead("#");
    x1->NestExpres.Add(x1).Add(x2);

    bit.Expres.Add(x1);
    bit.Collect.Add(x1).Add(x2);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("{}").Add("{  }").Add("{ x }").Add("{{ {  { x }}  }}").Add("  &&&").Add("  a & b & c & & d  &");
    bit.Expected.Add("{}").Add("{  }").Add("x").Add("x").Add("").Add("abcd");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("{");
    x1->SetTail("}");
    x1->Flag = Flags::BUBBLE | Flags::TRIM | Flags::DROPEMPTY;
    x1->NestExpres.Add(x1);
    x1->ParseCB = ([](const char *block, const MatchBit &item, const UNumber length, void *ptr) noexcept -> String {
        if (length > 2) {
            UNumber start = 1;
            UNumber limit = length - 2;
            String::SoftTrim(block, start, limit);

            if (limit > 0) {
                return String::Part(block, start, limit);
            }
        }

        return block;
    });

    Memory::AllocateBit<Expression>(&x2);
    x2->SetHead("&");
    x2->Flag    = Flags::SPLIT | Flags::GROUPED | Flags::TRIM | Flags::DROPEMPTY;
    x2->ParseCB = ([](const char *block, const MatchBit &item, const UNumber length, void *ptr) noexcept -> String {
        String nc = "";

        for (UNumber i = 0; i < item.NestMatch.Size; i++) {
            nc += String::Part(block, item.NestMatch[i].Offset, item.NestMatch[i].Length);
        }

        return nc;
    });

    bit.Expres.Add(x1).Add(x2);
    bit.Collect.Add(x1).Add(x2);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("<div>ae<div>str0</div></diva>");
    bit.Expected.Add("<div>ae(str0)</diva>");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("<div>");
    x1->SetTail("</div>");
    x1->Flag = Flags::BUBBLE;
    x1->NestExpres.Add(x1);
    x1->ParseCB = ([](const char *block, const MatchBit &item, const UNumber length, void *ptr) noexcept -> String {
        String nc = "(";
        nc += String::Part(block, 5, (length - 11));
        nc += ")";
        return nc;
    });

    bit.Expres.Add(x1);
    bit.Collect.Add(x1);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("}-0");
    bit.Expected.Add("-0");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("}");
    x1->MatchCB =
        ([](const char *content, UNumber &offset, const UNumber endOffset, MatchBit &item, Array<MatchBit> &items) noexcept -> void {
            if (content[offset] == '-') {
                // ++item.Length;
                ++offset;
                items += static_cast<MatchBit &&>(item);
            }
        });

    Memory::AllocateBit<Expression>(&x2);
    x2->SetHead("-");
    x2->SetReplace("1");

    bit.Expres.Add(x1).Add(x2);
    bit.Collect.Add(x1).Add(x2);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("/").Add("//").Add("/r").Add("/ r");
    bit.Expected.Add("//").Add("////").Add("/r").Add("// r");

    Memory::AllocateBit<Expression>(&x1);
    x1->SetHead("/");
    x1->SetReplace("//");
    x1->MatchCB =
        ([](const char *content, UNumber &offset, const UNumber endOffset, MatchBit &item, Array<MatchBit> &items) noexcept -> void {
            if ((content[offset] == '/') || (content[offset] == ' ') || (offset == endOffset)) {
                // If there is a space after \ or \ or it's at the end, then it's a match.
                items += static_cast<MatchBit &&>(item);
            }
        });

    bit.Expres.Add(x1);
    bit.Collect.Add(x1);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    bit      = TestBit();
    bit.Line = __LINE__;

    bit.Content.Add("{[{'0'}]}").Add("{[[123}]");
    bit.Expected.Add("{1}").Add("{[1");

    Memory::AllocateBit<Expression>(&x1);
    Memory::AllocateBit<Expression>(&x2);
    Memory::AllocateBit<Expression>(&x3);

    x1->SetHead("{");
    x1->SetTail("}");
    x1->Flag = Flags::BUBBLE;
    x1->NestExpres += x1;
    x1->NestExpres += x2;
    x1->NestExpres += x3;
    x1->ParseCB = ([](const char *block, const MatchBit &item, const UNumber length, void *ptr) noexcept -> String { return block; });

    x2->SetHead("'");
    x2->SetTail("'");
    x2->SetReplace("X");

    x3->SetHead("[");
    x3->SetTail("]");
    x3->SetReplace("1");
    x3->NestExpres += x1;
    x3->NestExpres += x2;
    x3->NestExpres += x3;

    bit.Expres.Add(x1);
    bit.Collect.Add(x1).Add(x2).Add(x3);
    bits += static_cast<TestBit &&>(bit);
    ///////////////////////////////////////////
    return bits;
}

static String FlipSplit(const char *block, const MatchBit &item, const UNumber length, void *ptr) noexcept {
    String nc = "";

    if (item.NestMatch.Size != 0) {
        for (UNumber i = 0; i < item.NestMatch.Size; i++) {
            nc = FlipSplit(block, item.NestMatch[i], length, nullptr) + nc;
        }

        if (item.NestMatch.Size > 1) {
            nc = String("[") + nc + "]";
        }
    } else {
        nc = String::Part(block, item.Offset, item.Length);
    }

    return nc;
}

} // namespace Test
} // namespace Qentem

#endif
