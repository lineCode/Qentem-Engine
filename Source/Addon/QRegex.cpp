
/**
 * Qentem Engine Regex extintion
 *
 * @brief     For implenting other patent.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Addon/QRegex.hpp"

using Qentem::Engine::Expression;
using Qentem::Engine::Expressions;
using Qentem::Engine::String;

UNumber Qentem::QRegex::OR(const String &content, const Expression &expr, Match *item, UNumber &started, UNumber &ended,
                           UNumber limit) noexcept {
    UNumber counter = 0;
    item->Tag       = 1;

    for (; counter < expr.Keyword.Length; counter++) {
        if (content.Str[started] == expr.Keyword.Str[counter]) {
            // Don't change where it was started.
            ended = started;

            while ((++counter < expr.Keyword.Length) && (expr.Keyword.Str[counter] != L'|')) {
                if (content.Str[++ended] != expr.Keyword.Str[counter]) {
                    // Mismatch.
                    counter = 0;
                    break;
                }
            }

            if (counter != 0) {
                return item->Tag;
            }
        }

        while (counter < expr.Keyword.Length) {
            if (expr.Keyword.Str[++counter] == L'|') {
                item->Tag += 1;
                break;
            }
        }
    }

    return 0;
}

// Short hand replace
String Qentem::QRegex::Replace(const String &content, const String &find, const String &replace) noexcept {
    Expression find_key;
    find_key.Keyword  = find;
    find_key.SearchCB = &(QRegex::OR);
    find_key.Replace  = replace;

    return Engine::Parse(content, Engine::Search(content, Expressions().Add(&find_key)));
}

// Short hand replace
String Qentem::QRegex::Replace(const String &content, const String &find, const Array<String> &replace) noexcept {
    Expression findexpr;
    findexpr.Keyword  = find;
    findexpr.SearchCB = &(QRegex::OR);
    findexpr.Replace  = replace[0]; // TODO:: Needs more work...

    // findexpr.ParseCB = &;

    return Engine::Parse(content, Engine::Search(content, Expressions().Add(&findexpr)));
}
