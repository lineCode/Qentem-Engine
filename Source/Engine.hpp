
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
static void Split(Array<Match> &items, String const &content, UNumber offset, UNumber endOffset) noexcept;
using _ParseCB = String(String const &content, Match const &item, void *other);
using _MatchCB = void(String const &content, UNumber &offset, UNumber const endOffset, Match &item, Array<Match> &items);

using Expressions = Array<Expression *>;
/////////////////////////////////
// Expressions flags
struct Flags {
    static UShort const NOTHING    = 0;   // ... NAN.
    static UShort const IGNORE     = 1;   // Match a Keyword but don't add it.
    static UShort const TRIM       = 2;   // Trim the match before adding it.
    static UShort const ONCE       = 4;   // Will stop searching after the first match.
    static UShort const POP        = 8;   // Search again with NestExprs if the match fails (See ALU.cpp).
    static UShort const BUBBLE     = 16;  // Parse nested matches.
    static UShort const SPLIT      = 32;  // Split a match at a keyword.
    static UShort const SPLITNEST  = 64;  // Split a Nested match.
    static UShort const GROUPSPLIT = 128; // Puts split matches into NestMatch, for one callback execution.
    static UShort const NOTBROKEN  = 256; // To Prevent the engine from assuming the match is broking.
    static UShort const DROPEMPTY  = 512; // Trim the match before adding it (spaces and newlines).
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
                    UNumber endOffset, UNumber const maxOffset) noexcept {
    UNumber const started        = offset;
    UNumber       current_offset = offset;

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
                keyword_offset     = 0;

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
                        if ((expr->NestExprs.Size == 0) ||
                            (current_offset == (item.Offset + item.OLength + keyword_offset))) {
                            MATCHED = true;
                            break;
                        }

                        // if (sub_offset < current_offset)
                        _search(item.NestMatch, content, expr->NestExprs, sub_offset, current_offset, maxOffset);

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
            _search(items, content, exprs[0]->NestExprs, started, endOffset, endOffset);
        }
    }
}
/////////////////////////////////
static Array<Match> Search(String const &content, Expressions const &exprs, UNumber const offset,
                           UNumber const limit) noexcept {
    Array<Match> items;

    UNumber const endOffset = (offset + limit);

    if (exprs.Size != 0) {
        _search(items, content, exprs, offset, endOffset, endOffset);

        if (items.Size != 0) {
            Match *item;
            bool   SPLIT = false;

            for (UNumber i = 0; i < items.Size; i++) {
                item = &(items[i]);
                if (!SPLIT && ((Flags::SPLIT & item->Expr->Flag) != 0)) {
                    SPLIT = true;
                }

                if ((item->NestMatch.Size != 0) && ((Flags::SPLITNEST & item->Expr->Flag) != 0)) {
                    Split(item->NestMatch, content, (item->Offset + item->OLength),
                          ((item->Offset + item->Length) - (item->CLength)));
                }
            }

            if (SPLIT) {
                Split(items, content, offset, endOffset);
            }
        }
    }

    return items;
}
/////////////////////////////////
static String Parse(String const &content, Array<Match> const &items, UNumber offset, UNumber limit,
                    void *other = nullptr) noexcept {
    if (limit == 0) {
        return content;
    }

    Match *      _item;
    UNumber      tmp_limit;
    StringStream rendered; // Final content

    for (UNumber id = 0; id < items.Size; id++) {
        // Current match
        _item = &(items[id]);

        if (_item->Offset < offset) {
            continue;
        }

        // Adding any content that comes before...
        if (offset < _item->Offset) {
            tmp_limit = (_item->Offset - offset);

            if (tmp_limit >= limit) {
                // If it's equal, then String::Part outside the for() will handel it.
                break;
            }

            limit -= tmp_limit;

            rendered += String::Part(content.Str, offset, tmp_limit);
        }

        if (_item->Expr->ParseCB != nullptr) {
            if ((Flags::BUBBLE & _item->Expr->Flag) != 0) {
                if (_item->NestMatch.Size != 0) {
                    rendered += _item->Expr->ParseCB(
                        Parse(content, _item->NestMatch, _item->Offset, _item->Length, other), *_item, other);
                } else {
                    rendered +=
                        _item->Expr->ParseCB(String::Part(content.Str, _item->Offset, _item->Length), *_item, other);
                }
            } else {
                rendered += _item->Expr->ParseCB(content, *_item, other);
            }
        } else if (_item->Expr->Replace.Length != 0) {
            // Defaults to replace: it can be empty.
            rendered += _item->Expr->Replace;
        }

        offset = _item->Offset + _item->Length;
        limit -= _item->Length;
    }

    if (limit != 0) {
        rendered += String::Part(content.Str, offset, limit);
    }

    return rendered.Eject();
}
} // namespace Engine
/////////////////////////////////
static void Engine::Split(Array<Match> &items, String const &content, UNumber const offset,
                          UNumber const endOffset) noexcept {
    Match *tmp = nullptr;

    if (items.Size == 1) {
        tmp = &items[0];
        if (((Flags::SPLITNEST & tmp->Expr->Flag) != 0) && (tmp->NestMatch.Size != 0)) {
            Split(tmp->NestMatch, content, (tmp->Offset + tmp->OLength), ((tmp->Offset + tmp->Length) - (tmp->CLength)));
            return;
        }

        if ((Flags::SPLIT & tmp->Expr->Flag) == 0) {
            return;
        }
    }

    Array<Match> splitted;
    Array<Match> nesties;
    Match        _item;
    UNumber      current_offset = offset;
    UNumber      ends;
    Expression * master = nullptr;
    Match *      tmp2   = nullptr;

    for (UNumber i = 0; i < items.Size; i++) {
        tmp        = &(items[i]);
        _item.Expr = tmp->Expr;

        if (((Flags::SPLITNEST & tmp->Expr->Flag) != 0) && (tmp->NestMatch.Size != 0)) {
            Split(tmp->NestMatch, content, (tmp->Offset + tmp->OLength), ((tmp->Offset + tmp->Length) - (tmp->CLength)));
        }

        if ((Flags::SPLIT & tmp->Expr->Flag) != 0) {
            _item.Offset = current_offset;

            if ((Flags::TRIM & _item.Expr->Flag) != 0) {
                while ((content[_item.Offset] == L' ') || (content[_item.Offset] == L'\n')) {
                    ++_item.Offset;
                }

                _item.Length = (tmp->Offset - _item.Offset);
                ends         = tmp->Offset;

                if (_item.Length != 0) {
                    while ((content[--ends] == L' ') || (content[ends] == L'\n')) {
                    }
                    ++ends;
                    _item.Length = (ends - _item.Offset);
                }
            } else {
                _item.Length = (tmp->Offset - _item.Offset);
            }

            master = _item.Expr;

            if (((Flags::DROPEMPTY & _item.Expr->Flag) == 0) || (_item.Length != 0)) {
                splitted += static_cast<Match &&>(_item);
                tmp2 = &splitted[(splitted.Size - 1)];

                if (tmp->NestMatch.Size != 0) {
                    tmp2->NestMatch = static_cast<Array<Match> &&>(tmp->NestMatch);
                } else if (nesties.Size != 0) {
                    tmp2->NestMatch = static_cast<Array<Match> &&>(nesties);
                } else if (tmp2->Expr->NestExprs.Size != 0) {
                    tmp2->NestMatch = Search(content, _item.Expr->NestExprs, _item.Offset, _item.Length);
                }
            }
            current_offset = (tmp->Offset + tmp->Length);
        } else {
            nesties += static_cast<Match &&>(*tmp);
        }
    }

    _item.Offset = current_offset;
    if ((Flags::TRIM & _item.Expr->Flag) != 0) {
        while ((content[_item.Offset] == L' ') || (content[_item.Offset] == L'\n')) {
            ++_item.Offset;
        }

        _item.Length = (endOffset - _item.Offset);
        ends         = endOffset;

        if (_item.Length != 0) {
            while ((content[--ends] == L' ') || (content[ends] == L'\n')) {
            }
            ++ends;
            _item.Length = (ends - _item.Offset);
        }
    } else {
        _item.Length = (endOffset - _item.Offset);
    }

    if (((Flags::DROPEMPTY & _item.Expr->Flag) == 0) || (_item.Length != 0)) {
        if (splitted.Size == splitted.Capacity) {
            splitted.Resize(items.Size + 1);
        }

        tmp2 = &splitted[splitted.Size];
        ++splitted.Size;
        *tmp2 = static_cast<Match &&>(_item);

        if (_item.Length != 0) {
            if (nesties.Size != 0) {
                tmp2->NestMatch = static_cast<Array<Match> &&>(nesties);
            } else if (_item.Expr->NestExprs.Size != 0) {
                tmp2->NestMatch = Search(content, _item.Expr->NestExprs, _item.Offset, _item.Length);
            }
        }
    }

    if (splitted.Size == 0) {
        items.Size = 0;
        return;
    }

    if ((master != nullptr) && (Flags::GROUPSPLIT & master->Flag) == 0) {
        items = static_cast<Array<Match> &&>(splitted);
    } else {
        items.SetCapacity(1);
        items.Size         = 1;
        items[0].Offset    = offset;
        items[0].Length    = (endOffset - offset);
        items[0].Expr      = master;
        items[0].NestMatch = static_cast<Array<Match> &&>(splitted);
    }
}
} // namespace Qentem
#endif
