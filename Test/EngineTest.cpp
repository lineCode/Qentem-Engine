
/**
 * Qentem Engine Test
 *
 * @brief     For testing Qentem Engine.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Addon/Test.hpp"
#include <ctime>
#include <iostream>

using Qentem::Array;
using Qentem::String;
using Qentem::Test::TestBit;

void qentem_test_engine(bool dumb_express, bool break_on_err) {
    Array<TestBit> bits     = Qentem::Test::GetBits();
    const UNumber  times    = 1; // 100000 To slow it down!
    const UNumber  start_at = 0;
    const UNumber  child_at = 0;
    UNumber        child    = 0;
    UNumber        errors   = 0;
    UNumber        total    = 0;
    UNumber        parse_ticks;
    UNumber        search_ticks;

    UNumber count = start_at;
    bool    pass  = false;
    std::wcout << L"\n #Engine::Search&Parse():\n";
    for (UNumber i = start_at; i < bits.Size; i++) {
        child = child_at;
        count += 1;

        for (UNumber t = child_at; t < bits[i].Content.Size; t++) {
            Array<Qentem::Engine::Match> matches;
            search_ticks = clock();
            for (UNumber x = 0; x < times; x++) {
                matches = Qentem::Engine::Search(bits[i].Content[t], bits[i].Exprs);
            }
            search_ticks = (clock() - search_ticks);

            String rendered = L"";
            parse_ticks     = clock();
            for (UNumber y = 0; y < times; y++) {
                rendered = Qentem::Engine::Parse(bits[i].Content[t], matches);
            }
            parse_ticks = (clock() - parse_ticks);

            pass = (rendered == bits[i].Expected[t]);
            child += 1;
            total += 1;

            if (pass) {
                std::wcout << L" ";
            } else {
                std::wcout << L"\n";
            }

            std::wcout << Qentem::String::FromNumber(static_cast<double>(count), 2).Str << L"-"
                       << Qentem::String::FromNumber(static_cast<double>(child), 2).Str;

            if (pass) {
                std::wcout << L": Pass";
            } else {
                std::wcout << L": Fail";
            }

            std::wcout << L" (Search: "
                       << Qentem::String::FromNumber(((static_cast<double>(search_ticks)) / CLOCKS_PER_SEC), 2, 3).Str
                       << L")" << L" (Parse: "
                       << Qentem::String::FromNumber(((static_cast<double>(parse_ticks)) / CLOCKS_PER_SEC), 2, 3).Str
                       << L")\n";

            if (!pass) {
                errors += 1;
                std::wcout << L" -----------" << L" Start debug " << count << L"-" << child << L" -----" << L"\n"
                           << L"  Line:      " << bits[i].Line << L"\n"
                           << L"  Content:  \"" << bits[i].Content[t].Str << L"\"\n"
                           << L"  Rendered: \"" << rendered.Str << L"\"\n"
                           << L"  Expected: \"" << bits[i].Expected[t].Str << L"\"\n"
                           << L"  Matches:\n"
                           << Qentem::Test::DumbMatches(bits[i].Content[t], matches, L"    ").Str;

                if (dumb_express) {
                    std::wcout << L"  Expressions:\n" << Qentem::Test::DumbExpressions(bits[i].Exprs, L"    ").Str;
                }

                std::wcout << L"\n  ---------- End debug " << count << L"-" << child << L" -------" << L"\n";

                if (break_on_err) {
                    break;
                }
            }
        }

        if (!pass && break_on_err) {
            break;
        }
    }

    Qentem::Test::CleanBits(bits);

    if (errors == 0) {
        std::wcout << L"\n Operational (Total Tests: " << total << L")\n";
    } else {
        std::wcout << L"\n Broken: " << errors << L" out of " << total << L"\n";
    }
}

int main() {
    // for (UNumber y = 0; y < 1000; y++) {
    //     qentem_test_engine(false, false);
    // }

    qentem_test_engine(false, true);

    // std::getwchar();

    return 1;
}
