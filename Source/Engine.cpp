
/**
 * Qentem Engine
 *
 * @brief     General purpose syntax parser.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Engine.hpp"

/**
 * @brief Search a content under specific Qentem expressions with limited range.
 *
 * @param content The text to match the give expressions in it.
 * @param exprs The Qentem expressions to be matched in the content.
 * @param offset An index to start from.
 * @param length The ending index to stop at.
 * @param max The maximum index of to work with (to solve cutoff keywords).
 * @return An array of matched items.
 */
Array<Qentem::Engine::Match> Qentem::Engine::Search(const String &content, const Expressions &exprs, size_t offset,
                                                    size_t length, size_t max) noexcept {
    if (length == 0) {
        length = content.Length;
    }

    // Feature: Add reversed search; instead of ++, use (1|-1) in a var to be implemented as: next_index+

    Array<Match> items = Array<Match>();
    if ((exprs.Size() == 0) || (offset >= length)) {
        // If something's wrong, return an empty array of type match.
        return items;
    }

    size_t counter = 0;       // Index for counting.
    size_t id      = 0;       // Expression's id.
    bool   LOCKED  = false;   // to keep matching the end of the current expression.
    Match  item    = Match(); // to store a single match for adding it to "items".

    // Seting the value of the current expression.
    Expression *ce = exprs[id++];

    size_t t_offset; // Temp offset.
    size_t t_nest;   // Temp variable for nesting matches.
    do {
        // Check every character (loop).
        if ((ce->Keyword.Length != 0) && (content.Str[offset] == ce->Keyword.Str[counter])) {
            // Maintaining a copy of the original offset.
            t_offset = offset;

            while (++counter < ce->Keyword.Length) { // Loop through every character.
                if (content.Str[++t_offset] != ce->Keyword.Str[counter]) {
                    // If there is a mismatch, then break (don't match the rest) and reset the counter.
                    counter = 0;
                    break;
                }
            }
        } else if (ce->SearchCB != nullptr) {
            // counter = ce->SearchCB(content, offset, t_offset);
            // Not implemented yet.
        }

        if (counter != 0) {
            // If there is a match, reset counter to prevent a float match.
            counter = 0;
            if (item.Options != 0) {
                // If the match is on "OverDrive", then break.
                item.Options = 0;
                // Seting the length of the matched text.
                length = t_offset;
            } else if (!LOCKED) {
                // If the expression is not contented (single), then collect it's info; done matching.
                item.Offset  = offset;
                item.OLength = ((t_offset - offset) + 1);
                item.Expr    = ce;
                offset       = t_offset;
                t_nest       = (offset + 1);
            }

            if (ce->Tail != nullptr) {
                // If the match is not single, then lock it and update the current match
                // to the attached one (Tailed).
                LOCKED = true;
                ce     = ce->Tail;
            } else {
                // If it's a nesting expression, search again but inside the current match.
                if (item.Expr->NestExprs.Size()) {
                    // Start a new search inside the current one.
                    const Array<Match> nest_items =
                        Engine::Search(content, item.Expr->NestExprs, t_nest, offset, ((max != 0) ? max : length));

                    if (nest_items.Size() != 0) {
                        // Add the new items to the existing ones.
                        item.NestMatch.Add(nest_items);

                        if (max > length) {
                            // This is important to have the seearch look ahead of the limited length
                            // in order to find the entire currect match.
                            length       = max;
                            item.Options = 1; // 1: OverDrive
                        }

                        // Seek to avoid having the same closing/ending keywork.
                        offset = t_nest =
                            nest_items[(nest_items.Size() - 1)].Offset + nest_items[(nest_items.Size() - 1)].Length;

                        continue; // Not done matching, so move to the next wchar_t.
                    }
                }

                if (item.Expr->Tail != nullptr) {
                    // Set the length of closing keyword, if it multikeyword match.
                    item.CLength = ((t_offset - offset) + 1);
                }

                item.Length = ((t_offset + 1) - item.Offset);

                if (item.Expr->SubExprs.Size() != 0) {
                    auto stt2 = content.Part(item.Offset, item.Length);

                    item.SubMatch =
                        Engine::Search(content, item.Expr->SubExprs, item.Offset, item.Offset + item.Length);
                }

                items.Add(item);

                // Prepare for the next match.
                item   = Match();
                LOCKED = false;
            }
        }

        if (!LOCKED) {
            // Switching to the next charrcter if all keywords have been tested.
            // Note: I could have  && (exprs.Size() > 1) but will case it to match tails; if iit one expr.
            if (id == exprs.Size()) {
                id = 0;
                ++offset;
            }
            // Seting the next keyword for searching.
            ce = exprs[id++];
        } else {
            // If there is an ongoing match, then move to the next wchar_t.
            ++offset;
        }

        // If it has gone too far (closing keyword is missing).
        if (offset >= length) {
            if (!LOCKED) {
                break;
            } else if (max > length) {
                // This is important to have the seearch look ahead of the limited length
                // in order to find the entire currect match.
                length       = max;
                item.Options = 1; // 1: OverDrive
                continue;
            }

            if (item.NestMatch.Size() != 0) {
                // if it's a nested search... with matched items move every thing that has been found to the
                // main items' list, to avoid searching them again.
                items.Add(item.NestMatch);

                {
                    Match *m = &(item.NestMatch)[(item.NestMatch.Size() - 1)];
                    // Seek the offset to where the last match ended.
                    offset = m->Offset + m->Length;
                }

                if (offset == length) {
                    break;
                }
            } else {
                // If it is a missmatch, go back to where it started and continue with the next expression if
                // possible.
                offset = item.Offset;
            }

            // House cleaning...
            item.Options = 0;
            LOCKED       = false;
        }
    } while (true);

    return items;
}

/**
 * @brief Parse the matched text (remove, replace, add, ...).
 *
 * @param content The text with the matches.
 * @param items The items that attended to be parsed.
 * @param offset An index to start from.
 * @param length The length of the content to work with.
 * @return The parsed (final) content.
 */
String Qentem::Engine::Parse(const String &content, const Array<Match> &items, size_t offset, size_t length) noexcept {
    if (length == 0) {
        length = content.Length;
    }

    if ((length == 0) || (items.Size() == 0)) {
        return content;
    }

    String rendered = L""; // Final content
    String block    = L""; // temp content
    size_t id       = 0;

    Match *item;
    size_t from;
    size_t limit;
    do {
        // Current match
        item = &(items[id]);

        // Adding any content that comes before...
        if ((offset < item->Offset)) {
            rendered += content.Part(offset, (item->Offset - offset));
        }

        if (item->Expr->ParseCB != nullptr) {
            if ((Flags::COMPLETE & item->Expr->Flag) == 0) {
                // Just the content inside the match. Default!
                from  = (item->Offset + item->OLength);
                limit = (item->Length - (item->OLength + item->CLength));
            } else {
                from  = item->Offset;
                limit = item->Length;
            }

            if ((item->NestMatch.Size() != 0) && ((Flags::BUBBLE & item->Expr->Flag) != 0)) {
                block = Engine::Parse(content, item->NestMatch, from, (from + limit));
            } else {
                block = content.Part(from, limit);
            }

            rendered += item->Expr->ParseCB(block, *item);

        } else {
            // Defaults to replace: it might be an empty string
            rendered += item->Expr->Replace;
        }

        offset = item->Offset + item->Length;
    } while (++id < items.Size());

    if (offset != 0) {
        if (offset < length) {
            // Adding the remaining of the text to the final rendered content.
            rendered += content.Part(offset, (length - offset));
        }

        return rendered;
    }

    // If there is no changes were made, then return the original content.
    return content;
}

/**
 * @brief Extract all matches form a given content into an array of strings.
 *
 * @param content a text to copy pmatch of.
 * @param items The match items of the content.
 * @return All matched items.
 */
Array<String> Qentem::Engine::Extract(const String &content, const Array<Match> &items) noexcept {
    Array<String> matches = Array<String>(items.Size());
    for (size_t i = 0; i < items.Size(); i++) {
        matches.Add(content.Part(items[i].Offset, items[i].Length));
    }

    return matches;
}

String Qentem::Engine::DumbExpressions(const Expressions &expres, const String offset, size_t index,
                                       Qentem::Engine::Expression *expr) noexcept {
    if (expres.Size() == 0) {
        return offset + L"No expres was found.";
    }

    String tree      = offset + L"(" + String::ToString((float)expres.Size()) + L") => {\n";
    String innoffset = L"    ";
    String l_offset  = offset + innoffset + innoffset;

    for (size_t i = index; i < expres.Size(); i++) {

        if (expres[i] == expr) {
            tree += offset + innoffset + L"[" + String::ToString((float)i) + L"]: " + L"This.\n";
            continue;
        }

        tree += offset + innoffset + L"[" + String::ToString((float)i) + L"]: \"" + expres[i]->Keyword + L"\" => [\n";
        tree += l_offset + L"Flags: (" + String::ToString((float)(expres[i]->Flag)) + L")";

        if ((expres[i]->Flag & Flags::COMPLETE) != 0) {
            tree += L" COMPLETE";
        }

        if ((expres[i]->Flag & Flags::BUBBLE) != 0) {
            tree += l_offset + L" BUBBLE";
        }

        if ((expres[i]->Flag & Flags::OVERLOOK) != 0) {
            tree += l_offset + L" OVERLOOK";
        }
        tree += L"\n";

        tree += l_offset + L"Replace: \"" + expres[i]->Replace + L"\"\n";
        tree += l_offset + L"SearchCB: " +
                ((expres[i]->SearchCB != nullptr) ? String::ToString((float)((size_t)(expres[i]->SearchCB))) : L"N/A");
        tree += L"\n";
        tree += l_offset + L"ParseCB: " +
                ((expres[i]->ParseCB != nullptr) ? String::ToString((float)((size_t)(expres[i]->ParseCB))) : L"N/A");
        tree += L"\n";

        if (expres[i]->Tail != nullptr) {
            tree += l_offset + L"Tail:\n";
            tree += Engine::DumbExpressions(Expressions().Add(expres[i]->Tail), innoffset + l_offset, 0, expres[i]);
        }

        if (expres[i]->NestExprs.Size() != 0) {
            tree += l_offset + L"NestExprs:\n";
            tree += Engine::DumbExpressions(expres[i]->NestExprs, innoffset + l_offset, 0, expres[i]);
        }

        if (expres[i]->SubExprs.Size() != 0) {
            tree += l_offset + L"SubExprs:\n";
            tree += Engine::DumbExpressions(expres[i]->SubExprs, innoffset + l_offset, 0, expres[i]);
        }

        // if (expres[i]->SplitExprs.Size() != 0) {
        //     tree += l_offset + L"SplitExprs:\n";
        //     tree += Engine::DumbExpressions(expres[i]->SplitExprs, innoffset + l_offset, 0, expres[i]);
        // }

        tree += l_offset + L"]\n";
    }

    return tree + offset + L"}\n";
}

String Qentem::Engine::DumbMatches(const String &content, const Array<Match> &matches, const String offset,
                                   size_t index) noexcept {
    if (matches.Size() == 0) {
        return offset + L"No match was found.";
    }

    Array<String> items = Engine::Extract(content, matches);

    String innoffset = L"    ";
    String tree      = offset + L"(" + String::ToString((float)(matches.Size())) + L") => {\n";

    for (size_t i = index; i < items.Size(); i++) {
        tree += innoffset + offset + L"[" + String::ToString((float)i) + L"]: \"" + items[i] + L"\"\n";

        if (matches[i].NestMatch.Size() != 0) {
            tree += innoffset + offset + L"-NestMatch:\n";
            tree += Engine::DumbMatches(content, matches[i].NestMatch, innoffset + innoffset + offset, 0);
        }

        if (matches[i].SubMatch.Size() != 0) {
            tree += innoffset + offset + L"-SubMatch:\n";
            tree += Engine::DumbMatches(content, matches[i].SubMatch, innoffset + innoffset + offset, 0);
        }
    }

    return tree + offset + L"}\n";
}
