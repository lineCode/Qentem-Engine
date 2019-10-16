
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
    static UShort const IGNORE    = 1;   // Match a Keyword but don't add it.
    static UShort const TRIM      = 2;   // Trim the match before adding it.
    static UShort const ONCE      = 4;   // Will stop searching after the first match.
    static UShort const BUBBLE    = 8;   // Parse nested matches.
    static UShort const SPLIT     = 16;  // Split a match at a keyword.
    static UShort const GROUPED   = 32;  // Puts splitted matches into NestMatch, for one callback execution.
    static UShort const DROPEMPTY = 64;  // Trim the match before adding it (spaces and newlines).
    static UShort const POP       = 128; // Search again with NestExprs if the match fails (See ALU.cpp).
};
/////////////////////////////////
struct Expression {
    String  Keyword = L""; // What to search for.
    UNumber Flag    = 0;   // Flags for the expression.
    UShort  ID      = 0;   // Expression ID.
    String  Replace;       // A text to replace the match.

    Expression *Connected = nullptr; // The next part of the match (the next keyword).
    _ParseCB *  ParseCB   = nullptr; // A callback function for custom rendering.
    _MatchCB *  MatchCB   = nullptr; // A callback function for a custom action on a match.

    Expressions NestExprs; // Expressions for nesting Search().
};
/////////////////////////////////
struct Match {
    UNumber Offset = 0; // The start position of the matched string.
    UNumber Length = 0; // The length of the entire match.

    UNumber OLength = 0; // Length of the start keyword.
    UNumber CLength = 0; // Length of the end keyword.

    Expression * Expr = nullptr;
    Array<Match> NestMatch; // To hold sub matches inside a match.
};
/////////////////////////////////
static void _search(Array<Match> &items, String const &content, Expressions const &exprs, UNumber offset,
                    UNumber endOffset, UNumber const maxOffset, bool &split) noexcept {
    UNumber const started        = offset;
    UNumber       current_offset = offset;
    bool          split_nest     = false;

    UNumber expr_id        = 0;
    UNumber keyword_offset = 0;

    Expression *expr;
    Match       item;

    bool MATCHED;

    while (offset < endOffset) {
        expr           = exprs[expr_id];
        keyword_offset = 0;
        current_offset = offset;

        while ((expr->Keyword.Length > keyword_offset) && expr->Keyword[keyword_offset] == content[current_offset]) {
            ++current_offset;
            ++keyword_offset;
        }

        if (keyword_offset == expr->Keyword.Length) {
            item.Offset = offset;

            if (!(MATCHED = (expr->Connected == nullptr))) {
                item.OLength = expr->Keyword.Length;
                expr         = expr->Connected;

                UNumber sub_offset = current_offset;
                bool    OVERDRIVE  = false;

                keyword_offset = 0;

                while (current_offset < endOffset) {
                    if (expr->Keyword[keyword_offset++] != content[current_offset++]) {
                        keyword_offset = 0;
                        if (current_offset == endOffset) {
                            OVERDRIVE = (endOffset != maxOffset);
                            endOffset = maxOffset;
                        }
                        continue;
                    }

                    if (expr->Keyword.Length == keyword_offset) {
                        if (expr->NestExprs.Size == 0) {
                            MATCHED = true;
                            break;
                        }

                        _search(item.NestMatch, content, expr->NestExprs, sub_offset, current_offset, maxOffset,
                                split_nest);

                        if (item.NestMatch.Size == 0) {
                            MATCHED = true;
                            break;
                        }

                        Match *sub_match = &(item.NestMatch[(item.NestMatch.Size - 1)]);
                        sub_offset       = (sub_match->Offset + sub_match->Length);

                        if (sub_offset < current_offset) {
                            if (OVERDRIVE) {
                                endOffset = sub_offset;
                            }

                            MATCHED = true;
                            break;
                        }

                        if ((sub_offset >= endOffset) && (endOffset != maxOffset)) {
                            endOffset = maxOffset;
                            OVERDRIVE = true;
                        }

                        current_offset = sub_offset;
                        keyword_offset = 0;
                    }
                }

                if (MATCHED) {
                    item.CLength = expr->Keyword.Length;
                } else if (item.NestMatch.Size != 0) {
                    items += static_cast<Array<Match> &&>(item.NestMatch);
                }
            }

            if (MATCHED) {
                if ((Flags::IGNORE & expr->Flag) == 0) {
                    item.Length = (current_offset - item.Offset);

                    if ((Flags::TRIM & expr->Flag) != 0) {
                        UNumber tmpIndex = (item.Offset + item.OLength);
                        while ((content[tmpIndex] == L' ') || (content[tmpIndex] == L'\n') ||
                               (content[tmpIndex] == L'\t') || (content[tmpIndex] == L'\r')) {
                            ++item.OLength;
                            ++tmpIndex;
                        }

                        tmpIndex = ((item.Offset + item.Length) - item.CLength);
                        if ((item.OLength + item.CLength) != item.Length) {
                            while ((content[--tmpIndex] == L' ') || (content[tmpIndex] == L'\n') ||
                                   (content[tmpIndex] == L'\t') || (content[tmpIndex] == L'\r')) {
                                ++item.CLength;
                            }
                        }
                    }

                    if (((Flags::DROPEMPTY & expr->Flag) == 0) || (item.Length != (item.OLength + item.CLength))) {
                        if (split_nest) {
                            split_nest = false;
                            _split(item.NestMatch, content, (item.Offset + item.OLength),
                                   (current_offset - item.CLength));
                        }

                        if (((Flags::SPLIT & expr->Flag) != 0) && !split) {
                            split = true;
                        }

                        item.Expr = expr;
                        if (expr->MatchCB == nullptr) {
                            items += static_cast<Match &&>(item);
                        } else {
                            expr->MatchCB(content, current_offset, endOffset, item, items);
                        }

                        if ((Flags::ONCE & expr->Flag) != 0) {
                            break;
                        }
                    }
                }

                offset  = current_offset;
                expr_id = 0;

                item.OLength = 0;
                item.CLength = 0;
                continue;
            }
        }

        if ((++expr_id) == exprs.Size) {
            expr_id = 0;
            ++offset;
        }
    }

    if (items.Size == 0) {
        if ((Flags::POP & exprs[0]->Flag) != 0) {
            _search(items, content, exprs[0]->NestExprs, started, endOffset, endOffset, split);
        }
    }
}
/////////////////////////////////
static Array<Match> Search(String const &content, Expressions const &exprs, UNumber const offset,
                           UNumber const limit) noexcept {
    Array<Match> items;

    UNumber const endOffset = (offset + limit);

    bool split = false;

    _search(items, content, exprs, offset, endOffset, endOffset, split);

    if (split) {
        _split(items, content, offset, endOffset);
    }

    return items;
}
/////////////////////////////////
static String Parse(String const &content, Array<Match> const &items, UNumber offset, UNumber limit,
                    void *other = nullptr) noexcept {
    Match *      item;
    UNumber      tmp_limit;
    StringStream rendered; // Final content

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

        if (item->Expr->ParseCB != nullptr) {
            if ((Flags::BUBBLE & item->Expr->Flag) != 0) {
                if (item->NestMatch.Size != 0) {
                    rendered += item->Expr->ParseCB(Parse(content, item->NestMatch, item->Offset, item->Length, other),
                                                    *item, other);
                } else {
                    rendered += item->Expr->ParseCB(String::Part(content.Str, item->Offset, item->Length), *item, other);
                }
            } else {
                rendered += item->Expr->ParseCB(content, *item, other);
            }
        } else if (item->Expr->Replace.Length != 0) {
            // Defaults to replace: it can be empty.
            rendered += item->Expr->Replace;
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
    Array<Match> splitted;
    Match        item;

    UNumber const started  = offset;
    Match *       item_ptr = nullptr;

    splitted.SetCapacity(items.Size + 1);

    for (UNumber i = 0;; i++) {
        if (i != items.Size) {
            item_ptr = &(items[i]);

            if ((Flags::SPLIT & item_ptr->Expr->Flag) == 0) {
                item.NestMatch += static_cast<Match &&>(*item_ptr);
                continue;
            }

            item.Expr   = item_ptr->Expr;
            item.Length = item_ptr->Offset - offset;
        } else {
            item.Length = endOffset - offset;
        }

        item.Offset = offset;

        offset += item.Length + item_ptr->Length;

        if ((Flags::TRIM & item.Expr->Flag) != 0) {
            String::SoftTrim(content.Str, item.Offset, item.Length);
        }

        if (((Flags::DROPEMPTY & item.Expr->Flag) == 0) || (item.Length != 0)) {
            if (item.Expr->NestExprs.Size != 0) {
                item.NestMatch = Search(content, item.Expr->NestExprs, item.Offset, item.Length);
            }

            splitted += static_cast<Match &&>(item);
        }

        if (i == items.Size) {
            break;
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
