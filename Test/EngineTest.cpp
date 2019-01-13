
/**
 * Qentem Engine
 *
 * @brief     For testing Qentem Engine.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Test.hpp"
#include <iomanip>
#include <iostream>
#include <time.h>

using Qentem::Test::TestBit;

String q_format_number(float num, size_t min) noexcept {
    const String str  = Qentem::String::ToString(num);
    String       str2 = L"";

    for (size_t i = str.Length; i < min; i++) {
        str2 += L"0";
    }

    return (str2 + str);
}

void qentem_test_engine() {
    // size_t start_at = 11;
    // size_t child_at = 2;
    const size_t   start_at = 0;
    const size_t   child_at = 0;
    const size_t   times    = 1; // 10000
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
    for (size_t i = start_at; i < bits.Size(); i++) {
        child = child_at;
        count += 1;

        for (size_t t = child_at; t < bits[i].Content.Size(); t++) {
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

            std::wcout << (pass ? L" " : L"\n") << q_format_number((float)count, 2).Str << L"-"
                       << q_format_number((float)child, 2).Str << (pass ? L": Pass" : L": Fail") << L" (Search: "
                       << std::setprecision(3) << (((float)search_ticks) / CLOCKS_PER_SEC) << L")" << L" (Parse: "
                       << std::setprecision(3) << (((float)parse_ticks) / CLOCKS_PER_SEC) << L")\n";
            if (!pass) {
                errors += 1;
                std::wcout << L" -----------" << L" Start debug " << count << L"-" << child << L" -----" << L"\n"
                           << L"  Line:      " << bits[i].Line << L"\n"
                           << L"  Content:  \"" << bits[i].Content[t].Str << L"\"\n"
                           << L"  Rendered: \"" << rendered.Str << L"\"\n"
                           << L"  Expected: \"" << bits[i].Expected[t].Str << L"\"\n"
                           << L"  Matches:\n"
                           << Qentem::Engine::DumbMatches(bits[i].Content[t], matches, L"    ").Str
                           << L"  Expressions:\n"
                           << Qentem::Engine::DumbExpressions(bits[i].Exprs, L"    ").Str
                           << L"\n  ---------- End debug " << count << L"-" << child << L" -------" << L"\n";

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
    // // mem leak, and some other things...
    // for (size_t y = 0; y < 100000; y++) {
    //     qentem_test_engine();
    // }
    // std::getwchar();

    qentem_test_engine();

    return 1;
}
