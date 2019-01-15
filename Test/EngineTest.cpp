
/**
 * Qentem Engine
 *
 * @brief     For testing Qentem Engine.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include <time.h>
#include <iostream>
#include "Addon/Test.hpp"
#include "Addon/ALU.hpp"

using Qentem::Test::TestBit;

void qentem_test_engine() {
    // size_t start_at = 0;
    // size_t child_at = 0;
    const size_t   start_at = 0;
    const size_t   child_at = 0;
    const size_t   times    = 5000; // To slow it down!!!
    size_t         search_ticks;
    size_t         parse_ticks;
    size_t         child  = 0;
    size_t         errors = 0;
    size_t         total  = 0;
    size_t         count  = start_at;
    Array<TestBit> bits   = Qentem::Test::GetBits();

    bool       pass         = false;
    const bool break_on_err = true;
    std::wcout << L"\n#Engine::Search&Parse():\n";
    for (size_t i = start_at; i < bits.Size; i++) {
        child = child_at;
        count += 1;

        for (size_t t = child_at; t < bits[i].Content.Size; t++) {
            Array<Qentem::Engine::Match> matches;
            search_ticks = clock();
            for (size_t x = 0; x < times; x++) {
                matches = Qentem::Engine::Search(bits[i].Content[t], bits[i].Exprs);
            }
            search_ticks = (clock() - search_ticks);

            String rendered = L"";
            parse_ticks     = clock();
            for (size_t y = 0; y < times; y++) {
                rendered = Qentem::Engine::Parse(bits[i].Content[t], matches);
            }
            parse_ticks = (clock() - parse_ticks);

            pass = (rendered == bits[i].Expected[t]);
            child += 1;
            total += 1;

            std::wcout << (pass ? L" " : L"\n") << Qentem::String::ToString((float)count, 2).Str << L"-"
                       << Qentem::String::ToString((float)child, 2).Str << (pass ? L": Pass" : L": Fail")
                       << L" (Search: " << Qentem::String::ToString((((float)search_ticks) / CLOCKS_PER_SEC), 2, 3).Str
                       << L")" << L" (Parse: "
                       << Qentem::String::ToString((((float)parse_ticks) / CLOCKS_PER_SEC), 2, 3).Str << L")\n";
            if (!pass) {
                errors += 1;
                std::wcout << L" -----------" << L" Start debug " << count << L"-" << child << L" -----" << L"\n"
                           << L"  Line:      " << bits[i].Line << L"\n"
                           << L"  Content:  \"" << bits[i].Content[t].Str << L"\"\n"
                           << L"  Rendered: \"" << rendered.Str << L"\"\n"
                           << L"  Expected: \"" << bits[i].Expected[t].Str << L"\"\n"
                           << L"  Matches:\n"
                           << Qentem::Test::DumbMatches(bits[i].Content[t], matches, L"    ").Str << L"  Expressions:\n"
                           << Qentem::Test::DumbExpressions(bits[i].Exprs, L"    ").Str << L"\n  ---------- End debug "
                           << count << L"-" << child << L" -------" << L"\n";

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
        std::wcout << L"\nAll good! (" << total << L" Tests)\n";
    } else {
        std::wcout << L"\nFAILED: " << errors << L" out of " << total << L"\n";
    }
}

int main() {
    // // for checking mem leaks, and other things...
    // for (size_t y = 0; y < 100000; y++) {
    //     qentem_test_engine();
    // }
    // std::getwchar();

    // float nnn = 0;
    // Qentem::String::ToNumber(L"  98.24   ", nnn);
    // auto content = L"";
    // float  aa      = Qentem::ALU::Execute(content);
    qentem_test_engine();

    return 1;
}
