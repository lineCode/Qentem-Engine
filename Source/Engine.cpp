
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
#include "StringStream.hpp"

using Qentem::Array;
using Qentem::String;
using Qentem::StringStream;
using Qentem::Engine::Match;

Array<Match> Qentem::Engine::Search(const String &content, const Expressions &exprs, UNumber index, UNumber limit,
                                    UNumber max, UNumber level) noexcept {
    if (limit == 0) {
        limit = (content.Length - index); // limit becomes the ending offset here.
    }

    // Feature: callback for reverse search

    Array<Match> items;
    if ((exprs.Size == 0) || (index >= limit)) {
        // If something's wrong, return an empty Array of type match.
        return items;
    }

    bool    LOCKED    = false;   // To keep matching the end of the current expression.
    bool    SPLIT     = false;   // To keep tracking a split match.
    bool    OVERDRIVE = false;   // To achieving nesting.
    UNumber counter   = 0;       // Index for counting.
    Match   item      = Match(); // Getting ready for a match.

    const UNumber started = index;
    UNumber       id      = 0; // Expression's id.
    // Seting the value of the current expression.
    Expression *ce = exprs[id++];

    UNumber end_offset  = 0; // Temp offset.
    UNumber nest_offset = 0; // Temp variable for nested matches.
    do {
        if (ce->SearchCB != nullptr) {
            counter = ce->SearchCB(content, *ce, &item, index, end_offset, limit);
        } else if (content.Str[index] == ce->Keyword.Str[counter]) {
            // Maintain index's value.
            end_offset = index;

            while (++counter < ce->Keyword.Length) {
                // Loop through every character.
                if (content.Str[++end_offset] != ce->Keyword.Str[counter]) {
                    // Mismatch.
                    counter = 0;
                    break;
                }
            }
        }

        if (counter != 0) {
            counter = 0;
            end_offset += 1;

            if (OVERDRIVE) {
                // If the match is on "OVERDRIVE", then break.
                OVERDRIVE = false;
                // Ste the length of the nested match.
                limit = (end_offset - 1);
            } else if (!LOCKED) {
                if (ce->Connected != nullptr) {
                    if ((Flags::TRIM & ce->Connected->Flag) != 0) {
                        while (content.Str[end_offset] == L' ') {
                            end_offset++;
                        }
                    }
                    item.OLength = (end_offset - index);
                }

                item.Offset = index;

                index       = (end_offset - 1);
                nest_offset = end_offset;
            }

            if (ce->Connected != nullptr) {
                if (LOCKED) {
                    item.OLength = 0;
                }
                ce     = ce->Connected;
                LOCKED = true;
            } else {
                // If it's a nesting expression, search again but inside the current match.
                if ((ce->NestExprs.Size != 0) && (nest_offset != index)) {
                    // Start a new search inside the current one.
                    UNumber _size = item.NestMatch.Size;
                    item.NestMatch.Add(Engine::Search(content, ce->NestExprs, nest_offset, index,
                                                      ((max != 0) ? max : limit), (level + 1)));

                    if (item.NestMatch.Size != _size) {
                        if (max > limit) {
                            // This is important to have the seearch look ahead of the limited length
                            // in order to find the entire currect match.
                            limit     = max; // TO THE MAX!
                            OVERDRIVE = true;
                        }

                        // Seek to avoid having the same closing/ending keywork matched again.
                        index = nest_offset = item.NestMatch[(item.NestMatch.Size - 1)].Offset +
                                              item.NestMatch[(item.NestMatch.Size - 1)].Length;

                        continue; // Not done matching. Move to the next char.
                    }
                }

                if ((Flags::IGNORE & ce->Flag) == 0) {
                    item.Length = (end_offset - item.Offset);

                    if (item.OLength != 0) {
                        if (((Flags::TRIM & ce->Flag) != 0) && ((index - item.Offset) != item.OLength)) {
                            while (content.Str[--index] == L' ') {
                            }
                            index++;
                        }
                        item.CLength = (end_offset - index);
                    }

                    if (((Flags::DROPEMPTY & ce->Flag) == 0) || (item.Length != (item.OLength + item.CLength))) {
                        if ((Flags::SPLIT & ce->Flag) != 0) {
                            SPLIT = true;
                        }

                        if (ce->SubExprs.Size != 0) {
                            item.SubMatch =
                                Engine::Search(content, ce->SubExprs, item.Offset, (item.Offset + item.Length));
                        }

                        item.Expr = ce;

                        if ((level == 0) &&
                            (((Flags::SPLITNEST & ce->Flag) != 0) || ((Flags::SPLITROOTONLY & ce->Flag) != 0))) {
                            Engine::Split(content, item.NestMatch, (item.Offset + item.OLength),
                                          ((item.Offset + item.Length) - (item.CLength)));
                        }

                        items.Add(item);

                        if ((Flags::ONCE & ce->Flag) != 0) {
                            return items;
                        }
                    }
                }

                // Prepare for the next match.
                item   = Match();
                LOCKED = false;
                id     = exprs.Size; // Reset expressions!
            }
        }

        if (!LOCKED) {
            // Switching to the next charrcter if all keywords have been checked.
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

                OVERDRIVE = true;
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

                item.NestMatch.Clear();
            } else {
                // If it is a missmatch, go back to where it started and continue with the next expression if
                // possible.
                index = item.Offset;
            }

            // House cleaning...
            OVERDRIVE = false;
            LOCKED    = false;
            if (id == exprs.Size) {
                return items;
            }

            ce = exprs[id];
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

void Qentem::Engine::Split(const String &content, Array<Match> &items, const UNumber index, const UNumber to) noexcept {
    if (items.Size == 0) {
        return;
    }

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

    Array<Match> splitted;
    splitted.SetCapacity(items.Size + 1);
    Array<Match> nesties;
    Match        item   = Match();
    UNumber      offset = index;
    UNumber      ends;
    Expression * master = nullptr;
    Match *      tmp    = nullptr;

    for (UNumber i = 0; i < items.Size; i++) {
        tmp       = &(items[i]);
        item.Expr = tmp->Expr;

        if ((Flags::SPLITNEST & tmp->Expr->Flag) != 0) {
            Engine::Split(content, tmp->NestMatch, (tmp->Offset + tmp->OLength),
                          ((tmp->Offset + tmp->Length) - (tmp->CLength)));
        }

        if ((Flags::SPLIT & tmp->Expr->Flag) != 0) {

            item.Offset = offset;
            if ((Flags::TRIM & item.Expr->Flag) != 0) {
                while (content.Str[item.Offset] == L' ') {
                    item.Offset++;
                }

                item.Length = (tmp->Offset - item.Offset);
                ends        = tmp->Offset;

                if (item.Length != 0) {
                    while (content.Str[--ends] == L' ') {
                    }
                    ends++;
                    item.Length = (ends - item.Offset);
                }
            } else {
                item.Length = (tmp->Offset - item.Offset);
            }

            item.Tag = tmp->Tag;
            master   = item.Expr;

            if (((Flags::DROPEMPTY & item.Expr->Flag) == 0) || (item.Length != 0)) {
                if (tmp->NestMatch.Size != 0) {
                    item.NestMatch.Move(tmp->NestMatch);
                } else if (nesties.Size != 0) {
                    item.NestMatch.Move(nesties);
                } else if (item.Expr->NestExprs.Size != 0) {
                    item.NestMatch =
                        Engine::Search(content, item.Expr->NestExprs, item.Offset, (item.Offset + item.Length));
                }

                splitted.Add(item);
                item.SubMatch.Move(tmp->SubMatch);
                item.NestMatch.Clear();
            }
            offset = (tmp->Offset + tmp->Length);
        } else {
            nesties.Add(*tmp);
        }
    }

    item.Offset = offset;
    if ((Flags::TRIM & item.Expr->Flag) != 0) {
        while (content.Str[item.Offset] == L' ') {
            item.Offset++;
        }

        item.Length = (to - item.Offset);
        ends        = to;

        if (item.Length != 0) {
            while (content.Str[--ends] == L' ') {
            }
            ends++;
            item.Length = (ends - item.Offset);
        }
    } else {
        item.Length = (to - item.Offset);
    }

    if (((Flags::DROPEMPTY & item.Expr->Flag) == 0) || (item.Length != 0)) {

        if (item.Length != 0) {
            if (nesties.Size != 0) {
                item.NestMatch.Move(nesties);
            } else if (item.Expr->NestExprs.Size != 0) {
                item.NestMatch = Engine::Search(content, item.Expr->NestExprs, item.Offset, (item.Offset + item.Length));
            }
        }

        splitted.Add(item);
    }

    if (splitted.Size == 0) {
        items.Clear();
        return;
    }

    if ((master != nullptr) && (Flags::GROUPSPLIT & master->Flag) == 0) {
        items.Move(splitted);
    } else {
        item.SubMatch.Clear();
        item.NestMatch.Move(splitted);

        item.Offset = index;
        item.Length = (to - item.Offset);
        item.Expr   = master;

        items.Clear();
        items.Add(item);
    }
}

String Qentem::Engine::Parse(const String &content, const Array<Match> &items, UNumber index, UNumber limit) noexcept {
    if (limit == 0) {
        limit = (content.Length - index);
        if (limit == 0) {
            return content;
        }
    }

    StringStream rendered; // Final content
    UNumber      offset;
    UNumber      length;
    Match *      item;

    for (UNumber id = 0; id < items.Size; id++) {
        // Current match
        item = &(items[id]);

        if ((Flags::NOPARSE & item->Expr->Flag) != 0) {
            continue;
        }

        if ((Flags::COMPACT & item->Expr->Flag) == 0) {
            offset = item->Offset;
            length = item->Length;
        } else {
            offset = (item->Offset + item->OLength);
            length = (item->Length - (item->OLength + item->CLength));
        }

        // Add any content that comes before...
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
            // Defaults to replace: it might be an empty string.
            rendered += item->Expr->Replace;
        }

        index = offset + length;
    }

    if (index != 0) {
        if (index < limit) {
            // Adding the remaining of the text to the final rendered content.
            rendered += String::Part(content, index, (limit - index));
        }

        return rendered.Eject();
    }

    // If there is no changes were made, return the original content.
    return content;
}
