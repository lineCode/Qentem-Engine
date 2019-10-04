
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
using _MatchCB = bool(String const &content, UNumber &offset, UNumber const endOffset, Match &item, Array<Match> &items);

using Expressions = Array<Expression *>;
/////////////////////////////////
// Expressions flags
struct Flags {
    static UShort const NOTHING    = 0;    // ... NAN.
    static UShort const NOPARSE    = 1;    // Match a Keyword but don't process it inside Parse().
    static UShort const IGNORE     = 2;    // Match a Keyword but don't add it.
    static UShort const TRIM       = 4;    // Trim the match before adding it.
    static UShort const ONCE       = 8;    // Will stop searching after the first match.
    static UShort const POP        = 16;   // Search again with NestExprs if the match fails (See ALU.cpp).
    static UShort const BUBBLE     = 32;   // Parse nested matches.
    static UShort const SPLIT      = 64;   // Split a match at a keyword.
    static UShort const SPLITNEST  = 128;  // Split a Nested match.
    static UShort const GROUPSPLIT = 256;  // Puts split matches into NestMatch, for one callback execution.
    static UShort const NOTBROKEN  = 512;  // To Prevent the engine from assuming the match is broking.
    static UShort const DROPEMPTY  = 1024; // Trim the match before adding it (spaces and newlines).
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
    UNumber Length = 0; // The length of the entire match.
    UNumber Offset = 0; // The start position of the matched string.

    UNumber OLength = 0; // Length of opening keyword.
    UNumber CLength = 0; // Length of closing keyword.

    Expression * Expr = nullptr;
    Array<Match> NestMatch; // To hold sub matches inside a match.
};
/////////////////////////////////
static void _search(Array<Match> &items, String const &content, Expressions const &exprs, UNumber offset,
                    UNumber endOffset, UNumber const maxOffset, UNumber const level) noexcept {
    bool    LOCKED      = false; // To keep matching the end of the current expression.
    bool    SPLIT_IT    = false; // To keep tracking a split match.
    bool    OVERDRIVE   = false; // For nesting.
    UNumber counter     = 0;     // Index for counting.
    UNumber nest_offset = 0;     // Temp variable for nested matches.
    UNumber id          = 1;     // Expression's id.

    Expression *ce = exprs[0];
    UNumber     end_at; // Temp offset.
    Match       _item;  // Temp match.

    UNumber const started = offset;

    for (;;) {
        if (content[offset] == ce->Keyword[0]) {
            end_at = offset;

            while (++counter < ce->Keyword.Length) {
                if (content[++end_at] != ce->Keyword[counter]) {
                    // Mismatch.
                    counter = 0;
                    break;
                }
            }

            if (counter != 0) {
                counter = 0;

                if (OVERDRIVE) {
                    // If the match is on "OVERDRIVE", then slow down.
                    OVERDRIVE = false;
                    // Set the length of the nested match.
                    endOffset = end_at;
                } else if (!LOCKED) {
                    if (ce->Connected != nullptr) {
                        _item.OLength = ((end_at + 1) - offset);
                    }

                    _item.Offset = offset;
                    offset       = end_at; // Update the position.
                    nest_offset  = end_at + 1;
                }

                ++end_at; // Next character

                if ((LOCKED = (ce->Connected != nullptr))) {
                    ce = ce->Connected;
                } else {
                    // If it's a nesting expression, search again but inside the current match.
                    if (ce->NestExprs.Index != 0) { //  && (nest_offset != end_at)
                        // Start a new search inside the current match.
                        _search(_item.NestMatch, content, ce->NestExprs, nest_offset, end_at, maxOffset, (level + 1));

                        if (_item.NestMatch.Index != 0) {
                            // Seek to avoid having the same closing/ending keywork matched again.
                            Match *tmp_item = &(_item.NestMatch[(_item.NestMatch.Index - 1)]);
                            nest_offset     = (tmp_item->Offset + tmp_item->Length);

                            if (nest_offset >= end_at) {
                                offset = nest_offset;

                                if (offset > end_at) {
                                    if (maxOffset > endOffset) {
                                        // This is important to have the search look ahead of the limited length
                                        // in order to find the entire currect match.
                                        endOffset = maxOffset; // TO THE MAX!
                                        OVERDRIVE = true;
                                    }
                                }

                                LOCKED = true;
                                continue; // Not done matching. Move to the next char.
                            }
                        }
                    }

                    if ((Flags::IGNORE & ce->Flag) == 0) {
                        _item.Length = (end_at - _item.Offset);

                        if (_item.OLength != 0) {
                            _item.CLength = (end_at - offset);

                            if ((Flags::TRIM & ce->Flag) != 0) {
                                UNumber tmpIndex = (_item.Offset + _item.OLength);
                                while ((content[tmpIndex] == L' ') || (content[tmpIndex] == L'\n') ||
                                       (content[tmpIndex] == L'\r') || (content[tmpIndex] == L'\t')) {
                                    ++_item.OLength;
                                    ++tmpIndex;
                                }

                                tmpIndex = ((_item.Offset + _item.Length) - _item.CLength);
                                if ((_item.OLength + _item.CLength) != _item.Length) {
                                    while ((content[--tmpIndex] == L' ') || (content[tmpIndex] == L'\n') ||
                                           (content[tmpIndex] == L'\r') || (content[tmpIndex] == L'\t')) {
                                        ++_item.CLength;
                                    }
                                }
                            }
                        }

                        if (((Flags::DROPEMPTY & ce->Flag) == 0) || (_item.Length != (_item.OLength + _item.CLength))) {
                            _item.Expr = ce;

                            if (level == 0) {
                                if (!SPLIT_IT && ((Flags::SPLIT & ce->Flag) != 0)) {
                                    SPLIT_IT = true;
                                }

                                if (((Flags::SPLITNEST & ce->Flag) != 0) && (_item.NestMatch.Index != 0)) {
                                    Split(_item.NestMatch, content, (_item.Offset + _item.OLength),
                                          ((_item.Offset + _item.Length) - (_item.CLength)));
                                }
                            }

                            // Adding the match
                            if (ce->MatchCB == nullptr) {
                                items += static_cast<Match &&>(_item);
                            } else if (!ce->MatchCB(content, offset, endOffset, _item, items)) {
                                // Ingnoring the match if the callback returned "false"
                                continue;
                            }

                            if ((Flags::ONCE & ce->Flag) != 0) {
                                return;
                            }
                        }
                    }

                    id = exprs.Index; // Reset expressions!
                }
            }
        }

        ++offset;

        if (!LOCKED) {
            // Switching to the next character if all keywords have been checked.
            if (id == exprs.Index) {
                id = 0;
            } else {
                --offset;
            }

            ce = exprs[id];
            ++id;
        }

        /////////////////////////////////
        if (offset >= endOffset) {
            if (!LOCKED) {
                break;
            }

            if (maxOffset > endOffset) {
                // This is important to have the seearch look ahead of the limited length
                // in order to find the entire currect match.
                endOffset = maxOffset;

                OVERDRIVE = true;
                continue;
            }

            if ((Flags::NOTBROKEN & ce->Flag) != 0) {
                // For spliting a search (resumable search).
                items += static_cast<Match &&>(_item);
                return;
            }

            // Take this path If it has gone too far (the other keyword is unmatched).
            if (_item.NestMatch.Index == 0) {
                // If it is a missmatch, go back to where it started and continue with the next expression if
                // possible.
                offset = _item.Offset;
            } else {
                // if it's a nested search... with matched items move every thing that has been found to the
                // root items' list, to avoid searching them again.

                // Seek the offset to where the last match ended.
                offset = (_item.NestMatch[(_item.NestMatch.Index - 1)].Offset +
                          _item.NestMatch[(_item.NestMatch.Index - 1)].Length);

                items += static_cast<Array<Match> &&>(_item.NestMatch);

                if (offset == endOffset) {
                    break;
                }
            }

            if (id == exprs.Index) {
                return;
            }

            // House cleaning...
            OVERDRIVE = false;
            LOCKED    = false;

            ce = exprs[id]; // Reset
        }
    }

    /////////////////////////////////
    if (items.Index == 0) {
        if ((Flags::POP & ce->Flag) != 0) {
            _search(items, content, ce->NestExprs, started, endOffset, 0, 0);
        }
        return;
    }

    if (SPLIT_IT) {
        Split(items, content, started, endOffset);
    }

    //////////////// Hani ///////////////////
    // AlJumaa, Jamada El Oula 12, 1440
    // Friday, January 18, 2019
}
/////////////////////////////////
inline static Array<Match> Search(String const &content, Expressions const &exprs, UNumber offset,
                                  UNumber limit) noexcept {
    Array<Match> items;

    UNumber end_offset = (offset + limit);

    if ((limit != 0) && (exprs.Index != 0)) {
        _search(items, content, exprs, offset, end_offset, end_offset, 0);
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

    for (UNumber id = 0; id < items.Index; id++) {
        // Current match
        _item = &(items[id]);

        if ((_item->Offset < offset) || ((Flags::NOPARSE & _item->Expr->Flag) != 0)) {
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
                if (_item->NestMatch.Index != 0) {
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

    if (items.Index == 1) {
        tmp = &items[0];
        if (((Flags::SPLITNEST & tmp->Expr->Flag) != 0) && (tmp->NestMatch.Index != 0)) {
            Split(tmp->NestMatch, content, (tmp->Offset + tmp->OLength), ((tmp->Offset + tmp->Length) - (tmp->CLength)));
            return;
        }

        if ((Flags::SPLIT & tmp->Expr->Flag) == 0) {
            return;
        }
    }

    if (items.Index == 0) {
        return;
    }

    Array<Match> splitted;
    Array<Match> nesties;
    Match        _item;
    UNumber      current_offset = offset;
    UNumber      ends;
    Expression * master = nullptr;
    Match *      tmp2   = nullptr;

    for (UNumber i = 0; i < items.Index; i++) {
        tmp        = &(items[i]);
        _item.Expr = tmp->Expr;

        if (((Flags::SPLITNEST & tmp->Expr->Flag) != 0) && (tmp->NestMatch.Index != 0)) {
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
                tmp2 = &splitted[(splitted.Index - 1)];

                if (tmp->NestMatch.Index != 0) {
                    tmp2->NestMatch = static_cast<Array<Match> &&>(tmp->NestMatch);
                } else if (nesties.Index != 0) {
                    tmp2->NestMatch = static_cast<Array<Match> &&>(nesties);
                } else if (tmp2->Expr->NestExprs.Index != 0) {
                    _search(tmp2->NestMatch, content, _item.Expr->NestExprs, _item.Offset, (_item.Offset + _item.Length),
                            0, 0);
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
        if (splitted.Index == splitted.Capacity) {
            splitted.Resize(items.Index + 1);
        }

        tmp2 = &splitted[splitted.Index];
        ++splitted.Index;
        *tmp2 = static_cast<Match &&>(_item);

        if (_item.Length != 0) {
            if (nesties.Index != 0) {
                tmp2->NestMatch = static_cast<Array<Match> &&>(nesties);
            } else if (_item.Expr->NestExprs.Index != 0) {
                _search(tmp2->NestMatch, content, _item.Expr->NestExprs, _item.Offset, (_item.Offset + _item.Length), 0,
                        0);
            }
        }
    }

    if (splitted.Index == 0) {
        items.Index = 0;
        return;
    }

    if ((master != nullptr) && (Flags::GROUPSPLIT & master->Flag) == 0) {
        items = static_cast<Array<Match> &&>(splitted);
    } else {
        items.SetCapacity(1);
        items.Index        = 1;
        items[0].Offset    = offset;
        items[0].Length    = (endOffset - offset);
        items[0].Expr      = master;
        items[0].NestMatch = static_cast<Array<Match> &&>(splitted);
    }
}
} // namespace Qentem
#endif
