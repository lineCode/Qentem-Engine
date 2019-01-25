
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
