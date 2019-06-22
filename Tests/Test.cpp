
/**
 * Qentem Engine Test
 *
 * @brief     For testing Qentem Engine.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include <Extension/ALU.hpp>
#include <Extension/Test.hpp>
#include <ctime>
#include <iostream>

using Qentem::Array;
using Qentem::String;
using Qentem::StringStream;
using Qentem::Engine::Match;
using Qentem::Test::TestBit;

static bool TestNumbersConv() noexcept;
static bool run_tests(const String &name, const Array<TestBit> &bits, const bool Dump_express,
                      const bool break_on_err) noexcept;

int main() {
    bool          pass  = false;
    const UNumber times = 1;

    const bool TestEngine   = true;
    const bool NumbersConv  = true;
    const bool TestALU      = true;
    const bool TestTemplate = true;

    Array<TestBit> bits = Array<TestBit>();
    for (UNumber i = 0; i < times; i++) {
        if (TestEngine) {
            // Core Engine Tests
            bits = Qentem::Test::GetEngineBits();
            pass = run_tests(L"Engine", bits, false, true);
            Qentem::Test::CleanBits(bits); // TODO: Implement a destructor

            if (!pass) {
                break;
            }

            std::wcout << "\n///////////////////////////////////////////////\n";
        }

        if (NumbersConv) {
            // Number Conversion Tests
            pass = TestNumbersConv();

            if (!pass) {
                break;
            }

            std::wcout << "\n///////////////////////////////////////////////\n";
        }

        if (TestALU) {
            // Arithmetic & logic Evaluation Tests
            bits = Qentem::Test::GetALUBits();
            pass = run_tests(L"Arithmetic & Logic Evaluation", bits, false, true);

            if (!pass) {
                break;
            }

            std::wcout << "\n///////////////////////////////////////////////\n";
        }

        if (TestTemplate) {
            // Template Tests
            bits = Qentem::Test::GetTemplateBits();
            pass = run_tests(L"Template", bits, false, true);
        }
    }

    if (pass) {
        std::wcout << L"\n ALL GOOD!\n\n";
    }

    // std::getwchar();

    return 10;
}

static bool run_tests(const String &name, const Array<TestBit> &bits, const bool Dump_express,
                      const bool break_on_err) noexcept {

    const UNumber times        = 1; // 10000 To slow it down!
    const UNumber start_at     = 0;
    UNumber       counter      = 0;
    UNumber       errors       = 0;
    UNumber       total        = 0;
    UNumber       total_search = 0;
    UNumber       search_ticks = 0;
    UNumber       total_parse  = 0;
    UNumber       parse_ticks  = 0;
    UNumber       count        = start_at;
    bool          pass         = false;

    StringStream ss;
    Array<Match> matches;

    ss += L"\n #";
    ss += name + L" Tests:\n";

    if (start_at != 0) {
        ss += L"\n Starting at ";
        ss += String::FromNumber(start_at, 2);
        ss += L"\n\n";
    }

    if (counter != 0) {
        ss += L"\n Counter at ";
        ss += String::FromNumber(counter, 2);
        ss += L"\n\n";
    }

    for (UNumber i = start_at; i < bits.Size; i++) {
        ++count;

        if (bits.Storage[i].Expected.Size != bits.Storage[i].Content.Size) {

            std::wcout << L"Check Expected & Content Size @" << String::FromNumber(bits.Storage[i].Line).Str << L'\n';
            return false;
        }

        for (UNumber t = counter; t < bits.Storage[i].Content.Size; t++) {
            search_ticks = static_cast<UNumber>(clock());
            for (UNumber x = 0; x < times; x++) {
                matches = Qentem::Engine::Search(bits.Storage[i].Content.Storage[t], bits.Storage[i].Exprs);
            }
            search_ticks = (static_cast<UNumber>(clock()) - search_ticks);
            total_search += search_ticks;

            String rendered;
            parse_ticks = static_cast<UNumber>(clock());
            for (UNumber y = 0; y < times; y++) {
                rendered = Qentem::Engine::Parse(bits.Storage[i].Content.Storage[t], matches);
            }
            parse_ticks = (static_cast<UNumber>(clock()) - parse_ticks);
            total_parse += parse_ticks;

            pass = (rendered == bits.Storage[i].Expected.Storage[t]);
            ++counter;
            ++total;

            if (pass) {
                ss += L' ';
            } else {
                ss += L"\n ";
            }

            ss += String::FromNumber(count, 2) + L'-';
            ss += String::FromNumber(counter, 2) + L": ";

            if (pass) {
                ss += L"Pass";
            } else {
                ss += L"Fail";
            }

            ss += L" (Search: ";
            ss += String::FromNumber((static_cast<double>(search_ticks) / CLOCKS_PER_SEC), 2, 3, 3) + L')';
            ss += L" (Parse: ";
            ss += String::FromNumber((static_cast<double>(parse_ticks) / CLOCKS_PER_SEC), 2, 3, 3) + L")\n";

            if (!pass) {
                ++errors;

                ss += L" ----------- Start debug ";
                ss += String::FromNumber(count) + L'-';
                ss += String::FromNumber(counter);
                ss += L" -----\n  Line:      ";
                ss += String::FromNumber(bits.Storage[i].Line) + L'\n';
                ss += L"  Content:  ";
                ss += Qentem::Test::ReplaceNewLine(bits.Storage[i].Content.Storage[t]) + L'\n';
                ss += L"  Rendered: \"";
                ss += Qentem::Test::ReplaceNewLine(rendered) + L"\"\n";
                ss += L"  Expected: \"";
                ss += Qentem::Test::ReplaceNewLine(bits.Storage[i].Expected.Storage[t]) + L"\"\n";
                ss += L"  Matches:\n";
                ss += Qentem::Test::DumpMatches(bits.Storage[i].Content.Storage[t], matches, L"    ");

                if (Dump_express) {
                    ss += L"  Expressions: ";
                    ss += Qentem::Test::DumpExpressions(bits.Storage[i].Exprs, L"    ");
                }

                ss += L"\n  ---------- End debug ";
                ss += String::FromNumber(count) + L'-';
                ss += String::FromNumber(counter) + L" -------\n";

                if (break_on_err) {
                    break;
                }
            }

            std::wcout << ss.Eject().Str;
        }

        counter = 0;

        if (!pass && break_on_err) {
            break;
        }
    }

    ss += "\n ";
    ss += name + L" is";
    if (errors == 0) {
        ss += L" operational! (Total Tests: ";
        ss += String::FromNumber(total) + L')';
    } else {
        ss += L" broken! (Failed: ";
        ss += String::FromNumber(errors) + L",  out of: ";
        ss += String::FromNumber(total) + L')';
    }

    ss += L", Total Search: ";
    ss += String::FromNumber((static_cast<double>(total_search) / CLOCKS_PER_SEC), 2, 3, 3);
    ss += L"s Total Parse: ";
    ss += String::FromNumber((static_cast<double>(total_parse) / CLOCKS_PER_SEC), 2, 3, 3) + L"s\n";

    std::wcout << ss.Eject().Str;

    return (errors == 0);
}

static bool TestNumbersConv() noexcept {
    struct N_test {
        double num;
        String result;
    };

    Array<N_test> test;
    const UNumber times       = 1; // 100000 To slow it down!
    UNumber       ticks       = 0;
    UNumber       total_ticks = 0;
    bool          pass        = false;
    ////////////////////////////////

    test.Add({2.0000000000009, L"2.0000000000009"});
    test.Add({3.99999999999999, L"3.99999999999999"});
    test.Add({99.1005099, L"99.1005099"});

    test.Add({871.080055555, L"871.080055555"});
    test.Add({999.1000099, L"999.1000099"});
    test.Add({9.1000099, L"9.1000099"});
    test.Add({2.30000000000001, L"2.30000000000001"});
    test.Add({66666.30000400000001, L"66666.300004"});
    test.Add({22.3000000000006, L"22.3000000000006"});
    test.Add({22.30000000000006, L"22.30000000000006"});
    test.Add({22.300000000000006, L"22.3"});

    test.Add({9788871.080055555, L"9788871.080055555"});
    test.Add({6666666.30000400000001, L"6666666.300004"});

    test.Add({22.300000000000055, L"22.30000000000005"});
    test.Add({22.300000000000059, L"22.30000000000006"});

    test.Add({71.080055555, L"71.080055555"});
    test.Add({1.000055555, L"1.000055555"});

    test.Add({22.300000000000054, L"22.30000000000005"});
    test.Add({2.00000000000001, L"2.00000000000001"});
    test.Add({2.000000000000001, L"2.000000000000001"});

    test.Add({222.300000000000055, L"222.3"});
    test.Add({0.11111111111111, L"0.11111111111111"});
    test.Add({2.00000000000001, L"2.00000000000001"});
    test.Add({22.00000000000005, L"22.00000000000005"});
    test.Add({22.300000000000055, L"22.30000000000005"});

    test.Add({2.300000000000055, L"2.300000000000055"});
    test.Add({2.000000000000015, L"2.000000000000015"});
    test.Add({22.000055555, L"22.000055555"});
    test.Add({2.00000000000051, L"2.00000000000051"});
    test.Add({2222.000000000000055, L"2222"});
    test.Add({2222.0000000000001, L"2222"});

    test.Add({2.00000000000001, L"2.00000000000001"});

    test.Add({222.00000000000001, L"222"});
    test.Add({333.99999999999999, L"334"});
    test.Add({22.87, L"22.87"});
    test.Add({-0.788065000000079, L"-0.788065000000079"});
    test.Add({9.99999999999901, L"9.99999999999901"});
    test.Add({3.99999999999909, L"3.99999999999909"});
    test.Add({55.0055, L"55.0055"});
    test.Add({55.0000055, L"55.0000055"});
    test.Add({0, L"0"}).Add({1, L"1"}).Add({-2, L"-2"}).Add({5, L"5"}).Add({9, L"9"}).Add({3.1, L"3.1"});  // 5
    test.Add({1000000, L"1000000"}).Add({11, L"11"}).Add({22, L"22"}).Add({55, L"55"}).Add({199, L"199"}); // 5
    test.Add({10.0, L"10"}).Add({11.00, L"11"}).Add({-22.87, L"-22.87"}).Add({55.0055, L"55.0055"});       // 4

    ////////////////////////////////

    std::wcout << L"\n #Number Conversion Tests:\n";

    for (UNumber i = 0; i < test.Size; i++) {
        std::wcout << L' ' << String::FromNumber((i + 1), 2).Str << L") ";

        ticks = static_cast<UNumber>(clock());

        for (size_t k = 0; k < times; k++) {
            pass = (test.Storage[i].result == String::FromNumber(test.Storage[i].num));
        }

        ticks = (static_cast<UNumber>(clock()) - ticks);
        total_ticks += ticks;

        if (pass) {
            std::wcout << L"Pass " << String::FromNumber((static_cast<double>(ticks) / CLOCKS_PER_SEC), 2, 3, 3).Str
                       << L'\n';
        } else {
            std::wcout << L"Fail " << test.Storage[i].result.Str << L" != "
                       << String::FromNumber(test.Storage[i].num).Str << L'\n';
            std::wcout << L"\n Math is wrong not me.\n";

            return false;
        }
    }

    std::wcout << L"\n Math looks good!";
    std::wcout << L" Total: " << String::FromNumber((static_cast<double>(total_ticks) / CLOCKS_PER_SEC), 2, 3, 3).Str
               << L'\n';

    ////////////////////////////////
    return true;
}
