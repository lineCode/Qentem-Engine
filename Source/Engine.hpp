
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
using _MatchCB = bool(String const &content, UNumber &offset, UNumber const endOffset, Match &item, Array<Match> &items,
                      UNumber const level);

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
    UNumber Length = 0; // The length of the entire match.
    UNumber Offset = 0; // The start position of the matched string.

    UNumber OLength = 0; // Length of opening keyword.
    UNumber CLength = 0; // Length of closing keyword.

    Expression * Expr = nullptr;
    Array<Match> NestMatch; // To hold sub matches inside a match.
};
/////////////////////////////////
static void _search(Array<Match> &items, String const &content, Expressions const &exprs, UNumber offset,
                    UNumber endOffset, UNumber const maxOffset, UNumber const level = 0) noexcept {
    bool    LOCKED      = false; // To keep matching the end of the current expression.
    bool    SPLIT_IT    = false; // To keep tracking a split match.
    bool    OVERDRIVE   = false; // For nesting.
    UNumber counter     = 0;
    UNumber nest_offset = 0; // Tmp variable for nested matches.
    UNumber id          = 1; // Expression's id.

    Expression *ce = exprs[0];
    UNumber     tmp_offset; // Tmp offset.
    Match       _item;      // Tmp match.

    UNumber const started = offset;

    for (;;) {
        if (content[offset] == ce->Keyword[0]) {
            tmp_offset = offset;

            while (++counter < ce->Keyword.Length) {
                if (content[++tmp_offset] != ce->Keyword[counter]) {
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
                    endOffset = tmp_offset;
                } else if (!LOCKED) {
                    if (ce->Connected != nullptr) {
                        _item.OLength = ((tmp_offset + 1) - offset);
                    }

                    _item.Offset = offset;
                    offset       = tmp_offset; // Update the position.
                    nest_offset  = tmp_offset + 1;
                }

                ++tmp_offset; // Next character

                if ((LOCKED = (ce->Connected != nullptr))) {
                    ce = ce->Connected;
                } else {
                    // If it's a nesting expression, search again but inside the current match.
                    if ((ce->NestExprs.Size != 0) && (nest_offset != tmp_offset)) {
                        // Start a new search inside the current match.
                        _search(_item.NestMatch, content, ce->NestExprs, nest_offset, tmp_offset, maxOffset,
                                (level + 1));

                        if (_item.NestMatch.Size != 0) {
                            // Seek to avoid having the same closing/ending keywork matched again.
                            Match *tmp_item = &(_item.NestMatch[(_item.NestMatch.Size - 1)]);
                            nest_offset     = (tmp_item->Offset + tmp_item->Length);

                            if (nest_offset >= tmp_offset) {
                                if ((nest_offset != tmp_offset) && (maxOffset > endOffset)) {
                                    // This is important to have the search look ahead of the limited length
                                    // in order to find the entire currect match.
                                    endOffset = maxOffset; // TO THE MAX!
                                    OVERDRIVE = true;
                                }

                                offset = nest_offset;
                                LOCKED = true;

                                continue; // Not done yet, so ove to the next char.
                            }
                        }
                    }

                    if ((Flags::IGNORE & ce->Flag) == 0) {
                        _item.Length = (tmp_offset - _item.Offset);

                        if (_item.OLength != 0) {
                            _item.CLength = (tmp_offset - offset);

                            if ((Flags::TRIM & ce->Flag) != 0) {
                                UNumber tmpIndex = (_item.Offset + _item.OLength);
                                while ((content[tmpIndex] == L' ') || (content[tmpIndex] == L'\n') ||
                                       (content[tmpIndex] == L'\t') || (content[tmpIndex] == L'\r')) {
                                    ++_item.OLength;
                                    ++tmpIndex;
                                }

                                tmpIndex = ((_item.Offset + _item.Length) - _item.CLength);
                                if ((_item.OLength + _item.CLength) != _item.Length) {
                                    while ((content[--tmpIndex] == L' ') || (content[tmpIndex] == L'\n') ||
                                           (content[tmpIndex] == L'\t') || (content[tmpIndex] == L'\r')) {
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

                                if (((Flags::SPLITNEST & ce->Flag) != 0) && (_item.NestMatch.Size != 0)) {
                                    Split(_item.NestMatch, content, (_item.Offset + _item.OLength),
                                          ((_item.Offset + _item.Length) - (_item.CLength)));
                                }
                            }

                            // Adding the match
                            if (ce->MatchCB == nullptr) {
                                items += static_cast<Match &&>(_item);
                            } else if (!ce->MatchCB(content, offset, endOffset, _item, items, level)) {
                                // Ingnoring the match if the callback returned "false"
                                continue;
                            }

                            if ((Flags::ONCE & ce->Flag) != 0) {
                                break;
                            }
                        }
                    }

                    id = exprs.Size; // Reset expressions!
                }
            }
        }

        ++offset;

        if (!LOCKED) {
            // Switching to the next character if all keywords have been checked.
            if (id == exprs.Size) {
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
            if (_item.NestMatch.Size == 0) {
                // If it is a missmatch, go back to where it started and continue with the next expression if
                // possible.
                offset = _item.Offset;
            } else {
                // if it's a nested search... with matched items move every thing that has been found to the
                // root items' list, to avoid searching them again.

                // Seek the offset to where the last match ended.
                offset = (_item.NestMatch[(_item.NestMatch.Size - 1)].Offset +
                          _item.NestMatch[(_item.NestMatch.Size - 1)].Length);

                items += static_cast<Array<Match> &&>(_item.NestMatch);

                if (offset == endOffset) {
                    break;
                }
            }

            if (id == exprs.Size) {
                break;
            }

            // House cleaning...
            OVERDRIVE = false;
            LOCKED    = false;

            ce = exprs[id]; // Reset
        }
    }

    /////////////////////////////////
    if (items.Size == 0) {
        if ((Flags::POP & ce->Flag) != 0) {
            _search(items, content, ce->NestExprs, started, endOffset, endOffset);
        }
    } else if (SPLIT_IT) {
        Split(items, content, started, endOffset);
    }

    //////////////// Hani ///////////////////
    // AlJumaa, Jamada El Oula 12, 1440
    // Friday, January 18, 2019
}
/////////////////////////////////
static Array<Match> Search(String const &content, Expressions const &exprs, UNumber const offset,
                           UNumber const limit) noexcept {
    Array<Match> items;

    UNumber const end_offset = (offset + limit);

    if ((limit != 0) && (exprs.Size != 0)) {
        _search(items, content, exprs, offset, end_offset, end_offset);
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

    if (items.Size == 0) {
        return;
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
