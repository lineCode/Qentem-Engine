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
static void _split(Array<Match> &items, String const &content, UNumber offset, UNumber endOffset) noexcept;
using _ParseCB = String(String const &content, Match const &item, void *other);
using _MatchCB = void(String const &content, UNumber &offset, UNumber const endOffset, Match &item, Array<Match> &items);

using Expressions = Array<Expression *>;
/////////////////////////////////
// Expressions flags
struct Flags {
    static UShort const BUBBLE    = 1;   // Parse nested matches.
    static UShort const DROPEMPTY = 2;   // Drop every splitted match that's empty.
    static UShort const GROUPED   = 4;   // Puts splitted matches into NestMatch, for one callback execution.
    static UShort const TRIM      = 8;   // Trim the splitted match before adding it (spaces and newlines).
    static UShort const SPLIT     = 16;  // Split a match at with the specified keyword.
    static UShort const ONCE      = 32;  // Will stop searching after the first match.
    static UShort const IGNORE    = 64;  // Igoring a match after finding it.
    static UShort const POP       = 128; // Search again with NestExprs if there is no match.
};
/////////////////////////////////
struct Expression {
    Expression *Connected = nullptr; // The next part of the match (the next keyword).
    UNumber     Flag      = 0;       // Flags for the expression.
    String      Keyword;             // What to search for.
    Expressions NestExprs;           // Expressions for nesting Search().

    _MatchCB *MatchCB = nullptr; // A callback function for a custom action on a match.
    UShort    ID      = 0;       // Expression ID.
    _ParseCB *ParseCB = nullptr; // A callback function for custom rendering.
    String    Replace;           // A text to replace the match.
};
/////////////////////////////////
struct Match {
    Expression * Expr = nullptr;
    Array<Match> NestMatch;  // To hold sub matches inside a match.
    UNumber      Offset = 0; // The start position of the matched string.
    UNumber      Length = 0; // The length of the entire match.
};
/////////////////////////////////
static UNumber _search(Array<Match> &items, String const &content, Expressions const &exprs, UNumber offset,
                       UNumber endOffset, UNumber const maxOffset, bool &split) noexcept {
    UNumber const started = offset;

    Match item;

    UNumber     expr_id = 0;
    Expression *expr;
    UNumber     keyword_offset;
    UNumber     current_offset = 0;

    while (offset < endOffset) {
        current_offset = offset;
        expr           = exprs[expr_id];
        keyword_offset = 0;

        while ((keyword_offset < expr->Keyword.Length) && (expr->Keyword[keyword_offset] == content[current_offset])) {
            ++current_offset;
            ++keyword_offset;
        }

        if (keyword_offset != expr->Keyword.Length) {
            if (exprs.Size == (++expr_id)) {
                expr_id = 0;
                ++offset;
            }
            continue;
        }

        if (expr->Connected != nullptr) {
            expr = expr->Connected;

            bool split_nest = false;

            UNumber const left_keyword_len = keyword_offset;

            UNumber sub_offset = current_offset;
            keyword_offset     = 0;

            while (current_offset != maxOffset) {
                if (expr->Keyword[keyword_offset++] != content[current_offset++]) {
                    keyword_offset = 0;
                    continue;
                }

                if (expr->Keyword.Length == keyword_offset) {
                    if ((expr->NestExprs.Size != 0) && ((sub_offset + keyword_offset) != current_offset)) {
                        sub_offset = _search(item.NestMatch, content, expr->NestExprs, sub_offset, current_offset,
                                             maxOffset, split_nest);
                    }

                    if (current_offset > sub_offset) {
                        break;
                    }

                    current_offset = sub_offset;
                    keyword_offset = 0;
                }
            }

            if (split_nest) {
                _split(item.NestMatch, content, (offset + left_keyword_len), (current_offset - expr->Keyword.Length));
            }

            if (keyword_offset == 0) {
                if (item.NestMatch.Size != 0) {
                    Match *sub_match = &(item.NestMatch[(item.NestMatch.Size - 1)]);
                    items += static_cast<Array<Match> &&>(item.NestMatch);
                    expr_id = 0;
                    offset  = (sub_match->Offset + sub_match->Length);
                    continue;
                }

                if (exprs.Size == (++expr_id)) {
                    expr_id = 0;
                    ++offset;
                }

                continue;
            }
        }

        if ((Flags::IGNORE & expr->Flag) == 0) {
            item.Expr   = expr;
            item.Offset = offset;
            item.Length = (current_offset - offset);

            if (((Flags::SPLIT & expr->Flag) != 0) && !split) {
                split = true;
            }

            if (expr->MatchCB != nullptr) {
                expr->MatchCB(content, current_offset, endOffset, item, items);
            } else {
                items += static_cast<Match &&>(item);
            }

            if ((Flags::ONCE & expr->Flag) != 0) {
                break;
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
static Array<Match> Search(String const &content, Expressions const &exprs, UNumber const offset,
                           UNumber const limit) noexcept {
    bool          split     = false;
    UNumber const endOffset = (offset + limit);
    Array<Match>  items;

    _search(items, content, exprs, offset, endOffset, endOffset, split);

    if (split) {
        _split(items, content, offset, endOffset);
    }

    return items;
}
/////////////////////////////////
static String Parse(String const &content, Array<Match> const &items, UNumber offset, UNumber limit,
                    void *other = nullptr) noexcept {
    StringStream rendered; // Final content
    UNumber      tmp_limit;
    Match *      item;

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
            rendered += String::Part(content.Str, offset, tmp_limit);
        }

        if (item->Expr->ParseCB == nullptr) {
            // Defaults to replace: it can be empty.
            rendered += item->Expr->Replace;
        } else if ((Flags::BUBBLE & item->Expr->Flag) == 0) {
            rendered += item->Expr->ParseCB(content, *item, other);
        } else if (item->NestMatch.Size != 0) {
            rendered +=
                item->Expr->ParseCB(Parse(content, item->NestMatch, item->Offset, item->Length, other), *item, other);
        } else {
            rendered += item->Expr->ParseCB(String::Part(content.Str, item->Offset, item->Length), *item, other);
        }

        offset = item->Offset + item->Length;
        limit -= item->Length;
    }

    if (limit != 0) {
        rendered += String::Part(content.Str, offset, limit);
    }

    return rendered.Eject();
}
} // namespace Engine
/////////////////////////////////
static void Engine::_split(Array<Match> &items, String const &content, UNumber offset,
                           UNumber const endOffset) noexcept {
    if (items.Size == 0) {
        return;
    }

    UNumber const started = offset;

    Array<Match> splitted;
    // splitted.SetCapacity(items.Size + 1);

    Match  item;
    Match *item_ptr = nullptr;

    for (UNumber i = 0; i <= items.Size; i++) {
        if (i != items.Size) {
            item_ptr = &(items[i]);

            if ((Flags::SPLIT & item_ptr->Expr->Flag) == 0) {
                item.NestMatch += static_cast<Match &&>(*item_ptr);
                continue;
            }

            item.Expr   = item_ptr->Expr;
            item.Offset = offset;
            item.Length = item_ptr->Offset - offset;
            offset += item.Length + item_ptr->Length;
        } else {
            item.Offset = offset;
            item.Length = endOffset - offset;
        }

        if ((Flags::TRIM & item.Expr->Flag) != 0) {
            String::SoftTrim(content.Str, item.Offset, item.Length);
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
        items.Size = 1;
        item_ptr   = &(items[0]);

        item_ptr->Offset    = started;
        item_ptr->Length    = (endOffset - started);
        item_ptr->Expr      = item.Expr;
        item_ptr->NestMatch = static_cast<Array<Match> &&>(splitted);
    }
}

} // namespace Qentem

#endif
