
/**
 * Qentem Engine Extended functions
 *
 * @brief     For implenting other patent.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Addon/Extend.hpp"

size_t Qentem::Extend::RegexOR(const String &content, const Expression &expr, size_t &started, size_t &ended) noexcept {
    size_t counter = 0;
    size_t tag     = 1;

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
                return tag;
            }
        }

        while (counter < expr.Keyword.Length) {
            if (expr.Keyword.Str[++counter] == L'|') {
                tag += 1;
                break;
            }
        }
    }

    return 0;
}
