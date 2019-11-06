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
struct Match;
struct Expression;
/////////////////////////////////
static void _split(Array<Match> &items, wchar_t const *content, UNumber offset, UNumber endOffset, UNumber count) noexcept;
using _ParseCB = String(wchar_t const *content, Match const &item, UNumber const length, void *other);
using _MatchCB = void(wchar_t const *content, UNumber &offset, UNumber const endOffset, Match &item, Array<Match> &items);

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

    // Search()
    static UShort const SPLIT  = 16;  // Split a match at with the specified keyword.
    static UShort const ONCE   = 32;  // Will stop searching after the first match.
    static UShort const IGNORE = 64;  // Igoring a match after finding it.
    static UShort const POP    = 128; // Search again with NestExprs if there is no match.
};
/////////////////////////////////
struct Expression {
    UNumber        Length{0};        // Keyword length.
    wchar_t const *Keyword{nullptr}; // What to search for.

    Expression *Connected{nullptr}; // The next part of the match (the next keyword).
    Expressions NestExprs;          // Expressions for nesting Search().
    UShort      Flag{0};            // Flags for the expression.
    _MatchCB *  MatchCB{nullptr};   // A callback function for a custom action on a match.

    // for after match
    UShort         ID{0};                // Expression ID.
    _ParseCB *     ParseCB{nullptr};     // A callback function for custom rendering.
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
struct Match {
    UNumber      Offset{0}; // The start position of the matched string.
    UNumber      Length{0}; // The length of the entire match.
    Array<Match> NestMatch; // To hold sub matches inside a match.
    Expression * Expr{nullptr};
};
/////////////////////////////////
static UNumber _search(Array<Match> &items, wchar_t const *content, Expressions const &exprs, UNumber offset, UNumber const endOffset,
                       UNumber const maxOffset, UShort &split) noexcept {
    UNumber const started = offset;

    Match item;

    UShort      keyword_offset;
    Expression *expr;
    UShort      expr_id        = 0;
    UNumber     current_offset = 0;

    while (offset < endOffset) {
        current_offset = offset;
        expr           = exprs[expr_id];
        keyword_offset = 0;

        while ((keyword_offset < expr->Length) && (expr->Keyword[keyword_offset] == content[current_offset])) {
            ++current_offset;
            ++keyword_offset;
        }

        if ((keyword_offset == 0) || (keyword_offset != expr->Length)) {
            if (exprs.Size == (++expr_id)) {
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
                    if ((expr->NestExprs.Size != 0) && ((sub_offset + keyword_offset) != current_offset)) {
                        sub_offset = _search(item.NestMatch, content, expr->NestExprs, sub_offset, current_offset, maxOffset, split_nest);
                    }

                    if (current_offset > sub_offset) {
                        break;
                    }

                    keyword_offset = 0;
                    current_offset = sub_offset;
                }
            }

            if (split_nest != 0) {
                _split(item.NestMatch, content, (offset + left_keyword_len), (current_offset - expr->Length), split_nest);
            }

            if (keyword_offset == 0) {
                if (item.NestMatch.Size != 0) {
                    Match *sub_match = &(item.NestMatch[(item.NestMatch.Size - 1)]);
                    items += static_cast<Array<Match> &&>(item.NestMatch);
                    expr_id = 0;
                    offset  = (sub_match->Offset + sub_match->Length);
                } else if (exprs.Size == (++expr_id)) {
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
                ++split;
            }

            if (expr->MatchCB == nullptr) {
                items += static_cast<Match &&>(item);
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

    if (((Flags::POP & exprs[0]->Flag) != 0) && (items.Size == 0)) {
        return _search(items, content, exprs[0]->NestExprs, started, endOffset, endOffset, split);
    }

    return current_offset;
}
/////////////////////////////////
static Array<Match> Search(wchar_t const *content, Expressions const &exprs, UNumber const offset, UNumber const limit) noexcept {
    UShort        split     = 0;
    UNumber const endOffset = (offset + limit);
    Array<Match>  items;

    _search(items, content, exprs, offset, endOffset, endOffset, split);

    if (split != 0) {
        _split(items, content, offset, endOffset, split);
    }

    return items;
}
/////////////////////////////////
static String Parse(wchar_t const *content, Array<Match> const &items, UNumber offset, UNumber limit, void *other = nullptr) noexcept {
    StringStream rendered; // Final content
    UNumber      tmp_limit;
    Match *      item;
    String       tmp_string;

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

        if (item->Expr->ParseCB == nullptr) {
            // Defaults to replace: it can be empty.
            if (item->Expr->RLength != 0) {
                rendered.Add(item->Expr->ReplaceWith, item->Expr->RLength);
            }
        } else if ((Flags::BUBBLE & item->Expr->Flag) == 0) {
            rendered += item->Expr->ParseCB(content, *item, item->Length, other);
        } else if (item->NestMatch.Size != 0) {
            tmp_string = Parse(content, item->NestMatch, item->Offset, item->Length, other);
            rendered += item->Expr->ParseCB(tmp_string.Str, *item, tmp_string.Length, other);
        } else {
            rendered += item->Expr->ParseCB(String::Part(content, item->Offset, item->Length).Str, *item, item->Length, other);
        }

        offset = item->Offset + item->Length;
        limit -= item->Length;
    }

    if (limit != 0) {
        rendered += String::Part(content, offset, limit);
    }

    return rendered.Eject();
}

} // namespace Engine
/////////////////////////////////
static void Engine::_split(Array<Match> &items, wchar_t const *content, UNumber offset, UNumber const endOffset,
                           UNumber const count) noexcept {
    UNumber const started  = offset;
    Match *       item_ptr = nullptr;

    Array<Match> splitted;
    splitted.SetCapacity(count + 1);
    Match item;

    for (UNumber i = 0; i <= items.Size; i++) {
        if (i != items.Size) {
            item_ptr = &(items[i]);

            if ((Flags::SPLIT & item_ptr->Expr->Flag) == 0) {
                item.NestMatch += static_cast<Match &&>(*item_ptr);
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
            if (item.Expr->NestExprs.Size != 0) {
                item.NestMatch = Search(content, item.Expr->NestExprs, item.Offset, item.Length);
            }

            splitted += static_cast<Match &&>(item);
        }
    }

    if ((Flags::GROUPED & item.Expr->Flag) == 0) {
        items = static_cast<Array<Match> &&>(splitted);
    } else {
        items.SetCapacity(1);
        ++items.Size;

        item_ptr = &(items[0]);

        item_ptr->Offset    = started;
        item_ptr->Length    = (endOffset - started);
        item_ptr->NestMatch = static_cast<Array<Match> &&>(splitted);
        item_ptr->Expr      = item.Expr;
    }
}

} // namespace Qentem

#endif
