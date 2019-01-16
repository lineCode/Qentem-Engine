
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

using Qentem::Array;
using Qentem::String;

/**
 * @brief Search a content under specific Qentem expressions with limited range.
 *
 * @param content The text to match the give expressions in it.
 * @param exprs The Qentem expressions to be matched in the content.
 * @param from An index to start from.
 * @param length The ending index to stop at.
 * @param max The maximum index of to work with (to solve cutoff keywords).
 * @return An array of matched items.
 */
Array<Qentem::Engine::Match> Qentem::Engine::Search(const String &content, const Expressions &exprs, size_t from,
                                                    size_t to, size_t max) noexcept {
    if (to == 0) {
        to = content.Length;
    }

    // Feature: Add reversed search; instead of ++, use (1|-1) in a var to be implemented as: next_index+

    Array<Match> items = Array<Match>();
    if ((exprs.Size == 0) || (from >= to)) {
        // If something's wrong, return an empty array of type match.
        return items;
    }

    const size_t started  = from;
    size_t       counter  = 0;       // Index for counting.
    size_t       id       = 0;       // Expression's id.
    bool         LOCKED   = false;   // To keep matching the end of the current expression.
    bool         SPLITTED = false;   // To keep tracking a split match.
    Match        item     = Match(); // To store a single match for adding it to "items".

    // Seting the value of the current expression.
    Expression *ce = exprs[id++];

    size_t t_offset = 0; // Temp offset.
    size_t t_nest   = 0; // Temp variable for nesting matches.
    do {
        if (ce->SearchCB != nullptr) {
            item.Tag = counter = ce->SearchCB(content, *ce, from, t_offset);
        } else if (content.Str[from] == ce->Keyword.Str[counter]) {
            // Maintaining a copy of the original offset.
            t_offset = from;

            while (++counter < ce->Keyword.Length) {
                // Loop through every character.
                if (content.Str[++t_offset] != ce->Keyword.Str[counter]) {
                    // Mismatch.
                    counter = 0;
                    break;
                }
            }
        }

        if (counter != 0) {
            // If there is a match, reset counter to prevent a double match.
            counter = 0;
            if (item.Status != 0) {
                // If the match is on "OverDrive", then break.
                item.Status = 0;
                // Seting the length of the matched text.
                to = t_offset;
            } else if (!LOCKED) {
                // If the expression is not contented (single), then collect it's info; done matching.
                item.Offset  = from;
                item.OLength = ((t_offset - from) + 1);
                item.Expr    = ce;
                from         = t_offset;
                t_nest       = (from + 1);
            }

            if (ce->Tail != nullptr) {
                // If the match is not single, then lock it and update the current match
                // to the attached one (Tailed).
                LOCKED = true;
                ce     = ce->Tail;
            } else {
                // If it's a nesting expression, search again but inside the current match.
                if (item.Expr->NestExprs.Size != 0) {
                    // Start a new search inside the current one.
                    const Array<Match> nest_items =
                        Engine::Search(content, item.Expr->NestExprs, t_nest, from, ((max != 0) ? max : to));

                    if (nest_items.Size != 0) {
                        // Add the new items to the existing ones.
                        item.NestMatch.Add(nest_items);

                        if (max > to) {
                            // This is important to have the seearch look ahead of the limited length
                            // in order to find the entire currect match.
                            to          = max; // TO THE LIMIT
                            item.Status = 1;   // 1: OverDrive
                        }

                        // Seek to avoid having the same closing/ending keywork.
                        from = t_nest =
                            nest_items[(nest_items.Size - 1)].Offset + nest_items[(nest_items.Size - 1)].Length;

                        continue; // Not done matching, so move to the next wchar_t.
                    }
                }

                if (item.Expr->Tail != nullptr) {
                    // Set the length of closing keyword, if it multikeyword match.
                    item.CLength = ((t_offset - from) + 1);
                }

                item.Length = ((t_offset + 1) - item.Offset);

                if (item.Expr->SubExprs.Size != 0) {
                    item.SubMatch =
                        Engine::Search(content, item.Expr->SubExprs, item.Offset, (item.Offset + item.Length));
                }

                if ((Flags::SPLIT & item.Expr->Flag) != 0) {
                    Engine::Split(content, &item, &items, started, 0);
                    SPLITTED = true;
                } else {
                    items.Add(item);
                }

                // Prepare for the next match.
                item   = Match();
                LOCKED = false;
            }
        }

        if (!LOCKED) {
            // Switching to the next charrcter if all keywords have been tested.
            // Note: Could add  if ((id == exprs.Size) && (exprs.Size > 1)) but will case it to match tails; if it's one
            // expr.
            if (id == exprs.Size) {
                id = 0;
                ++from;
            }
            // Seting the next keyword for searching.
            ce = exprs[id++];
        } else {
            // If there is an ongoing match, then move to the next wchar_t.
            ++from;
        }

        // If it has gone too far (linked keyword is missing).
        if (from >= to) {
            if (!LOCKED) {
                break;
            }

            if (max > to) {
                // This is important to have the seearch look ahead of the limited length
                // in order to find the entire currect match.
                to          = max;
                item.Status = 1; // 1: OverDrive
                continue;
            }

            if (item.NestMatch.Size != 0) {
                // if it's a nested search... with matched items move every thing that has been found to the
                // main items' list, to avoid searching them again.
                items.Add(item.NestMatch);

                {
                    Match *m = &(item.NestMatch)[(item.NestMatch.Size - 1)];
                    // Seek the offset to where the last match ended.
                    from = m->Offset + m->Length;
                }

                if (from == to) {
                    break;
                }
            } else {
                // If it is a missmatch, go back to where it started and continue with the next expression if
                // possible.
                from = item.Offset;
            }

            // House cleaning...
            item.Status = 0;
            LOCKED      = false;
        }
    } while (true);

    if (SPLITTED) {
        Match *vis  = &(items[0]);
        vis->Offset = started;
        vis->Length = to - vis->Offset;
        Engine::Split(content, vis, &items, started, to);
    }

    if ((items.Size == 0) && ((Flags::POP & ce->Flag) != 0)) { //&& (ce->NestExprs.Size != 0)
        return Engine::Search(content, ce->NestExprs, started, to);
    }

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

    if ((length == 0) || (items.Size == 0)) {
        return content;
    }

    String rendered = L""; // Final content
    size_t id       = 0;

    Match *item;
    size_t from;
    size_t limit;
    do {
        // Current match
        item = &(items[id]);

        if ((Flags::IGNORE & item->Expr->Flag) != 0) {
            continue;
        }

        if ((Flags::COMPACT & item->Expr->Flag) == 0) {
            from  = item->Offset;
            limit = item->Length;
        } else {
            from  = (item->Offset + item->OLength);
            limit = (item->Length - (item->OLength + item->CLength));
        }

        // Adding any content that comes before...
        if ((offset < from)) {
            rendered += content.Part(offset, (from - offset));
        }

        if (item->Expr->ParseCB != nullptr) {
            if ((Flags::BUBBLE & item->Expr->Flag) != 0) {
                if (item->NestMatch.Size != 0) {
                    rendered +=
                        item->Expr->ParseCB(Engine::Parse(content, item->NestMatch, from, (from + limit)), *item);
                } else {
                    rendered += item->Expr->ParseCB(content.Part(from, limit), *item);
                }
            } else {
                rendered += item->Expr->ParseCB(content, *item);
            }
        } else {
            // Defaults to replace: it might be an empty string
            rendered += item->Expr->Replace;
        }

        offset = from + limit;
    } while (++id < items.Size);

    if (offset != 0) {
        if (offset < length) {
            // Adding the remaining of the text to the final rendered content.
            rendered += content.Part(offset, (length - offset));
        }

        return rendered;
    }

    // If there is no changes were made, return the original content.
    return content;
}

void Qentem::Engine::Split(const String &content, Match *item, Array<Match> *items, size_t from, size_t to) noexcept {
    Match tmp = Match();
    // tmp.Expr  = item.Expr;
    bool root = false;

    if (items->Size != 0) {
        for (size_t id = 0; id < items->Size; id++) {
            if ((Flags::SPLIT & (*items)[id].Expr->Flag) == 0) {
                tmp.NestMatch.Add((*items)[id]);
            }
        }

        root = ((Flags::SPLIT & (*items)[0].Expr->Flag) != 0);
        if (!root) {
            items->Size = 0;
        } else {
            items->Size = 1; // Shrinking; because we want one root in a split match.
        }
    }

    if (!root) {
        tmp.Offset = from;
        tmp.Length = (item->Offset - tmp.Offset);
        tmp.Tag    = item->Tag;

        if (item->Expr->NestExprs.Size != 0) {
            // Start a nest search inside the current one.
            tmp.NestMatch = Engine::Search(content, item->Expr->NestExprs, tmp.Offset, (tmp.Offset + tmp.Length));
        }

        item->NestMatch.Add(tmp);

        items->Add(*item);
    } else {
        if (to == 0) {
            to = item->Offset;
        }

        Match *ins = &((*items)[0].NestMatch[((*items)[0].NestMatch.Size - 1)]);

        // Adding a new item
        tmp.Offset = ins->Offset + ins->Length + (*items)[0].OLength + (*items)[0].CLength;
        tmp.Length = (to - tmp.Offset);
        tmp.Tag    = item->Tag;

        if (item->Expr->NestExprs.Size != 0) {
            tmp.NestMatch = Engine::Search(content, item->Expr->NestExprs, tmp.Offset, (tmp.Offset + tmp.Length));
        }

        (*items)[0].NestMatch.Add(tmp);
    }
}
