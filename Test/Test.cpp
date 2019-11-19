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

enum ParseType { Engine = 0, ALE = 1 };

const static UNumber TimesToRun = 1;
// static const bool    StreasTest = true;
static const bool StreasTest = false;
// static const bool BigJSON    = true;
static const bool BigJSON = false;

static String   readFile(const char *path) noexcept;
static Document getDocument() noexcept;
static bool     runTest(const char *name, const Array<TestBit> &bits, bool break_on_err, Document *other = nullptr,
                        ParseType parse_type = ParseType::Engine) noexcept;
static bool     NumbersConvTest() noexcept;
static bool     JSONTest() noexcept;
static bool     XMLTest() noexcept;

struct NCTest {
    double      Number{0};
    const char *Expected{nullptr};
    UShort      Max{0};

    NCTest() = default;

    NCTest(double n, const char *e) : Number(n), Expected(e) {
    }

    NCTest(double n, const char *e, UShort rma) : Number(n), Expected(e), Max(rma) {
    }
};

int main() {
    bool Pass  = false;
    bool Pause = false;

    bool TestEngine   = false;
    bool NumbersConv  = false;
    bool TestALE      = false;
    bool TestTemplate = false;
    bool TestXML      = false;
    bool TestJSON     = false;

    // This way is faster; just comment out the line instead of changing the value.
    // Pause = true;

    if (!BigJSON) {
        TestEngine   = true;
        NumbersConv  = true;
        TestALE      = true;
        TestTemplate = true;
        TestXML      = true;
    }

    TestJSON = true;

    Array<TestBit> bits;

    UNumber total = static_cast<UNumber>(clock());

    for (UNumber i = 0; i < TimesToRun; i++) {
        if (TestEngine) {
            // Core Engine Test
            bits = Qentem::Test::GetEngineBits();
            Pass = runTest("Engine", bits, true);
            Qentem::Test::CleanBits(bits);
            if (!Pass) {
                break;
            }
            std::cout << "\n///////////////////////////////////////////////\n";
        }

        if (NumbersConv) {
            // Number Conversion Test
            Pass = NumbersConvTest();
            if (!Pass) {
                break;
            }
            std::cout << "\n///////////////////////////////////////////////\n";
        }

        if (TestALE) {
            // Arithmetic & logic Evaluation Test
            bits = Qentem::Test::GetALEBits();
            Pass = runTest("Arithmetic & Logic Evaluation", bits, true, nullptr, ParseType::ALE);
            if (!Pass) {
                break;
            }
            std::cout << "\n///////////////////////////////////////////////\n";
        }

        if (TestTemplate) {
            // Template Test
            Document data;
            bits = Qentem::Test::GetTemplateBits(data);
            Pass = runTest("Template", bits, true, &data);
            if (!Pass) {
                break;
            }
            std::cout << "\n///////////////////////////////////////////////\n";
        }

        if (TestXML) {
            // JSON Test
            Pass = XMLTest();
            if (!Pass) {
                break;
            }
            std::cout << "\n///////////////////////////////////////////////\n";
        }

        if (TestJSON) {
            // JSON Test
            Pass = JSONTest();
            if (!Pass) {
                break;
            }
            std::cout << "\n///////////////////////////////////////////////\n";
        }
    }

    total = (static_cast<UNumber>(clock()) - total);

    if (Pass) {
        std::cout << "\n ALL GOOD. Took: " << String::FromNumber((static_cast<double>(total) / CLOCKS_PER_SEC), 1, 3, 3).Str << "s\n\n";
        if (Pause) {
            std::getwchar();
        }
        return 10;
    }

    std::cout << "\n Something is wrong!"
              << "\n\n";

    return 0;
}

static bool runTest(const char *name, const Array<TestBit> &bits, bool break_on_err, Document *other, const ParseType parse_type) noexcept {
    const UNumber times        = StreasTest ? 10000 : 1;
    const UNumber start_at     = 0;
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

    ss += "\n #";
    ss += name;
    ss += " Test:\n";

    if (start_at != 0) {
        ss += "\n Starting at ";
        ss += String::FromNumber(start_at);
        if (counter == 0) {
            ss += "\n\n";
        }
    }

    if (counter != 0) {
        ss += "\n Counter is set to ";
        ss += String::FromNumber(counter);
        ss += "\n\n";
    }

    for (UNumber i = start_at; i < bits.Size; i++) {
        ++count;

        if (bits[i].Expected.Size != bits[i].Content.Size) {
            std::cout << "Check Expected & Content Size @" << String::FromNumber(bits[i].Line).Str << '\n';
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

            switch (parse_type) {
                case ParseType::ALE: {
                    double  ALE_num = 0.0;
                    UNumber offset  = 0;
                    UNumber limit   = length;
                    String::SoftTrim(bits[i].Content[t], offset, limit);

                    parse_ticks = static_cast<UNumber>(clock());
                    for (UNumber y = 0; y < times; y++) {
                        Qentem::ALE::Process(ALE_num, matches, bits[i].Content[t], offset, limit);
                    }
                    parse_ticks = (static_cast<UNumber>(clock()) - parse_ticks);
                    rendered    = String::FromNumber(ALE_num, 1, 0, 3);
                    break;
                }
                default: {
                    parse_ticks = static_cast<UNumber>(clock());
                    for (UNumber y = 0; y < times; y++) {
                        rendered = Qentem::Engine::Parse(matches, bits[i].Content[t], 0, length, other);
                    }
                    parse_ticks = (static_cast<UNumber>(clock()) - parse_ticks);
                    break;
                }
            }

            total_parse += parse_ticks;

            Pass = (rendered == bits[i].Expected[t]);

            ++counter;
            ++total;

            ss += Pass ? " " : "\n ";

            ss += String::FromNumber(count, 2) + "-";
            ss += String::FromNumber(counter, 2) + ": ";

            if (Pass) {
                ss += "Pass";
            } else {
                ss += "Fail";
            }

            ss += " (Match: ";
            ss += String::FromNumber((static_cast<double>(search_ticks) / CLOCKS_PER_SEC), 2, 3, 3) + ")";
            ss += " (Parse: ";
            ss += String::FromNumber((static_cast<double>(parse_ticks) / CLOCKS_PER_SEC), 2, 3, 3) + ")\n";

            if (!Pass) {
                ++fail;

                ss += " ----------- Start debug ";
                ss += String::FromNumber(count) + "-";
                ss += String::FromNumber(counter);
                ss += " -----\n  Line:      ";
                ss += String::FromNumber(bits[i].Line) + "\n";
                ss += "  Content:  ";
                ss += Qentem::Test::ReplaceNewLine(bits[i].Content[t], length, "\\n") + "\n";
                ss += "  Rendered: \"";
                ss += Qentem::Test::ReplaceNewLine(rendered.Str, rendered.Length, "\\n") + "\"\n";
                ss += "  Expected: \"";
                ss += Qentem::Test::ReplaceNewLine(bits[i].Expected[t], String::Count(bits[i].Expected[t]), "\\n") + "\"\n";
                ss += "  Matches:\n";
                ss += Qentem::Test::DumpMatches(matches, bits[i].Content[t], "    ");

                ss += "\n  ---------- End debug ";
                ss += String::FromNumber(count) + "-";
                ss += String::FromNumber(counter) + " -------\n";

                if (break_on_err) {
                    break;
                }
            }

            std::cout << ss.ToString().Str;
        }

        counter = 0;

        if (!Pass && break_on_err) {
            break;
        }
    }

    ss += "\n ";
    ss += name;
    ss += " is";

    if (fail == 0) {
        ss += " operational! (Total Tests: ";
        ss += String::FromNumber(total) + ")";
    } else {
        ss += " broken! (Failed: ";
        ss += String::FromNumber(fail) + ",  out of: ";
        ss += String::FromNumber(total) + ")";
    }

    ss += ", Total Match: ";
    ss += String::FromNumber((static_cast<double>(total_search) / CLOCKS_PER_SEC), 2, 3, 3);
    ss += "s Total Parse: ";
    ss += String::FromNumber((static_cast<double>(total_parse) / CLOCKS_PER_SEC), 2, 3, 3) + "s\n";

    std::cout << ss.ToString().Str;

    return (fail == 0);
}

static bool NumbersConvTest() noexcept {
    Array<NCTest> test;
    const UNumber times       = StreasTest ? 100000 : 1;
    UNumber       ticks       = 0;
    UNumber       total_ticks = 0;
    bool          Pass        = false;
    ////////////////////////////////
    test.Add({0, "0"}).Add({1, "1"}).Add({-2, "-2"}).Add({5, "5"}).Add({9, "9"}).Add({-3.1, "-3.1"});
    test.Add({1000000, "1000000"}).Add({12, "12"}).Add({23, "23"}).Add({55, "55"}).Add({199, "199"});
    test.Add({10.0, "10"}).Add({11.00, "11"}).Add({-22.87, "-22.87", 2}).Add({-55.0055, "-55.0055", 10});

    test.Add({-0.123455678987455, "-0.123455678987455"}).Add({-0.123455678987452, "-0.123455678987452"});
    test.Add({0.999999, "1", 5}).Add({0.999999, "0.999999", 6}).Add({0.999999, "0.999999", 10});
    test.Add({0.123e-12, "0.000000000000123"}).Add({9876.543210, "9876.54321", 5});
    test.Add({-2.000000000000999, "-2.000000000000999"}).Add({3.9999999999999, "3.9999999999999", 14});
    test.Add({99.1005099, "99.1005099", 7}).Add({871.080055555, "871.080055555", 9});
    test.Add({999.1000099, "999.1000099", 10}).Add({9.1000099, "9.1000099", 10});
    test.Add({66666.30000400000001, "66666.300004", 10}).Add({2.30000000000001, "2.30000000000001"});
    test.Add({22.3000000000006, "22.3000000000006", 13}).Add({22.30000000000006, "22.30000000000006", 14});
    test.Add({22.300000000000006, "22.3", 1}).Add({9788871.080055555, "9788871.080055555", 9});
    test.Add({6666666.30000400000001, "6666666.300004", 9}).Add({22.300000000000055, "22.30000000000005", 14});
    test.Add({22.300000000000059, "22.30000000000006", 14}).Add({71.080055555, "71.080055555", 9});
    test.Add({1.000055555, "1.000055555"}).Add({22.300000000000054, "22.300000000000054"});
    test.Add({2.00000000000001, "2.00000000000001"}).Add({2.000000000000001, "2", 1});
    test.Add({222.300000000000055, "222.3", 1}).Add({0.11111111111111, "0.11111111111111"});
    test.Add({2.00000000000001, "2.00000000000001"}).Add({22.00000000000005, "22.00000000000005", 14});
    test.Add({22.300000000000055, "22.30000000000005", 14});
    test.Add({2.000000000000015, "2.000000000000015"}).Add({22.000055555, "22.000055555", 12});
    test.Add({2.00000000000051, "2.00000000000051", 14}).Add({2222.000000000000055, "2222"});
    test.Add({2222.0000000000001, "2222"}).Add({2.00000000000001, "2.00000000000001"});
    test.Add({222.00000000000001, "222"}).Add({333.99999999999999, "334"});
    test.Add({22.87, "22.87", 2}).Add({-0.788065000000079, "-0.788065000000079"});
    test.Add({9.99999999999901, "9.99999999999901"}).Add({3.99999999999909, "3.99999999999909"});
    test.Add({55.0055, "55.0055", 14}).Add({55.0000055, "55.0000055"});

    test.Add({0.00056599999999999999, "0.001", 3});
    ////////////////////////////////

    std::cout << "\n #Number Conversion Test:\n";

    for (UNumber i = 0; i < test.Size; i++) {
        std::cout << " " << String::FromNumber((i + 1), 2).Str << ": ";

        ticks = static_cast<UNumber>(clock());

        for (UNumber k = 0; k < times; k++) {
            Pass = (String::FromNumber(test[i].Number, 1, 0, test[i].Max) == test[i].Expected);
        }

        ticks = (static_cast<UNumber>(clock()) - ticks);
        total_ticks += ticks;

        if (Pass) {
            std::cout << "Pass " << String::FromNumber((static_cast<double>(ticks) / CLOCKS_PER_SEC), 2, 3, 3).Str << '\n';
        } else {
            std::cout << "Fail: " << String::FromNumber(test[i].Number, 1, 0, test[i].Max).Str << " Expected: " << test[i].Expected << '\n';
            std::cout << "\n Number Conversion failed to pass the test.\n";

            return false;
        }
    }

    std::cout << "\n Number Conversion is operational!";
    std::cout << " Total: " << String::FromNumber((static_cast<double>(total_ticks) / CLOCKS_PER_SEC), 2, 3, 3).Str << '\n';

    ////////////////////////////////
    return true;
}

static bool XMLTest() noexcept {
    Array<XTag>   tags;
    const UNumber times = StreasTest ? 30000 : 1;
    UNumber       ticks = 0;
    bool          Pass  = false;
    std::cout << "\n #XML Test:\n";

    String XMLContent = readFile("./Test/test.html");
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
            std::cout << " Pass";
        } else {
            std::cout << " Fail";
            Pass = false;
        }
        std::cout << " Tags' count: " << String::FromNumber(tags.Size).Str << '\n';

        UNumber id;
        if (Pass) {
            id = 0;
            if (tags[id].Name == "br") {
                std::cout << " Pass";
            } else {
                std::cout << " Fail";
                Pass = false;
            }
            std::cout << " Tag id:" << String::FromNumber(id).Str << " (" << tags[id].Name.Str << ") inline tag\n";
        }

        if (Pass) {
            id = 1;
            if (tags[id].Name == "img") {
                std::cout << " Pass";
            } else {
                std::cout << " Fail";
                Pass = false;
            }
            std::cout << " Tag id:" << String::FromNumber(id).Str << " (" << tags[id].Name.Str << ") inline tag\n";
        }

        if (Pass) {
            id = 1;
            if ((tags[id].Properties.Size == 3) && (tags[id].Properties[0].Name == "src") && (tags[id].Properties[1].Name == "id") &&
                (tags[id].Properties[2].Name == "class") && (tags[id].Properties[0].Value == "www") &&
                (tags[id].Properties[1].Value == "  m  ") && (tags[id].Properties[2].Value == "y")) {
                std::cout << " Pass";
            } else {
                std::cout << " Fail";
                Pass = false;
            }
            std::cout << " Tag id:" << String::FromNumber(id).Str << " (" << tags[id].Name.Str
                      << ") properties count: " << String::FromNumber(tags[id].Properties.Size).Str << '\n';
        }

        if (Pass) {
            id = 2;
            if ((tags[id].Name == "div") && (tags[id].InnerNodes.Size == 2) && (tags[id].InnerNodes[0].InnerText == "string")) {
                std::cout << " Pass";
            } else {
                std::cout << " Fail";
                Pass = false;
            }
            std::cout << " Tag id:" << String::FromNumber(id).Str << " (" << tags[id].Name.Str
                      << ") InnerNodes' count: " << String::FromNumber(tags[id].InnerNodes.Size).Str << '\n';
        }

        if (Pass) {
            id = 3;
            if ((tags[id].Name == "p") && tags[id].InnerText == "para") {
                std::cout << " Pass";
            } else {
                std::cout << " Fail";
                Pass = false;
            }
            std::cout << " Tag id:" << String::FromNumber(id).Str << " (" << tags[id].Name.Str << ") Inner text: " << tags[id].InnerText.Str
                      << '\n';
        }
    }

    if (Pass) {
        std::cout << "\n XML looks good!";
        std::cout << " Took: " << String::FromNumber((static_cast<double>(ticks) / CLOCKS_PER_SEC), 2, 3, 3).Str << '\n';
    } else {
        std::cout << "\n XML test failed!\n\n";
    }

    return Pass;
}

static bool JSONTest() noexcept {
    const UNumber times = ((StreasTest && !BigJSON) ? 1000 : 1);
    UNumber       took  = 0;
    String        final = "";
    Document      data;
    std::cout << "\n #JSON Test:\n";

    String json_content = readFile(!BigJSON ? "./Test/test.json" : "./Test/bigjson.json");
    if (json_content.Length == 0) {
        json_content = readFile(!BigJSON ? "./test.json" : "./bigjson.json");
    }

    if (!BigJSON) {
        data             = getDocument();
        final            = data.ToJSON();
        String n_content = Qentem::Test::ReplaceNewLine(json_content.Str, json_content.Length, "");
        n_content        = Qentem::Test::Replace(n_content.Str, n_content.Length, "\": ", "\":");
        if (final != n_content) {
            std::cout << "\n Document() might be broken!\n";
            std::cout << "\n File:\n" << n_content.Str << "\n";
            std::cout << "\n Document():\n" << final.Str << "\n";
            return false;
        }
    }

    for (UNumber i = 0; i < 10; i++) {
        std::cout << " Importing... ";
        took = static_cast<UNumber>(clock());
        for (UNumber y = 1; y <= times; y++) {
            data = Document::FromJSON(json_content);
            // Qentem::Engine::Match(json_content.Str, Document::getJsonExpres(), 0, json_content.Length);
        }
        took = (static_cast<UNumber>(clock()) - took);
        std::cout << String::FromNumber((static_cast<double>(took) / CLOCKS_PER_SEC), 2, 3, 3).Str << " ";

        std::cout << " Exporting... ";
        took = static_cast<UNumber>(clock());
        for (UNumber y = 1; y <= times; y++) {
            final = data.ToJSON();
        }
        took = (static_cast<UNumber>(clock()) - took);
        std::cout << String::FromNumber((static_cast<double>(took) / CLOCKS_PER_SEC), 2, 3, 3).Str << '\n';
    }

    if (BigJSON) {
        std::cout << "\n Big JSON Run\n";
        return true;
    }

    json_content = Qentem::Test::ReplaceNewLine(json_content.Str, json_content.Length, "");
    json_content = Qentem::Test::Replace(json_content.Str, json_content.Length, "\": ", "\":");

    if (final == json_content) {
        std::cout << "\n JSON looks good!\n";
        return true;
    }

    std::cout << "\n JSON is borken!\n\n";
    std::cout << "\n-File:\n";
    std::cout << json_content.Str;
    std::cout << "\n-End-\n";
    std::cout << "\n-ToJSON:\n";
    std::cout << final.Str;
    std::cout << "\n-End-\n";

    return false;
}

static Document getDocument() noexcept {
    Document data = Document();

    data["w"]    = "r\\\\";
    data["var1"] = "\"1\"";
    data["PP"]   = "gg";
    // String sss     = data["PP"].GetString();
    data["nu"]    = nullptr;
    data["bool"]  = false;
    data["bool2"] = true;
    data["&&"]    = 1000.0;

    data["var2"]       = "11";
    data["var2"]       = "2"; // Testing override .
    data["var3"]       = "3";
    data["var4"]       = "4";
    data["var5"]       = "5";
    data["num34"]      = Array<String>().Add("3").Add("4");
    data["var_string"] = "image";
    data["engine"]     = "Qentem";
    data["abc1"]       = Array<String>().Add("B").Add("C").Add("D").Add("A");
    data["abc2"]       = Array<String>().Add("E").Add("F").Add("A");
    data["numbers"]    = Array<double>().Add(0).Add(1).Add(2).Add(3).Add(4).Add(5);
    data["empty"]      = "";
    data["math"]       = "((2* (1 * 3)) + 1 - 4) + (((10 - 5) - 6 + ((1 + 1) + (1 + 1))) * (8 / 4 + 1)) - (1) - (-1) + 2";

    data["abc"]        = Document();
    data["abc"]["B"]   = "b";
    data["abc"]["@@"]  = 100.0;
    data["abc"]["A"]   = "a";
    data["abc"]["C"]   = "c";
    data["abc"]["D"]   = "d";
    data["abc"]["E"]   = Array<String>().Add("O").Add("K!");
    data["abc"]["A-Z"] = "ABCDEFGHIGKLMNOBQRST.....";

    data["multi"]              = Document();
    data["multi"]["arr1"]      = Document();
    data["multi"]["arr1"]["E"] = Array<String>().Add("O").Add("K!");
    data["multi"]["arr2"]      = Array<String>().Add("B").Add("C").Add("D").Add("A");
    data["multi"]["C"]         = "cool";

    return data;
}

static String readFile(const char *path) noexcept {
    std::ifstream file(path, std::ios::ate | std::ios::out);
    if (file.is_open()) {
        std::streampos size = file.tellg();
        file.seekg(0, std::ios::beg);

        String content(static_cast<UNumber>(size));
        content.Length = content.Capacity;

        file.read(content.Str, size);
        file.close();

        content[content.Length] = '\0';
        return content;
    }

    std::cout << '\n' << path << " does not exist!\n";

    return "";
}
