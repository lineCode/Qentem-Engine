
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
using Qentem::Engine::Match;

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
Array<Match> Qentem::Engine::Search(const String &content, const Expressions &exprs, size_t from, size_t to, size_t max,
                                    size_t level) noexcept {
    if (to == 0) {
        to = from + content.Length;
    }

    // Feature: Add reversed search; instead of ++, use (1|-1) in a var to be implemented as: next_index+

    Array<Match> items = Array<Match>();
    if ((exprs.Size == 0) || (from >= to)) {
        // If something's wrong, return an empty array of type match.
        return items;
    }

    const size_t started = from;
    size_t       counter = 0;       // Index for counting.
    size_t       id      = 0;       // Expression's id.
    bool         LOCKED  = false;   // To keep matching the end of the current expression.
    bool         SPLIT   = false;   // To keep tracking a split match.
    Match        item    = Match(); // To store a single match for adding it to "items".
    Array<Match> nest_items;
    // Seting the value of the current expression.
    Expression *ce = exprs[id++];

    size_t t_offset = 0; // Temp offset.
    size_t t_nest   = 0; // Temp variable for nesting matches.
    do {
        if (ce->SearchCB != nullptr) {
            item.Tag = counter = ce->SearchCB(content, *ce, from, t_offset);
        } else if (content.Str[from] == ce->Keyword.Str[counter]) {
            // Don't change where it was started.
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
                    nest_items = Engine::Search(content, item.Expr->NestExprs, t_nest, from, ((max != 0) ? max : to),
                                                (level + 1));

                    if (nest_items.Size != 0) {
                        if (max > to) {
                            // This is important to have the seearch look ahead of the limited length
                            // in order to find the entire currect match.
                            to          = max; // TO THE LIMIT!
                            item.Status = 1;   // 1: OverDrive // Could use bool, then deal with mem alinement
                        }

                        // Seek to avoid having the same closing/ending keywork matched again.
                        from = t_nest =
                            nest_items[(nest_items.Size - 1)].Offset + nest_items[(nest_items.Size - 1)].Length;

                        item.NestMatch.Add(nest_items);

                        continue; // Not done matching, so move to the next char.
                    }
                }

                if (item.Expr->Tail != nullptr) {
                    // Sets the length of the closing keyword, if it's multi-keyword match.
                    item.CLength = ((t_offset - from) + 1);
                }

                item.Length = ((t_offset + 1) - item.Offset);

                if ((level == 0) && ((Flags::NESTSPLIT & item.Expr->Flag) != 0)) {
                    Engine::Split(content, item.Expr, item.NestMatch, (item.Offset + item.OLength),
                                  ((item.Offset + item.Length) - (item.CLength)));
                }

                if (item.Expr->SubExprs.Size != 0) {
                    item.SubMatch =
                        Engine::Search(content, item.Expr->SubExprs, item.Offset, (item.Offset + item.Length));
                }

                if ((Flags::SPLIT & item.Expr->Flag) != 0) {
                    SPLIT = true;
                }

                items.Add(item);

                if ((Flags::ONCE & item.Expr->Flag) != 0) {
                    return items;
                }

                // Prepare for the next match.
                item   = Match();
                LOCKED = false;
            }
        }

        if (!LOCKED) {
            // Switching to the next charrcter if all keywords have been tested.
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

    if (items.Size == 0) {
        if ((Flags::POP & ce->Flag) != 0) { //&& (ce->NestExprs.Size != 0)
            return Engine::Search(content, ce->NestExprs, started, to);
        }
    } else if ((level == 0) && SPLIT) {
        Engine::Split(content, nullptr, items, started, to);
    }

    // Friday, January 18, 2019
    // AlJumaa, Jamada El Oula 12, 1440

    return items;
}

void Qentem::Engine::Split(const String &content, Expression *expr, Array<Match> &items, size_t from,
                           size_t to) noexcept {
    if (items.Size > 0) {
        if (items.Size == 1) {
            if ((Flags::NESTSPLIT & items[0].Expr->Flag) != 0) {
                Engine::Split(content, items[0].Expr, items[0].NestMatch, (items[0].Offset + items[0].OLength),
                              ((items[0].Offset + items[0].Length) - (items[0].CLength)));
                return;
            }

            if ((Flags::SPLIT & items[0].Expr->Flag) == 0) {
                return;
            }
        }

        Array<Match> new_set = Array<Match>((items.Size + 1));
        Array<Match> nesties;
        Match        item   = Match();
        size_t       offset = from;

        Match *tmp = nullptr;
        for (size_t i = 0; i < items.Size; i++) {
            tmp = &(items[i]);
            if ((Flags::NESTSPLIT & tmp->Expr->Flag) != 0) {
                Engine::Split(content, tmp->Expr, tmp->NestMatch, (tmp->Offset + tmp->OLength),
                              ((tmp->Offset + tmp->Length) - (tmp->CLength)));
            }

            if ((Flags::SPLIT & tmp->Expr->Flag) != 0) {
                item.Offset   = offset;
                item.Length   = (tmp->Offset - item.Offset);
                item.Expr     = ((expr == nullptr) ? tmp->Expr : expr);
                item.SubMatch = tmp->SubMatch;
                item.Tag      = tmp->Tag;

                if (tmp->NestMatch.Size != 0) {
                    item.NestMatch = tmp->NestMatch;
                } else if (nesties.Size != 0) {
                    item.NestMatch = nesties;
                } else if (item.Expr->NestExprs.Size != 0) {
                    item.NestMatch =
                        Engine::Search(content, item.Expr->NestExprs, item.Offset, (item.Offset + item.Length));
                }

                new_set.Add(item);

                if (item.NestMatch.Size != 0) {
                    item.NestMatch = Array<Match>();
                }
                if (item.SubMatch.Size != 0) {
                    item.SubMatch = Array<Match>();
                }
                if (nesties.Size != 0) {
                    nesties = Array<Match>();
                }

                offset = (tmp->Offset + tmp->Length);
            } else {
                nesties.Add(*tmp);
            }
        }

        item.Expr   = ((expr != nullptr) ? expr : item.Expr);
        item.Offset = offset;
        item.Length = (to - item.Offset);

        if (item.Length != 0) {
            if (nesties.Size != 0) {
                item.NestMatch = nesties;
            } else if (item.Expr->NestExprs.Size != 0) {
                item.NestMatch =
                    Engine::Search(content, item.Expr->NestExprs, item.Offset, (item.Offset + item.Length));
            }
        }

        new_set.Add(item);
        item.Offset    = from;
        item.Length    = (to - item.Offset);
        item.NestMatch = new_set; // TODO(Hani): Use move
        items          = Array<Match>();
        items.Add(item);
    }
}

/**
 * @brief Parse matched content (Slice & Dice).
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
        if (length == 0) {
            return content;
        }
    }

    String rendered = L""; // Final content

    Match *item;
    size_t from;
    size_t limit;
    for (size_t id = 0; id < items.Size; id++) {
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
            rendered += String::Part(content, offset, (from - offset));
        }

        if (item->Expr->ParseCB != nullptr) {
            if ((Flags::BUBBLE & item->Expr->Flag) != 0) {
                if (item->NestMatch.Size != 0) {
                    rendered +=
                        item->Expr->ParseCB(Engine::Parse(content, item->NestMatch, from, (from + limit)), *item);
                } else {
                    rendered += item->Expr->ParseCB(String::Part(content, from, limit), *item);
                }
            } else {
                rendered += item->Expr->ParseCB(content, *item);
            }
        } else {
            // Defaults to replace: it might be an empty string
            rendered += item->Expr->Replace;
        }

        offset = from + limit;
    }

    if (offset != 0) {
        if (offset < length) {
            // Adding the remaining of the text to the final rendered content.
            rendered += String::Part(content, offset, (length - offset));
        }

        return rendered;
    }

    // If there is no changes were made, return the original content.
    return content;
}
