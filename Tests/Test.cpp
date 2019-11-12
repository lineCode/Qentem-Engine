/**
 * Qentem Engine Test
 *
 * @brief     For testing Qentem Engine.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Test.hpp"
#include <Extension/XML.hpp>
#include <ctime>
#include <fstream>
#include <iostream>

using Qentem::Array;
using Qentem::Document;
using Qentem::String;
using Qentem::StringStream;
using Qentem::UNumber;
using Qentem::UShort;
using Qentem::Engine::MatchBit;
using Qentem::Test::TestBit;
using Qentem::XMLParser::XTag;

static UNumber const TimesToRun = 1;
// static bool const    StreasTest = true;
static bool const StreasTest = false;
// static bool const BigJSON    = true;
static bool const BigJSON = false;

static String   readFile(char const *path) noexcept;
static Document getDocument() noexcept;
static bool     runTests(wchar_t const *name, Array<TestBit> const &bits, bool break_on_err, Document *other = nullptr) noexcept;
static bool     NumbersConvTest() noexcept;
static bool     JSONTests() noexcept;
static bool     XMLTests() noexcept;

struct NCTest {
    double         Number{0};
    wchar_t const *Expected{nullptr};
    UShort         Max{0};

    NCTest() = default;

    NCTest(double n, wchar_t const *e) : Number(n), Expected(e) {
    }

    NCTest(double n, wchar_t const *e, UShort rma) : Number(n), Expected(e), Max(rma) {
    }
};

int main() {
    bool Pass  = false;
    bool Pause = false;

    bool TestEngine   = false;
    bool NumbersConv  = false;
    bool TestALU      = false;
    bool TestTemplate = false;
    bool TestXML      = false;
    bool TestJSON     = false;

    // This way is faster; just comment out the line instead of changing the value.
    // Pause = true;

    if (!BigJSON) {
        TestEngine   = true;
        NumbersConv  = true;
        TestALU      = true;
        TestTemplate = true;
        TestXML      = true;
    }

    TestJSON = true;

    Array<TestBit> bits;

    UNumber total = static_cast<UNumber>(clock());

    for (UNumber i = 0; i < TimesToRun; i++) {
        if (TestEngine) {
            // Core Engine Tests
            bits = Qentem::Test::GetEngineBits();
            Pass = runTests(L"Engine", bits, true);
            Qentem::Test::CleanBits(bits);
            if (!Pass) {
                break;
            }
            std::wcout << L"\n///////////////////////////////////////////////\n";
        }

        if (NumbersConv) {
            // Number Conversion Tests
            Pass = NumbersConvTest();
            if (!Pass) {
                break;
            }
            std::wcout << L"\n///////////////////////////////////////////////\n";
        }

        if (TestALU) {
            // Arithmetic & logic Evaluation Tests
            bits = Qentem::Test::GetALUBits();
            Pass = runTests(L"Arithmetic & Logic Evaluation", bits, true);
            if (!Pass) {
                break;
            }
            std::wcout << L"\n///////////////////////////////////////////////\n";
        }

        if (TestTemplate) {
            // Template Tests
            Document data;
            bits = Qentem::Test::GetTemplateBits(data);
            Pass = runTests(L"Template", bits, true, &data);
            if (!Pass) {
                break;
            }
            std::wcout << L"\n///////////////////////////////////////////////\n";
        }

        if (TestXML) {
            // JSON Tests
            Pass = XMLTests();
            if (!Pass) {
                break;
            }
            std::wcout << L"\n///////////////////////////////////////////////\n";
        }

        if (TestJSON) {
            // JSON Tests
            Pass = JSONTests();
            if (!Pass) {
                break;
            }
            std::wcout << L"\n///////////////////////////////////////////////\n";
        }
    }

    total = (static_cast<UNumber>(clock()) - total);

    if (Pass) {
        std::wcout << L"\n ALL GOOD. Took: " << String::FromNumber((static_cast<double>(total) / CLOCKS_PER_SEC), 1, 3, 3).Str << L"s\n\n";
        if (Pause) {
            std::getwchar();
        }
        return 10;
    }

    std::wcout << L"\n Something is wrong!" << L"\n\n";

    return 0;
}

static bool runTests(wchar_t const *name, Array<TestBit> const &bits, bool break_on_err, Document *other) noexcept {
    UNumber const times        = StreasTest ? 10000 : 1;
    UNumber const start_at     = 0;
    UNumber       counter      = 0;
    UNumber       fail         = 0;
    UNumber       total        = 0;
    UNumber       total_search = 0;
    UNumber       search_ticks = 0;
    UNumber       total_parse  = 0;
    UNumber       parse_ticks  = 0;
    UNumber       count        = start_at;
    UNumber       length       = 0;
    bool          Pass         = false;

    StringStream    ss;
    Array<MatchBit> matches;

    ss += L"\n #";
    ss += name;
    ss += L" Tests:\n";

    if (start_at != 0) {
        ss += L"\n Starting at ";
        ss += String::FromNumber(start_at);
        if (counter == 0) {
            ss += L"\n\n";
        }
    }

    if (counter != 0) {
        ss += L"\n Counter is set to ";
        ss += String::FromNumber(counter);
        ss += L"\n\n";
    }

    for (UNumber i = start_at; i < bits.Size; i++) {
        ++count;

        if (bits[i].Expected.Size != bits[i].Content.Size) {
            std::wcout << L"Check Expected & Content Size @" << String::FromNumber(bits[i].Line).Str << L'\n';
            return false;
        }

        for (UNumber t = counter; t < bits[i].Content.Size; t++) {
            length = String::Count(bits[i].Content[t]);

            search_ticks = static_cast<UNumber>(clock());
            for (UNumber x = 0; x < times; x++) {
                matches = Qentem::Engine::Match(bits[i].Expres, bits[i].Content[t], 0, length);
            }
            search_ticks = (static_cast<UNumber>(clock()) - search_ticks);
            total_search += search_ticks;

            String rendered;
            parse_ticks = static_cast<UNumber>(clock());
            for (UNumber y = 0; y < times; y++) {
                rendered = Qentem::Engine::Parse(matches, bits[i].Content[t], 0, length, other);
            }
            parse_ticks = (static_cast<UNumber>(clock()) - parse_ticks);
            total_parse += parse_ticks;

            ++counter;
            ++total;

            Pass = (rendered == bits[i].Expected[t]);
            ss += Pass ? L" " : L"\n ";

            ss += String::FromNumber(count, 2) + L"-";
            ss += String::FromNumber(counter, 2) + L": ";

            if (Pass) {
                ss += L"Pass";
            } else {
                ss += L"Fail";
            }

            ss += L" (Match: ";
            ss += String::FromNumber((static_cast<double>(search_ticks) / CLOCKS_PER_SEC), 2, 3, 3) + L")";
            ss += L" (Parse: ";
            ss += String::FromNumber((static_cast<double>(parse_ticks) / CLOCKS_PER_SEC), 2, 3, 3) + L")\n";

            if (!Pass) {
                ++fail;

                ss += L" ----------- Start debug ";
                ss += String::FromNumber(count) + L"-";
                ss += String::FromNumber(counter);
                ss += L" -----\n  Line:      ";
                ss += String::FromNumber(bits[i].Line) + L"\n";
                ss += L"  Content:  ";
                ss += Qentem::Test::ReplaceNewLine(bits[i].Content[t], length, L"\\n") + L"\n";
                ss += L"  Rendered: \"";
                ss += Qentem::Test::ReplaceNewLine(rendered.Str, rendered.Length, L"\\n") + L"\"\n";
                ss += L"  Expected: \"";
                ss += Qentem::Test::ReplaceNewLine(bits[i].Expected[t], String::Count(bits[i].Expected[t]), L"\\n") + L"\"\n";
                ss += L"  Matches:\n";
                ss += Qentem::Test::DumpMatches(bits[i].Content[t], matches, L"    ");

                ss += L"\n  ---------- End debug ";
                ss += String::FromNumber(count) + L"-";
                ss += String::FromNumber(counter) + L" -------\n";

                if (break_on_err) {
                    break;
                }
            }

            std::wcout << ss.Eject().Str;
        }

        counter = 0;

        if (!Pass && break_on_err) {
            break;
        }
    }

    ss += L"\n ";
    ss += name;
    ss += L" is";

    if (fail == 0) {
        ss += L" operational! (Total Tests: ";
        ss += String::FromNumber(total) + L")";
    } else {
        ss += L" broken! (Failed: ";
        ss += String::FromNumber(fail) + L",  out of: ";
        ss += String::FromNumber(total) + L")";
    }

    ss += L", Total Match: ";
    ss += String::FromNumber((static_cast<double>(total_search) / CLOCKS_PER_SEC), 2, 3, 3);
    ss += L"s Total Parse: ";
    ss += String::FromNumber((static_cast<double>(total_parse) / CLOCKS_PER_SEC), 2, 3, 3) + L"s\n";

    std::wcout << ss.Eject().Str;

    return (fail == 0);
}

static bool NumbersConvTest() noexcept {
    Array<NCTest> test;
    UNumber const times       = StreasTest ? 100000 : 1;
    UNumber       ticks       = 0;
    UNumber       total_ticks = 0;
    bool          Pass        = false;
    ////////////////////////////////
    test.Add({0, L"0"}).Add({1, L"1"}).Add({-2, L"-2"}).Add({5, L"5"}).Add({9, L"9"}).Add({-3.1, L"-3.1"});
    test.Add({1000000, L"1000000"}).Add({12, L"12"}).Add({23, L"23"}).Add({55, L"55"}).Add({199, L"199"});
    test.Add({10.0, L"10"}).Add({11.00, L"11"}).Add({-22.87, L"-22.87", 2}).Add({-55.0055, L"-55.0055", 10});

    test.Add({-0.123455678987455, L"-0.123455678987455"}).Add({-0.123455678987452, L"-0.123455678987452"});
    test.Add({0.999999, L"1", 5}).Add({0.999999, L"0.999999", 6}).Add({0.999999, L"0.999999", 10});
    test.Add({0.123e-12, L"0.000000000000123"}).Add({9876.543210, L"9876.54321", 5});
    test.Add({-2.000000000000999, L"-2.000000000000999"}).Add({3.9999999999999, L"3.9999999999999", 14});
    test.Add({99.1005099, L"99.1005099", 7}).Add({871.080055555, L"871.080055555", 9});
    test.Add({999.1000099, L"999.1000099", 10}).Add({9.1000099, L"9.1000099", 10});
    test.Add({66666.30000400000001, L"66666.300004", 10}).Add({2.30000000000001, L"2.30000000000001"});
    test.Add({22.3000000000006, L"22.3000000000006", 13}).Add({22.30000000000006, L"22.30000000000006", 14});
    test.Add({22.300000000000006, L"22.3", 1}).Add({9788871.080055555, L"9788871.080055555", 9});
    test.Add({6666666.30000400000001, L"6666666.300004", 9}).Add({22.300000000000055, L"22.30000000000005", 14});
    test.Add({22.300000000000059, L"22.30000000000006", 14}).Add({71.080055555, L"71.080055555", 9});
    test.Add({1.000055555, L"1.000055555"}).Add({22.300000000000054, L"22.300000000000054"});
    test.Add({2.00000000000001, L"2.00000000000001"}).Add({2.000000000000001, L"2", 1});
    test.Add({222.300000000000055, L"222.3", 1}).Add({0.11111111111111, L"0.11111111111111"});
    test.Add({2.00000000000001, L"2.00000000000001"}).Add({22.00000000000005, L"22.00000000000005", 14});
    test.Add({22.300000000000055, L"22.30000000000005", 14});
    test.Add({2.000000000000015, L"2.000000000000015"}).Add({22.000055555, L"22.000055555", 12});
    test.Add({2.00000000000051, L"2.00000000000051", 14}).Add({2222.000000000000055, L"2222"});
    test.Add({2222.0000000000001, L"2222"}).Add({2.00000000000001, L"2.00000000000001"});
    test.Add({222.00000000000001, L"222"}).Add({333.99999999999999, L"334"});
    test.Add({22.87, L"22.87", 2}).Add({-0.788065000000079, L"-0.788065000000079"});
    test.Add({9.99999999999901, L"9.99999999999901"}).Add({3.99999999999909, L"3.99999999999909"});
    test.Add({55.0055, L"55.0055", 14}).Add({55.0000055, L"55.0000055"});

    test.Add({0.00056599999999999999, L"0.001", 3});
    ////////////////////////////////

    std::wcout << L"\n #Number Conversion Tests:\n";

    for (UNumber i = 0; i < test.Size; i++) {
        std::wcout << L" " << String::FromNumber((i + 1), 2).Str << L": ";

        ticks = static_cast<UNumber>(clock());

        for (UNumber k = 0; k < times; k++) {
            Pass = (String::FromNumber(test[i].Number, 1, 0, test[i].Max) == test[i].Expected);
        }

        ticks = (static_cast<UNumber>(clock()) - ticks);
        total_ticks += ticks;

        if (Pass) {
            std::wcout << L"Pass " << String::FromNumber((static_cast<double>(ticks) / CLOCKS_PER_SEC), 2, 3, 3).Str << L'\n';
        } else {
            std::wcout << L"Fail: " << String::FromNumber(test[i].Number, 1, 0, test[i].Max).Str << L" Expected: " << test[i].Expected
                       << L'\n';
            std::wcout << L"\n Number Conversion failed to pass the test.\n";

            return false;
        }
    }

    std::wcout << L"\n Number Conversion is operational!";
    std::wcout << L" Total: " << String::FromNumber((static_cast<double>(total_ticks) / CLOCKS_PER_SEC), 2, 3, 3).Str << L'\n';

    ////////////////////////////////
    return true;
}

static bool XMLTests() noexcept {
    Array<XTag>   tags;
    UNumber const times = StreasTest ? 30000 : 1;
    UNumber       ticks = 0;
    bool          Pass  = false;
    std::wcout << L"\n #XML Tests:\n";

    String XMLContent = readFile("./Tests/test.html");
    if (XMLContent.Length == 0) {
        XMLContent = readFile("./test.html");
    }

    ticks = static_cast<UNumber>(clock());
    for (UNumber k = 0; k < times; k++) {
        tags = Qentem::XMLParser::Parse(XMLContent);
    }
    ticks = (static_cast<UNumber>(clock()) - ticks);

    if (tags.Size != 0) {
        Pass = true;

        if (tags.Size == 4) {
            std::wcout << L" Pass";
        } else {
            std::wcout << L" Fail";
            Pass = false;
        }
        std::wcout << L" Tags' count: " << String::FromNumber(tags.Size).Str << L'\n';

        UNumber id;
        if (Pass) {
            id = 0;
            if (tags[id].Name == L"br") {
                std::wcout << L" Pass";
            } else {
                std::wcout << L" Fail";
                Pass = false;
            }
            std::wcout << L" Tag id:" << String::FromNumber(id).Str << L" (" << tags[id].Name.Str << L") inline tag\n";
        }

        if (Pass) {
            id = 1;
            if (tags[id].Name == L"img") {
                std::wcout << L" Pass";
            } else {
                std::wcout << L" Fail";
                Pass = false;
            }
            std::wcout << L" Tag id:" << String::FromNumber(id).Str << L" (" << tags[id].Name.Str << L") inline tag\n";
        }

        if (Pass) {
            id = 1;
            if ((tags[id].Properties.Size == 3) && (tags[id].Properties[0].Name == L"src") && (tags[id].Properties[1].Name == L"id") &&
                (tags[id].Properties[2].Name == L"class") && (tags[id].Properties[0].Value == L"www") &&
                (tags[id].Properties[1].Value == L"  m  ") && (tags[id].Properties[2].Value == L"y")) {
                std::wcout << L" Pass";
            } else {
                std::wcout << L" Fail";
                Pass = false;
            }
            std::wcout << L" Tag id:" << String::FromNumber(id).Str << L" (" << tags[id].Name.Str << L") properties count: "
                       << String::FromNumber(tags[id].Properties.Size).Str << L'\n';
        }

        if (Pass) {
            id = 2;
            if ((tags[id].Name == L"div") && (tags[id].InnerNodes.Size == 2) && (tags[id].InnerNodes[0].InnerText == L"string")) {
                std::wcout << L" Pass";
            } else {
                std::wcout << L" Fail";
                Pass = false;
            }
            std::wcout << L" Tag id:" << String::FromNumber(id).Str << L" (" << tags[id].Name.Str << L") InnerNodes' count: "
                       << String::FromNumber(tags[id].InnerNodes.Size).Str << L'\n';
        }

        if (Pass) {
            id = 3;
            if ((tags[id].Name == L"p") && tags[id].InnerText == L"para") {
                std::wcout << L" Pass";
            } else {
                std::wcout << L" Fail";
                Pass = false;
            }
            std::wcout << L" Tag id:" << String::FromNumber(id).Str << L" (" << tags[id].Name.Str << L") Inner text: "
                       << tags[id].InnerText.Str << L'\n';
        }
    }

    if (Pass) {
        std::wcout << L"\n XML looks good!";
        std::wcout << L" Took: " << String::FromNumber((static_cast<double>(ticks) / CLOCKS_PER_SEC), 2, 3, 3).Str << L'\n';
    } else {
        std::wcout << L"\n XML test failed!\n\n";
    }

    return Pass;
}

static bool JSONTests() noexcept {
    UNumber const times = ((StreasTest && !BigJSON) ? 1000 : 1);
    UNumber       took  = 0;
    String        final = L"";
    Document      data;
    std::wcout << L"\n #JSON Tests:\n";

    String json_content = readFile(!BigJSON ? "./Tests/test.json" : "./Tests/bigjson.json");
    if (json_content.Length == 0) {
        json_content = readFile(!BigJSON ? "./test.json" : "./bigjson.json");
    }

    if (!BigJSON) {
        data             = getDocument();
        final            = data.ToJSON();
        String n_content = Qentem::Test::ReplaceNewLine(json_content.Str, json_content.Length, L"");
        n_content        = Qentem::Test::Replace(n_content.Str, n_content.Length, L"\": ", L"\":");
        if (final != n_content) {
            std::wcout << L"\n Document() might be broken!\n";
            std::wcout << L"\n File:\n" << n_content.Str << L"\n";
            std::wcout << L"\n Document():\n" << final.Str << L"\n";
            return false;
        }
    }

    for (UNumber i = 0; i < 10; i++) {
        std::wcout << L" Importing... ";
        took = static_cast<UNumber>(clock());
        for (UNumber y = 1; y <= times; y++) {
            data = Document::FromJSON(json_content);
            // Qentem::Engine::Match(json_content.Str, Document::getJsonExpres(), 0, json_content.Length);
        }
        took = (static_cast<UNumber>(clock()) - took);
        std::wcout << String::FromNumber((static_cast<double>(took) / CLOCKS_PER_SEC), 2, 3, 3).Str << L" ";

        std::wcout << L" Exporting... ";
        took = static_cast<UNumber>(clock());
        for (UNumber y = 1; y <= times; y++) {
            final = data.ToJSON();
        }
        took = (static_cast<UNumber>(clock()) - took);
        std::wcout << String::FromNumber((static_cast<double>(took) / CLOCKS_PER_SEC), 2, 3, 3).Str << L'\n';
    }

    if (BigJSON) {
        std::wcout << L"\n Big JSON Run\n";
        return true;
    }

    json_content = Qentem::Test::ReplaceNewLine(json_content.Str, json_content.Length, L"");
    json_content = Qentem::Test::Replace(json_content.Str, json_content.Length, L"\": ", L"\":");

    if (final == json_content) {
        std::wcout << L"\n JSON looks good!\n";
        return true;
    }

    std::wcout << L"\n JSON is borken!\n\n";
    std::wcout << L"\n-File:\n";
    std::wcout << json_content.Str;
    std::wcout << L"\n-End-\n";
    std::wcout << L"\n-ToJSON:\n";
    std::wcout << final.Str;
    std::wcout << L"\n-End-\n";

    return false;
}

static Document getDocument() noexcept {
    Document data = Document();

    data[L"w"]    = L"r\\\\";
    data[L"var1"] = L"\"1\"";
    data[L"PP"]   = L"gg";
    // String sss     = data[L"PP"].GetString();
    data[L"nu"]    = nullptr;
    data[L"bool"]  = false;
    data[L"bool2"] = true;
    data[L"&&"]    = 1000.0;

    data[L"var2"]       = L"11";
    data[L"var2"]       = L"2"; // Testing override .
    data[L"var3"]       = L"3";
    data[L"var4"]       = L"4";
    data[L"var5"]       = L"5";
    data[L"num34"]      = Array<String>().Add(L"3").Add(L"4");
    data[L"var_string"] = L"image";
    data[L"engine"]     = L"Qentem";
    data[L"abc1"]       = Array<String>().Add(L"B").Add(L"C").Add(L"D").Add(L"A");
    data[L"abc2"]       = Array<String>().Add(L"E").Add(L"F").Add(L"A");
    data[L"numbers"]    = Array<double>().Add(0).Add(1).Add(2).Add(3).Add(4).Add(5);
    data[L"empty"]      = L"";
    data[L"math"]       = L"((2* (1 * 3)) + 1 - 4) + (((10 - 5) - 6 + ((1 + 1) + (1 + 1))) * (8 / 4 + 1)) - (1) - (-1) + 2";

    data[L"abc"]         = Document();
    data[L"abc"][L"B"]   = L"b";
    data[L"abc"][L"@@"]  = 100.0;
    data[L"abc"][L"A"]   = L"a";
    data[L"abc"][L"C"]   = L"c";
    data[L"abc"][L"D"]   = L"d";
    data[L"abc"][L"E"]   = Array<String>().Add(L"O").Add(L"K!");
    data[L"abc"][L"A-Z"] = L"ABCDEFGHIGKLMNOBQRST.....";

    data[L"multi"]                = Document();
    data[L"multi"][L"arr1"]       = Document();
    data[L"multi"][L"arr1"][L"E"] = Array<String>().Add(L"O").Add(L"K!");
    data[L"multi"][L"arr2"]       = Array<String>().Add(L"B").Add(L"C").Add(L"D").Add(L"A");
    data[L"multi"][L"C"]          = L"cool";

    return data;
}

static String readFile(char const *path) noexcept {
    std::ifstream file(path, std::ios::ate | std::ios::out);
    if (file.is_open()) {
        std::streampos size = file.tellg();
        file.seekg(0, std::ios::beg);

        char *file_content;
        Qentem::Memory::Allocate<char>(&file_content, UNumber(size));
        file.read(file_content, size);

        file.close();

        String content(file_content);
        Qentem::Memory::Deallocate<char>(&file_content);

        return content;
    }

    std::wcout << L'\n' << path << L" does not exist!\n";

    return L"";
}
