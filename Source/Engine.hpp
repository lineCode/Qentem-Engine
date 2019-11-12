/**
 * Qentem Engine
 *
 * @brief     General purpose syntax parser.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "StringStream.hpp"

#ifndef QENTEM_ENGINE_H
#define QENTEM_ENGINE_H

namespace Qentem {
namespace Engine {
/////////////////////////////////
struct MatchBit;
struct Expression;
/////////////////////////////////
static void split(Array<MatchBit> &items, wchar_t const *content, UNumber offset, UNumber endOffset, UNumber count) noexcept;
using MatchCB_ = void(wchar_t const *content, UNumber &offset, UNumber const endOffset, MatchBit &item, Array<MatchBit> &items);
using ParseCB_ = String(wchar_t const *content, MatchBit const &item, UNumber const length, void *other);

using Expressions = Array<Expression *>;
/////////////////////////////////
// Expressions flags
struct Flags {
    // Parse()
    static UShort const BUBBLE = 1; // Parse nested matches.

    // Split()
    static UShort const DROPEMPTY = 2; // Drop every splitted match that's empty.
    static UShort const GROUPED   = 4; // Puts splitted matches into NestMatch, for one callback execution.
    static UShort const TRIM      = 8; // Trim the splitted match before adding it (spaces and newlines).

    // Match()
    static UShort const SPLIT  = 16;  // Split a match at with the specified keyword.
    static UShort const ONCE   = 32;  // Will stop searching after the first match.
    static UShort const IGNORE = 64;  // Igoring a match after finding it.
    static UShort const POP    = 128; // Match again with NestExpres if there is no match.
};
/////////////////////////////////
struct Expression {
    UNumber           Length{0};          // Keyword length.
    wchar_t const *   Keyword{nullptr};   // What to search for.
    Expression const *Connected{nullptr}; // The next part of the match (the next keyword).

    Expressions NestExpres;       // Expressions for nesting Match().
    UShort      Flag{0};          // Flags for the expression.
    MatchCB_ *  MatchCB{nullptr}; // A callback function for a custom action on a match.

    // for after match
    UShort         ID{0};                // Expression ID.
    ParseCB_ *     ParseCB{nullptr};     // A callback function for custom rendering.
    wchar_t const *ReplaceWith{nullptr}; // A text to replace a match.
    UNumber        RLength{0};           // Keyword length.

    void SetKeyword(wchar_t const *string) {
        Keyword = string;
        Length  = String::Count(string);
    }

    void SetReplace(wchar_t const *string) {
        ReplaceWith = string;
        RLength     = String::Count(string);
    }
};
/////////////////////////////////
struct MatchBit {
    UNumber           Offset{0}; // The start position of the matched string.
    UNumber           Length{0}; // The length of the entire match.
    Array<MatchBit>   NestMatch; // To hold sub matches inside a match.
    Expression const *Expr{nullptr};
};
/////////////////////////////////
static UNumber match(Array<MatchBit> &items, Expressions const &expres, wchar_t const *content, UNumber offset, UNumber const endOffset,
                     UNumber const maxOffset, UShort &split_count) noexcept {
    UNumber const started = offset;

    MatchBit item;

    UShort            keyword_offset;
    Expression const *expr;
    UShort            expr_id        = 0;
    UNumber           current_offset = 0;

    while (offset < endOffset) {
        current_offset = offset;
        expr           = expres[expr_id];
        keyword_offset = 0;

        while ((keyword_offset < expr->Length) && (expr->Keyword[keyword_offset] == content[current_offset])) {
            ++current_offset;
            ++keyword_offset;
        }

        if ((keyword_offset == 0) || (keyword_offset != expr->Length)) {
            if (expres.Size == (++expr_id)) {
                expr_id = 0;
                ++offset;
            }
            continue;
        }

        if (expr->Connected != nullptr) {
            UShort split_nest = 0;

            UShort const left_keyword_len = keyword_offset;

            UNumber sub_offset = current_offset;
            keyword_offset     = 0;
            expr               = expr->Connected;

            while (current_offset != maxOffset) {
                if (expr->Keyword[keyword_offset++] != content[current_offset++]) {
                    keyword_offset = 0;
                } else if (expr->Length == keyword_offset) {
                    if ((expr->NestExpres.Size != 0) && ((sub_offset + keyword_offset) != current_offset)) {
                        sub_offset = match(item.NestMatch, expr->NestExpres, content, sub_offset, current_offset, maxOffset, split_nest);
                    }

                    if (current_offset > sub_offset) {
                        break;
                    }

                    keyword_offset = 0;
                    current_offset = sub_offset;
                }
            }

            if (split_nest != 0) {
                split(item.NestMatch, content, (offset + left_keyword_len), (current_offset - expr->Length), split_nest);
            }

            if (keyword_offset == 0) {
                if (item.NestMatch.Size != 0) {
                    MatchBit *sub_match = &(item.NestMatch[(item.NestMatch.Size - 1)]);
                    items += static_cast<Array<MatchBit> &&>(item.NestMatch);
                    expr_id = 0;
                    offset  = (sub_match->Offset + sub_match->Length);
                } else if (expres.Size == (++expr_id)) {
                    expr_id = 0;
                    ++offset;
                }

                continue;
            }
        }

        if ((Flags::IGNORE & expr->Flag) == 0) {
            item.Offset = offset;
            item.Length = (current_offset - offset);
            item.Expr   = expr;

            if ((Flags::SPLIT & expr->Flag) != 0) {
                ++split_count;
            }

            if (expr->MatchCB == nullptr) {
                items += static_cast<MatchBit &&>(item);
            } else {
                expr->MatchCB(content, current_offset, endOffset, item, items);
            }

            if ((Flags::ONCE & expr->Flag) != 0) {
                return current_offset;
            }
        }

        expr_id = 0;
        offset  = current_offset;
    }

    if (((Flags::POP & expres[0]->Flag) != 0) && (items.Size == 0)) {
        return match(items, expres[0]->NestExpres, content, started, endOffset, endOffset, split_count);
    }

    return current_offset;
}
/////////////////////////////////
static Array<MatchBit> Match(Expressions const &expres, wchar_t const *content, UNumber const offset, UNumber const limit) noexcept {
    UShort          split_count = 0;
    UNumber const   endOffset   = (offset + limit);
    Array<MatchBit> items;

    match(items, expres, content, offset, endOffset, endOffset, split_count);

    if (split_count != 0) {
        split(items, content, offset, endOffset, split_count);
    }

    return items;
}
/////////////////////////////////
static String Parse(Array<MatchBit> const &items, wchar_t const *content, UNumber offset, UNumber limit, void *other = nullptr) noexcept {
    StringStream rendered; // Final content
    UNumber      tmp_limit;
    MatchBit *   item;

    for (UNumber id = 0; id < items.Size; id++) {
        // Current match
        item = &(items[id]);

        if (item->Offset < offset) {
            continue;
        }

        // Adding any content that comes before...
        if (offset < item->Offset) {
            tmp_limit = (item->Offset - offset);

            if (tmp_limit >= limit) {
                // If it's equal, then String::Part outside the for() will handel it.
                break;
            }

            limit -= tmp_limit;
            rendered += String::Part(content, offset, tmp_limit);
        }

        offset = item->Offset + item->Length;
        limit -= item->Length;

        if (item->Expr->ParseCB == nullptr) {
            // Defaults to replace: it can be empty.
            if (item->Expr->RLength != 0) {
                rendered.Add(item->Expr->ReplaceWith, item->Expr->RLength);
            }
        } else if ((Flags::BUBBLE & item->Expr->Flag) == 0) {
            rendered += item->Expr->ParseCB(content, *item, item->Length, other);
        } else if (item->NestMatch.Size != 0) {
            String tmp_string(Parse(item->NestMatch, content, item->Offset, item->Length, other));
            rendered += item->Expr->ParseCB(tmp_string.Str, *item, tmp_string.Length, other);
        } else {
            rendered += item->Expr->ParseCB(String::Part(content, item->Offset, item->Length).Str, *item, item->Length, other);
        }
    }

    if (limit != 0) {
        rendered += String::Part(content, offset, limit);
    }

    return rendered.Eject();
}

} // namespace Engine
/////////////////////////////////
static void Engine::split(Array<MatchBit> &items, wchar_t const *content, UNumber offset, UNumber const endOffset,
                          UNumber const count) noexcept {
    UNumber const started  = offset;
    MatchBit *    item_ptr = nullptr;

    Array<MatchBit> splitted(count + 1);
    MatchBit        item;

    for (UNumber i = 0; i <= items.Size; i++) {
        if (i != items.Size) {
            item_ptr = &(items[i]);

            if ((Flags::SPLIT & item_ptr->Expr->Flag) == 0) {
                item.NestMatch += static_cast<MatchBit &&>(*item_ptr);
                continue;
            }

            item.Offset = offset;
            item.Length = item_ptr->Offset - offset;
            item.Expr   = item_ptr->Expr;

            offset += item.Length + item_ptr->Length;
        } else {
            item.Offset = offset;
            item.Length = endOffset - offset;
        }

        if ((Flags::TRIM & item.Expr->Flag) != 0) {
            String::SoftTrim(content, item.Offset, item.Length);
        }

        if (((Flags::DROPEMPTY & item.Expr->Flag) == 0) || (item.Length != 0)) {
            if (item.Expr->NestExpres.Size != 0) {
                item.NestMatch = Match(item.Expr->NestExpres, content, item.Offset, item.Length);
            }

            splitted += static_cast<MatchBit &&>(item);
        }
    }

    if ((Flags::GROUPED & item.Expr->Flag) == 0) {
        items = static_cast<Array<MatchBit> &&>(splitted);
    } else {
        items.SetCapacity(1);
        ++items.Size;

        item_ptr = &(items[0]);

        item_ptr->Offset    = started;
        item_ptr->Length    = (endOffset - started);
        item_ptr->NestMatch = static_cast<Array<MatchBit> &&>(splitted);
        item_ptr->Expr      = item.Expr;
    }
}

} // namespace Qentem

#endif
