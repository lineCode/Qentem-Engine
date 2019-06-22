
/**
 * Qentem Engine
 *
 * @brief     General purpose syntax parser.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#ifndef QENTEM_ENGINE_H
#define QENTEM_ENGINE_H

#include "StringStream.hpp"

namespace Qentem {
namespace Engine {

/////////////////////////////////
struct Match;
struct Expression;
/////////////////////////////////
// Expressions def
using Expressions = Array<Expression *>;
// Parse Callback
using _PARSECB = String(const String &, const Match &);

void Split(Array<Match> &, const String &, const UNumber, const UNumber) noexcept;
/////////////////////////////////
// Expressions flags
struct Flags {
    static const unsigned short NOTHING       = 0;    // ... NAN.
    static const unsigned short COMPACT       = 1;    // Processing only the content inside Keywords Parse().
    static const unsigned short NOPARSE       = 2;    // Match a Keyword but don't process it inside Parse().
    static const unsigned short IGNORE        = 4;    // Match a Keyword but don't add it.
    static const unsigned short TRIM          = 8;    // Trim the match before adding it.
    static const unsigned short ONCE          = 16;   // Will stop searching after the first match.
    static const unsigned short POP           = 32;   // Search again with NestExprs if the match fails (See ALU.cpp).
    static const unsigned short BUBBLE        = 64;   // Parse nested matches.
    static const unsigned short SPLIT         = 128;  // Split a match at a keyword.
    static const unsigned short SPLITNEST     = 256;  // Split a Nested match.
    static const unsigned short GROUPSPLIT    = 512;  // Puts split matches into NestMatch, for one callback execution.
    static const unsigned short SPLITROOTONLY = 1024; // e.g. IF-ELSE (Template.cpp)
    static const unsigned short DROPEMPTY     = 2048; // Trim the match before adding it (spaces and newlines).

    // TODO: Add Flag resume
};
/////////////////////////////////
struct Expression {
    String  Keyword = L""; // What to search for.
    UNumber Flag    = 0;   // Flags for the expression
    UNumber ID      = 0;   // Expression ID
    String  Replace;       // A text to replace the match.

    Expression *Connected = nullptr; // The next part of the match (the next keyword).
    _PARSECB *  ParseCB   = nullptr; // A callback function for custom rendering.

    Expressions NestExprs; // Expressions for nesting Search().
};
/////////////////////////////////
struct Match {
    UNumber Length = 0; // The length of the entire match.
    UNumber Offset = 0; // The start position of the matched string

    UNumber OLength = 0; // Length of opening keyword; For trim
    UNumber CLength = 0; // Length of closing keyword

    Expression * Expr = nullptr;
    Array<Match> NestMatch; // To hold sub matches inside a match.
};
/////////////////////////////////
static void _search(Array<Match> &items, const String &content, const Expressions &exprs, UNumber index, UNumber limit,
                    const UNumber max, const UNumber level) noexcept {
    bool        LOCKED      = false; // To keep matching the end of the current expression.
    bool        SPLIT_IT    = false; // To keep tracking a split match.
    bool        OVERDRIVE   = false; // To achieving nesting.
    UNumber     counter     = 0;     // Index for counting.
    UNumber     nest_offset = 0;     // Temp variable for nested matches.
    UNumber     id          = 0;     // Expression's id.
    Expression *ce          = exprs.Storage[id++];
    UNumber     end_at; // Temp offset.
    Match       _item;  // Temp match

    const UNumber started = index;

    for (;;) {
        if (content.Str[index] == ce->Keyword.Str[0]) {
            end_at = index;

            while (++counter < ce->Keyword.Length) {
                if (content.Str[++end_at] != ce->Keyword.Str[counter]) {
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
                    limit = end_at;
                } else if (!LOCKED) {
                    if (ce->Connected != nullptr) {
                        if ((Flags::TRIM & ce->Connected->Flag) != 0) {
                            // TODO: limit the loop to the max size
                            while ((content.Str[++end_at] == L' ') || (content.Str[end_at] == L'\n')) {
                            }
                            --end_at;
                        }
                        _item.OLength = ((end_at + 1) - index);
                    }

                    _item.Offset = index;
                    index        = end_at; // Update the position.
                    nest_offset  = end_at + 1;
                }

                ++end_at; // Next character

                if (ce->Connected == nullptr) {
                    // If it's a nesting expression, search again but inside the current match.
                    if ((ce->NestExprs.Size != 0) && (nest_offset != index)) {

                        // Start a new search inside the current match.
                        const UNumber _size = _item.NestMatch.Size;
                        _search(_item.NestMatch, content, ce->NestExprs, nest_offset, index, ((max == 0) ? limit : max),
                                (level + 1));

                        if (_item.NestMatch.Size != _size) {
                            if (max > limit) {
                                // This is important to have the search look ahead of the limited length
                                // in order to find the entire currect match.
                                limit     = max; // TO THE MAX!
                                OVERDRIVE = true;
                            }

                            // Seek to avoid having the same closing/ending keywork matched again.
                            nest_offset = (_item.NestMatch.Storage[(_item.NestMatch.Size - 1)].Offset +
                                           _item.NestMatch.Storage[(_item.NestMatch.Size - 1)].Length);

                            if (index < nest_offset) {
                                index = nest_offset;
                            }

                            continue; // Not done matching. Move to the next char.
                        }
                    }

                    if ((Flags::IGNORE & ce->Flag) == 0) {
                        _item.Length = (end_at - _item.Offset);

                        if (_item.OLength != 0) {
                            if (((Flags::TRIM & ce->Flag) != 0) && ((index - _item.Offset) != _item.OLength)) {
                                while ((content.Str[--index] == L' ') || (content.Str[index] == L'\n')) {
                                }
                                ++index;
                            }
                            _item.CLength = (end_at - index);
                        }

                        if (((Flags::DROPEMPTY & ce->Flag) == 0) || (_item.Length != (_item.OLength + _item.CLength))) {
                            _item.Expr = ce;

                            if (level == 0) {
                                if (!SPLIT_IT) {
                                    SPLIT_IT = ((Flags::SPLIT & ce->Flag) != 0);
                                }

                                if (((Flags::SPLITNEST & ce->Flag) != 0) && (_item.NestMatch.Size != 0)) {
                                    // This could run on its own thread.
                                    Split(_item.NestMatch, content, (_item.Offset + _item.OLength),
                                          ((_item.Offset + _item.Length) - (_item.CLength)));
                                }
                            }

                            if (items.Size == items.Capacity) {
                                items.Resize((items.Size + 1) * 4);
                            }

                            items.Storage[items.Size] = static_cast<Match &&>(_item);
                            ++items.Size;

                            // TODO: If it's a resumed match, return
                            if ((Flags::ONCE & ce->Flag) != 0) {
                                return;
                            }
                        }
                    }

                    LOCKED = false;
                    id     = exprs.Size; // Reset expressions!
                }

                if (ce->Connected != nullptr) { // This way is faster than using "else"
                    ce     = ce->Connected;
                    LOCKED = true; // Locks the engine from swiching expretions.
                }
            }
        }

        /////////////////////////////////
        ++index;

        if (!LOCKED) {
            // Switching to the next character if all keywords have been checked.
            if (id == exprs.Size) {
                id = 0;
            } else {
                --index;
            }

            ce = exprs.Storage[id++];
        }

        /////////////////////////////////
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

            // Take this path If it has gone too far (the other keyword is unmatched).

            // TODO: Implement resume to preserve matches and complete matching later (Multi threading)
            if (_item.NestMatch.Size == 0) {
                // If it is a missmatch, go back to where it started and continue with the next expression if
                // possible.
                index = _item.Offset;
            } else {
                // if it's a nested search... with matched items move every thing that has been found to the
                // root items' list, to avoid searching them again.

                // Seek the offset to where the last match ended.
                index = (_item.NestMatch.Storage[(_item.NestMatch.Size - 1)].Offset +
                         _item.NestMatch.Storage[(_item.NestMatch.Size - 1)].Length);

                items.Add(_item.NestMatch, true);

                if (index == limit) {
                    break;
                }
            }

            if (id == exprs.Size) {
                return;
            }

            // House cleaning...
            OVERDRIVE = false;
            LOCKED    = false;

            ce = exprs.Storage[id]; // Reset
        }
    }

    /////////////////////////////////

    if (((Flags::POP & ce->Flag) != 0) && (items.Size == 0)) {
        _search(items, content, ce->NestExprs, started, limit, 0, 0);
        return;
    }

    if (SPLIT_IT) {
        Split(items, content, started, limit);
    }

    //////////////// Hani ///////////////////
    // AlJumaa, Jamada El Oula 12, 1440
    // Friday, January 18, 2019
}
/////////////////////////////////
inline static Array<Match> Search(const String &content, const Expressions &exprs, UNumber index = 0, UNumber length = 0,
                                  UNumber max = 0) noexcept {
    Array<Match> items;

    if (length == 0) {
        length = (content.Length - index); // limit becomes the ending offset here.
    }

    if ((content.Length == 0) || (index >= length)) {
        return items;
    }

    _search(items, content, exprs, index, length, max, 0);

    return items;
}
/////////////////////////////////
static String Parse(const String &content, const Array<Match> &items, UNumber index = 0, UNumber length = 0) noexcept {
    if (length == 0) {
        length = (content.Length - index);
    }

    if (index >= length) {
        return content;
    }
    // Note: Do not return on an empty match as some content is limitted by "index" and length. It should always return
    // part of the string or a copy of it)
    StringStream rendered; // Final content
    UNumber      offset;
    UNumber      end_offset;
    Match *      _item;

    for (UNumber id = 0; id < items.Size; id++) {
        // Current match
        _item = &(items.Storage[id]);

        if ((Flags::NOPARSE & _item->Expr->Flag) != 0) {
            continue;
        }

        if ((Flags::COMPACT & _item->Expr->Flag) == 0) {
            offset     = _item->Offset;
            end_offset = _item->Length;
        } else {
            offset     = (_item->Offset + _item->OLength);
            end_offset = (_item->Length - (_item->OLength + _item->CLength));
        }

        // Add any content that comes before...
        if ((index < offset)) {
            rendered += String::Part(content, index, (offset - index));
        }

        if (_item->Expr->ParseCB != nullptr) {
            if ((Flags::BUBBLE & _item->Expr->Flag) != 0) {
                if (_item->NestMatch.Size != 0) {
                    rendered +=
                        _item->Expr->ParseCB(Parse(content, _item->NestMatch, offset, (offset + end_offset)), *_item);
                } else {
                    rendered += _item->Expr->ParseCB(String::Part(content, offset, end_offset), *_item);
                }
            } else {
                rendered += _item->Expr->ParseCB(content, *_item);
            }
        } else if (_item->Expr->Replace.Length != 0) {
            // Defaults to replace: it might be an empty string.
            rendered += _item->Expr->Replace;
        }

        index = offset + end_offset;
    }

    if (index != 0) {
        if (index < length) {
            // Adding the remaining of the text to the final rendered content.
            rendered += String::Part(content, index, (length - index));
        }

        return rendered.Eject();
    }

    // If there is no changes were made, return the original content.
    return content;
}
} // namespace Engine

/////////////////////////////////
void Engine::Split(Array<Match> &items, const String &content, const UNumber index, const UNumber to) noexcept {
    Match *tmp = nullptr;

    if (items.Size == 1) {
        tmp = &items.Storage[0];
        if (((Flags::SPLITNEST & tmp->Expr->Flag) != 0) && (tmp->NestMatch.Size != 0)) {
            Engine::Split(tmp->NestMatch, content, (tmp->Offset + tmp->OLength),
                          ((tmp->Offset + tmp->Length) - (tmp->CLength)));
            return;
        }

        if ((Flags::SPLIT & tmp->Expr->Flag) == 0) {
            return;
        }
    }

    Array<Match> splitted;
    Array<Match> nesties;
    Match        _item;
    UNumber      offset = index;
    UNumber      ends;
    Expression * master = nullptr;
    Match *      tmp2   = nullptr;

    for (UNumber i = 0; i < items.Size; i++) {
        tmp        = &(items.Storage[i]);
        _item.Expr = tmp->Expr;

        if (((Flags::SPLITNEST & tmp->Expr->Flag) != 0) && (tmp->NestMatch.Size != 0)) {
            Engine::Split(tmp->NestMatch, content, (tmp->Offset + tmp->OLength),
                          ((tmp->Offset + tmp->Length) - (tmp->CLength)));
        }

        if ((Flags::SPLIT & tmp->Expr->Flag) != 0) {
            _item.Offset = offset;

            if ((Flags::TRIM & _item.Expr->Flag) != 0) {
                while ((content.Str[_item.Offset] == L' ') || (content.Str[_item.Offset] == L'\n')) {
                    ++_item.Offset;
                }

                _item.Length = (tmp->Offset - _item.Offset);
                ends         = tmp->Offset;

                if (_item.Length != 0) {
                    while ((content.Str[--ends] == L' ') || (content.Str[ends] == L'\n')) {
                    }
                    ++ends;
                    _item.Length = (ends - _item.Offset);
                }
            } else {
                _item.Length = (tmp->Offset - _item.Offset);
            }

            master = _item.Expr;

            if (((Flags::DROPEMPTY & _item.Expr->Flag) == 0) || (_item.Length != 0)) {
                if (splitted.Size == splitted.Capacity) {
                    splitted.Resize((splitted.Size + 1) * 4);
                }

                tmp2 = &splitted.Storage[splitted.Size];
                ++splitted.Size;
                *tmp2 = static_cast<Match &&>(_item);

                if (tmp->NestMatch.Size != 0) {
                    tmp2->NestMatch = static_cast<Array<Match> &&>(tmp->NestMatch);
                } else if (nesties.Size != 0) {
                    tmp2->NestMatch = static_cast<Array<Match> &&>(nesties);
                } else if (tmp2->Expr->NestExprs.Size != 0) {
                    Engine::_search(tmp2->NestMatch, content, _item.Expr->NestExprs, _item.Offset,
                                    (_item.Offset + _item.Length), 0, 0);
                }
            }
            offset = (tmp->Offset + tmp->Length);
        } else {
            if (nesties.Size == nesties.Capacity) {
                nesties.Resize((nesties.Size + 1) * 4);
            }

            nesties.Storage[nesties.Size] = static_cast<Match &&>(*tmp);
            ++nesties.Size;
        }
    }

    _item.Offset = offset;
    if ((Flags::TRIM & _item.Expr->Flag) != 0) {
        while ((content.Str[_item.Offset] == L' ') || (content.Str[_item.Offset] == L'\n')) {
            ++_item.Offset;
        }

        _item.Length = (to - _item.Offset);
        ends         = to;

        if (_item.Length != 0) {
            while ((content.Str[--ends] == L' ') || (content.Str[ends] == L'\n')) {
            }
            ++ends;
            _item.Length = (ends - _item.Offset);
        }
    } else {
        _item.Length = (to - _item.Offset);
    }

    if (((Flags::DROPEMPTY & _item.Expr->Flag) == 0) || (_item.Length != 0)) {
        if (splitted.Size == splitted.Capacity) {
            splitted.Resize((items.Size + 1) * 4);
        }

        tmp2 = &splitted.Storage[splitted.Size];
        ++splitted.Size;
        *tmp2 = static_cast<Match &&>(_item);

        if (_item.Length != 0) {
            if (nesties.Size != 0) {
                tmp2->NestMatch = static_cast<Array<Match> &&>(nesties);
            } else if (_item.Expr->NestExprs.Size != 0) {
                Engine::_search(tmp2->NestMatch, content, _item.Expr->NestExprs, _item.Offset,
                                (_item.Offset + _item.Length), 0, 0);
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
        items.Size = 1;
        tmp        = &items.Storage[0];

        tmp->Offset    = index;
        tmp->Length    = (to - index);
        tmp->Expr      = master;
        tmp->NestMatch = static_cast<Array<Match> &&>(splitted);
    }
}

} // namespace Qentem

#endif
