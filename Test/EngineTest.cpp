
/**
 * Qentem Engine Test
 *
 * @brief     For testing Qentem Engine.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include <Extension/Test.hpp>
#include <ctime>
#include <iostream>

void qentem_test_engine(bool dumb_express, bool break_on_err) noexcept {
    Qentem::Array<Qentem::Test::TestBit> &&bits = Qentem::Test::GetBits();

    const UNumber times        = 10000; // 10000 To slow it down!
    const UNumber start_at     = 0;
    UNumber       counter      = 0;
    UNumber       errors       = 0;
    UNumber       total        = 0;
    UNumber       total_search = 0;
    UNumber       search_ticks = 0;
    UNumber       total_parse  = 0;
    UNumber       parse_ticks  = 0;

    Qentem::Array<Qentem::Engine::Match> matches;

    UNumber count = start_at;
    bool    pass  = false;

    std::wcout << L"\n #Engine::Search&Parse():\n";
    for (UNumber i = start_at; i < bits.Size; i++) {
        count += 1;

        for (UNumber t = counter; t < bits.Storage[i].Content.Size; t++) {
            search_ticks = static_cast<UNumber>(clock());
            for (UNumber x = 0; x < times; x++) {
                matches = Qentem::Engine::Search(bits.Storage[i].Content.Storage[t], bits.Storage[i].Exprs);
            }
            search_ticks = (static_cast<UNumber>(clock()) - search_ticks);
            total_search += search_ticks;

            Qentem::String rendered;
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
                std::wcout << L" ";
            } else {
                std::wcout << L"\n";
            }

            std::wcout << Qentem::String::FromNumber(count, 2).Str << L"-" << Qentem::String::FromNumber(counter, 2).Str;

            if (pass) {
                std::wcout << L": Pass";
            } else {
                std::wcout << L": Fail";
            }

            std::wcout << L" (Search: "
                       << Qentem::String::FromNumber((static_cast<double>(search_ticks) / CLOCKS_PER_SEC), 2, 3).Str
                       << L")" << L" (Parse: "
                       << Qentem::String::FromNumber((static_cast<double>(parse_ticks) / CLOCKS_PER_SEC), 2, 3).Str
                       << L")\n";

            if (!pass) {
                errors += 1;
                std::wcout << L" -----------" << L" Start debug " << count << L"-" << counter << L" -----" << L"\n"
                           << L"  Line:      " << bits.Storage[i].Line << L"\n"
                           << L"  Content:  \"" << bits.Storage[i].Content.Storage[t].Str << L"\"\n"
                           << L"  Rendered: \"" << rendered.Str << L"\"\n"
                           << L"  Expected: \"" << bits.Storage[i].Expected.Storage[t].Str << L"\"\n"
                           << L"  Matches:\n"
                           << Qentem::Test::DumbMatches(bits.Storage[i].Content.Storage[t], matches, L"    ").Str;

                if (dumb_express) {
                    std::wcout << L"  Expressions:\n"
                               << Qentem::Test::DumbExpressions(bits.Storage[i].Exprs, L"    ").Str;
                }

                std::wcout << L"\n  ---------- End debug " << count << L"-" << counter << L" -------" << L"\n";

                if (break_on_err) {
                    break;
                }
            }
        }

        counter = 0;

        if (!pass && break_on_err) {
            break;
        }
    }

    Qentem::Test::CleanBits(bits);

    if (errors == 0) {
        std::wcout << L"\n Operational (Total Tests: " << total << L")";
    } else {
        std::wcout << L"\n Broken (Failed: " << errors << L",  out of: " << total << L")";
    }

    std::wcout << L", Total Search: "
               << Qentem::String::FromNumber((static_cast<double>(total_search) / CLOCKS_PER_SEC), 2, 3).Str
               << L"s Total Parse: "
               << Qentem::String::FromNumber((static_cast<double>(total_parse) / CLOCKS_PER_SEC), 2, 3).Str << L"s\n";
}

int main() {
    // for (UNumber y = 0; y < 50; y++) {
    //     qentem_test_engine(false, false);
    // }

    qentem_test_engine(false, false);

    // std::getwchar();

    return 1;
}
