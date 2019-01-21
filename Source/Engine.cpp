
/**
 * Qentem Engine .^
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
 * @param index An index to start from.
 * @param limit The number of characters to search.
 * @param max The maximum index of to work with (to solve cutoff keywords).
 * @return An Array of matched items.
 */
Array<Match> Qentem::Engine::Search(const String &content, const Expressions &exprs, UNumber index, UNumber limit,
                                    UNumber max, UNumber level) noexcept {
    if (limit == 0) {
        limit = content.Length - index; // limit becomes the ending offset here.
    }

    // Feature: Add reversed search; instead of ++, use (1|-1) in a var to be implemented as: next_index+

    Array<Match> items = Array<Match>();
    if ((exprs.Size == 0) || (index >= limit)) {
        // If something's wrong, return an empty Array of type match.
        return items;
    }

    UNumber counter   = 0;     // Index for counting.
    UNumber id        = 0;     // Expression's id.
    bool    LOCKED    = false; // To keep matching the end of the current expression.
    bool    SPLIT     = false; // To keep tracking a split match.
    bool    OverDrive = false;
    Match   item      = Match(); // To store a single match for adding it to "items".

    const UNumber started = index;
    // Seting the value of the current expression.
    Expression *ce = exprs[id++];

    UNumber ground_zero = 0; // Temp offset.
    UNumber t_nest      = 0; // Temp variable for nesting matches.
    do {
        if (ce->SearchCB != nullptr) {
            item.Tag = counter = ce->SearchCB(content, *ce, index, ground_zero);
        } else if (content.Str[index] == ce->Keyword.Str[counter]) {
            // Don't change index.
            ground_zero = index;

            while (++counter < ce->Keyword.Length) {
                // Loop through every character.
                if (content.Str[++ground_zero] != ce->Keyword.Str[counter]) {
                    // Mismatch.
                    counter = 0;
                    break;
                }
            }
        }

        if (counter != 0) {
            counter = 0;

            if (OverDrive) {
                // If the match is on "OverDrive", then break.
                OverDrive = false;
                // Seting the length of the nested match.
                limit = ground_zero;
            } else if (!LOCKED) {
                // If the expression is not contented (single), then collect it's info; done matching.
                if (ce->Next != nullptr) {
                    item.OLength = ((ground_zero + 1) - index);
                }

                item.Offset = index;
                index       = ground_zero;
                t_nest      = (index + 1);
            }

            if (ce->Next != nullptr) {
                if (LOCKED) {
                    item.OLength = 0;
                }
                ce     = ce->Next;
                LOCKED = true;
            } else {
                // If it's a nesting expression, search again but inside the current match.
                if (ce->NestExprs.Size != 0) {
                    // Start a new search inside the current one.
                    UNumber _size = item.NestMatch.Size;
                    item.NestMatch.Add(
                        Engine::Search(content, ce->NestExprs, t_nest, index, ((max != 0) ? max : limit), (level + 1)));

                    if (item.NestMatch.Size != _size) {
                        if (max > limit) {
                            // This is important to have the seearch look ahead of the limited length
                            // in order to find the entire currect match.
                            limit     = max; // TO THE MAX!
                            OverDrive = true;
                        }

                        // Seek to avoid having the same closing/ending keywork matched again.
                        index = t_nest = item.NestMatch[(item.NestMatch.Size - 1)].Offset +
                                         item.NestMatch[(item.NestMatch.Size - 1)].Length;

                        continue; // Not done matching. Move to the next char.
                    }
                }

                if (item.OLength != 0) {
                    item.CLength = ((ground_zero + 1) - index);
                }

                item.Length = ((ground_zero + 1) - item.Offset);
                item.Expr   = ce;

                if (ce->SubExprs.Size != 0) {
                    item.SubMatch = Engine::Search(content, ce->SubExprs, item.Offset, (item.Offset + item.Length));
                }

                if ((Flags::SPLIT & ce->Flag) != 0) {
                    SPLIT = true;
                }

                if ((level == 0) && ((Flags::SPLITNEST & ce->Flag) != 0)) {
                    Engine::Split(content, item.NestMatch, (item.Offset + item.OLength),
                                  ((item.Offset + item.Length) - (item.CLength)));
                }

                items.Add(item);

                if ((Flags::ONCE & ce->Flag) != 0) {
                    return items;
                }

                // Prepare for the next match.
                item   = Match();
                LOCKED = false;
                id     = exprs.Size; // Reset expressions!
            }
        }

        if (!LOCKED) {
            // Switching to the next charrcter if all keywords have been tested.
            if (id == exprs.Size) {
                id = 0;
                ++index;
            }
            // Seting the next keyword for searching.
            ce = exprs[id++];
        } else {
            // If there is an ongoing match, then move to the next wchar_t.
            ++index;
        }

        // If it has gone too far (linked keyword is missing).
        if (index >= limit) {
            if (!LOCKED) {
                break;
            }

            if (max > limit) {
                // This is important to have the seearch look ahead of the limited length
                // in order to find the entire currect match.
                limit = max;

                OverDrive = true;
                continue;
            }

            if (item.NestMatch.Size != 0) {
                // if it's a nested search... with matched items move every thing that has been found to the
                // main items' list, to avoid searching them again.
                items.Add(item.NestMatch);

                {
                    Match *m = &(item.NestMatch)[(item.NestMatch.Size - 1)];
                    // Seek the offset to where the last match ended.
                    index = m->Offset + m->Length;
                }

                if (index == limit) {
                    break;
                }
            } else {
                // If it is a missmatch, go back to where it started and continue with the next expression if
                // possible.
                index = item.Offset;
            }

            // House cleaning...
            OverDrive = false;
            LOCKED    = false;
        }
    } while (true);

    if (items.Size == 0) {
        if ((Flags::POP & ce->Flag) != 0) { //&& (ce->NestExprs.Size != 0)
            return Engine::Search(content, ce->NestExprs, started, limit);
        }
    } else if ((level == 0) && SPLIT) {
        Engine::Split(content, items, started, limit);
    }

    // Friday, January 18, 2019
    // AlJumaa, Jamada El Oula 12, 1440

    return items;
}

void Qentem::Engine::Split(const String &content, Array<Match> &items, UNumber index, UNumber to) noexcept {
    if (items.Size > 0) {
        if (items.Size == 1) {
            if ((Flags::SPLITNEST & items[0].Expr->Flag) != 0) {
                Engine::Split(content, items[0].NestMatch, (items[0].Offset + items[0].OLength),
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
        UNumber      offset = index;

        Match *tmp = nullptr;
        for (UNumber i = 0; i < items.Size; i++) {
            tmp       = &(items[i]);
            item.Expr = tmp->Expr;

            if ((Flags::SPLITNEST & tmp->Expr->Flag) != 0) {
                Engine::Split(content, tmp->NestMatch, (tmp->Offset + tmp->OLength),
                              ((tmp->Offset + tmp->Length) - (tmp->CLength)));
            }

            if ((Flags::SPLIT & tmp->Expr->Flag) != 0) {
                item.Offset   = offset;
                item.Length   = (tmp->Offset - item.Offset);
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

                item.NestMatch.Clear();
                item.SubMatch.Clear();
                nesties.Clear();

                offset = (tmp->Offset + tmp->Length);
            } else {
                nesties.Add(*tmp);
            }
        }

        item.Offset = offset;
        item.Length = (to - item.Offset);

        if (item.Length != 0) {
            if (nesties.Size != 0) {
                item.NestMatch = nesties;
            } else if (item.Expr->NestExprs.Size != 0) {
                item.NestMatch = Engine::Search(content, item.Expr->NestExprs, item.Offset, (item.Offset + item.Length));
            }
        }

        new_set.Add(item);

        if ((Flags::GROUPSPLIT & item.Expr->Flag) == 0) {
            items = new_set;
        } else {
            item.Offset    = index;
            item.Length    = (to - item.Offset);
            item.NestMatch = new_set;
            items          = Array<Match>();
            items.Add(item);
        }
    }
}

/**
 * @brief Parse matched content (Slice & Dice).
 *
 * @param content The text with the matches.
 * @param items The items that attended to be parsed.
 * @param offset An index to start index.
 * @param length The length of the content to work with.
 * @return The parsed (final) content.
 */
String Qentem::Engine::Parse(const String &content, const Array<Match> &items, UNumber index, UNumber limit) noexcept {
    if (limit == 0) {
        limit = index + content.Length;
        if (limit == 0) {
            return content;
        }
    }

    String rendered = L""; // Final content

    Match * item;
    UNumber offset;
    UNumber length;
    for (UNumber id = 0; id < items.Size; id++) {
        // Current match
        item = &(items[id]);

        if ((Flags::IGNORE & item->Expr->Flag) != 0) {
            continue;
        }

        if ((Flags::COMPACT & item->Expr->Flag) == 0) {
            offset = item->Offset;
            length = item->Length;
        } else {
            offset = (item->Offset + item->OLength);
            length = (item->Length - (item->OLength + item->CLength));
        }

        // Adding any content that comes before...
        if ((index < offset)) {
            rendered += String::Part(content, index, (offset - index));
        }

        if (item->Expr->ParseCB != nullptr) {
            if ((Flags::BUBBLE & item->Expr->Flag) != 0) {
                if (item->NestMatch.Size != 0) {
                    rendered +=
                        item->Expr->ParseCB(Engine::Parse(content, item->NestMatch, offset, (offset + length)), *item);
                } else {
                    rendered += item->Expr->ParseCB(String::Part(content, offset, length), *item);
                }
            } else {
                rendered += item->Expr->ParseCB(content, *item);
            }
        } else {
            // Defaults to replace: it might be an empty string
            rendered += item->Expr->Replace;
        }

        index = offset + length;
    }

    if (index != 0) {
        if (index < limit) {
            // Adding the remaining of the text to the final rendered content.
            rendered += String::Part(content, index, (limit - index));
        }

        return rendered;
    }

    // If there is no changes were made, return the original content.
    return content;
}
