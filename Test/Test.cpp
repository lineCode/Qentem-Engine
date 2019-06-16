
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
static bool run_test(const String &name, const Array<TestBit> &bits, const bool Dump_express,
                     const bool break_on_err) noexcept;

int main() {
    bool pass = false;

    Array<TestBit> bits = Array<TestBit>();
    for (UNumber i = 0; i < 1; i++) {
        // Core Engine Tests
        bits = Qentem::Test::GetEngineBits();
        pass = run_test(L"Engine", bits, false, true);
        Qentem::Test::CleanBits(bits); // TODO: Implement a destructor

        if (!pass) {
            break;
        }

        // std::wcout << "\n///////////////////////////////////////////////\n";

        // Number Conversion Tests
        // pass = TestNumbersConv();

        // if (!pass) {
        //     break;
        // }

        std::wcout << "\n///////////////////////////////////////////////\n";

        // Arithmetic & logic Evaluation Tests
        bits = Qentem::Test::GetALUBits();
        pass = run_test(L"Arithmetic & Logic Evaluation", bits, false, true);

        if (!pass) {
            break;
        }

        std::wcout << "\n///////////////////////////////////////////////\n";

        // Template Tests
        bits = Qentem::Test::GetTemplateBits();
        pass = run_test(L"Template", bits, false, true);
    }

    if (pass) {
        std::wcout << L"\n ALL GOOD!" << '\n';
    }

    // std::getwchar();

    return 10;
}

static bool run_test(const String &name, const Array<TestBit> &bits, const bool Dump_express,
                     const bool break_on_err) noexcept {

    const UNumber times        = 10000; // 10000 To slow it down!
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

    for (UNumber i = start_at; i < bits.Size; i++) {
        count += 1;

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
            counter += 1;
            total += 1;

            if (pass) {
                ss += L' ';
            } else {
                ss += L'\n';
            }

            ss += String::FromNumber(count, 2) + L'-';
            ss += String::FromNumber(counter, 2) + L": ";

            if (pass) {
                ss += L"Pass";
            } else {
                ss += L"Fail";
            }

            ss += L" (Search: ";
            ss += String::FromNumber((static_cast<double>(search_ticks) / CLOCKS_PER_SEC), 2, 3) + L')';
            ss += L" (Parse: ";
            ss += String::FromNumber((static_cast<double>(parse_ticks) / CLOCKS_PER_SEC), 2, 3) + L")\n";

            if (!pass) {
                errors += 1;

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
    ss += String::FromNumber((static_cast<double>(total_search) / CLOCKS_PER_SEC), 2, 3);
    ss += L"s Total Parse: ";
    ss += String::FromNumber((static_cast<double>(total_parse) / CLOCKS_PER_SEC), 2, 3) + L"s\n";

    std::wcout << ss.Eject().Str;

    return pass;
}

static bool TestNumbersConv() noexcept {
    struct N_test {
        double num;
        String result;
    };

    Array<N_test> test;
    const UNumber times       = 1; // 4000000 To slow it down!
    UNumber       ticks       = 0;
    UNumber       total_ticks = 0;
    bool          pass        = false;
    ////////////////////////////////

    test.Add({0, L"0"}).Add({1, L"1"}).Add({2, L"2"}).Add({5, L"5"}).Add({9, L"9"});                       // 5
    test.Add({1000000, L"1000000"}).Add({11, L"11"}).Add({22, L"22"}).Add({55, L"55"}).Add({199, L"199"}); // 5
    test.Add({10.0, L"10"}).Add({11.00, L"11"}).Add({22.8, L"22.1"}).Add({55.0055, L"55.0055"});           // 4

    ////////////////////////////////

    std::wcout << L"\n #Number Conversion Tests:\n";

    for (UNumber i = 0; i < test.Size; i++) {
        std::wcout << L' ' << String::FromNumber((i + 1), 2).Str << L") ";

        ticks = static_cast<UNumber>(clock());

        for (size_t k = 0; k < times; k++) {
            pass = (test.Storage[i].result == String::FromNumber(test.Storage[i].num, 1, 10));
        }

        ticks = (static_cast<UNumber>(clock()) - ticks);
        total_ticks += ticks;

        if (pass) {
            std::wcout << L"Pass " << String::FromNumber((static_cast<double>(ticks) / CLOCKS_PER_SEC), 2, 3).Str
                       << L'\n';
        } else {
            std::wcout << L"Fail " << test.Storage[i].num << L" != " << String::FromNumber(test.Storage[i].num).Str
                       << L'\n';
            // return false;
        }
    }

    std::wcout << L"\n Total: " << String::FromNumber((static_cast<double>(total_ticks) / CLOCKS_PER_SEC), 2, 3).Str
               << L'\n';

    ////////////////////////////////
    return true;
}
