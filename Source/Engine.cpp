
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

Array<Match> Qentem::Engine::Search(const String &content, const Expressions &exprs, UNumber index, UNumber length,
                                    UNumber max) noexcept {
    Array<Match> items;

    if (length == 0) {
        length = (content.Length - index); // limit becomes the ending offset here.
    }

    if ((content.Length == 0) || (exprs.Size == 0) || (index >= length)) {
        return items;
    }

    Engine::_search(items, content, exprs, index, length, max, 0);

    return items;
}

void Qentem::Engine::_search(Array<Match> &items, const String &content, const Expressions &exprs, UNumber index,
                             UNumber limit, UNumber max, const UNumber level) noexcept {
    bool    LOCKED    = false; // To keep matching the end of the current expression.
    bool    SPLIT     = false; // To keep tracking a split match.
    bool    OVERDRIVE = false; // To achieving nesting.
    UNumber counter   = 0;     // Index for counting.

    Match _item;

    const UNumber started = index;

    UNumber id = 0; // Expression's id.
    // Seting the value of the current expression.
    Expression *ce = exprs.Storage[id++];

    UNumber end_at;      // Temp offset.
    UNumber nest_offset; // Temp variable for nested matches.
    while (true) {
        if (ce->SearchCB == nullptr) { // Note: This condition cost 6% of the execution.
            if (content.Str[index] == ce->Keyword.Str[counter]) {
                end_at = index;
                while (++counter < ce->Keyword.Length) {
                    // Loop through every character.
                    if (content.Str[++end_at] != ce->Keyword.Str[counter]) {
                        // Mismatch.
                        counter = 0;
                        break;
                    }
                }
            }
        } else {
            counter = ce->SearchCB(content, *ce, &_item, index, end_at, limit);
        }

        if (counter != 0) {
            counter = 0;
            end_at += 1;

            if (OVERDRIVE) {
                // If the match is on "OVERDRIVE", then break.
                OVERDRIVE = false;
                // Set the length of the nesting match.
                limit = (end_at - 1);
            } else if (!LOCKED) {
                if (ce->Connected != nullptr) {
                    if ((Flags::TRIM & ce->Connected->Flag) != 0) {
                        while ((content.Str[end_at] == L' ') || (content.Str[end_at] == L'\n')) {
                            end_at++;
                        }
                    }
                    _item.OLength = (end_at - index);
                }

                _item.Offset = index;
                index        = (end_at - 1); // Update the possession
                nest_offset  = end_at;
            }

            if (ce->Connected == nullptr) {
                // If it's a nesting expression, search again but inside the current match.
                if ((ce->NestExprs.Size != 0) && (nest_offset != index)) {
                    // Start a new search inside the current match.
                    UNumber _size = _item.NestMatch.Size;
                    // TODO: Use local var to prevent _item from expanding very often.
                    Engine::_search(_item.NestMatch, content, ce->NestExprs, nest_offset, index,
                                    ((max != 0) ? max : limit), (level + 1));

                    if (_item.NestMatch.Size != _size) {
                        if (max > limit) {
                            // This is important to have the search look ahead of the limited length
                            // in order to find the entire currect match.
                            limit     = max; // TO THE MAX!
                            OVERDRIVE = true;
                        }

                        // Seek to avoid having the same closing/ending keywork matched again.
                        index = nest_offset = (_item.NestMatch.Storage[(_item.NestMatch.Size - 1)].Offset +
                                               _item.NestMatch.Storage[(_item.NestMatch.Size - 1)].Length);

                        continue; // Not done matching. Move to the next char.
                    }
                }

                if ((Flags::IGNORE & ce->Flag) == 0) {
                    _item.Length = (end_at - _item.Offset);

                    if (_item.OLength != 0) {
                        if (((Flags::TRIM & ce->Flag) != 0) && ((index - _item.Offset) != _item.OLength)) {
                            while ((content.Str[--index] == L' ') || (content.Str[index] == L'\n')) {
                            }
                            index++;
                        }
                        _item.CLength = (end_at - index);
                    }

                    if (((Flags::DROPEMPTY & ce->Flag) == 0) || (_item.Length != (_item.OLength + _item.CLength))) {
                        _item.Expr = ce;

                        if ((Flags::SPLIT & ce->Flag) != 0) {
                            SPLIT = true;
                        }

                        if (ce->SubExprs.Size != 0) {
                            // This could run on its own thread.
                            Engine::_search(_item.SubMatch, content, ce->SubExprs, _item.Offset,
                                            (_item.Offset + _item.Length), 0, 0);
                        }

                        // TODO: Reevaluate
                        if ((level == 0) && (_item.NestMatch.Size != 0) &&
                            (((Flags::SPLITNEST & ce->Flag) != 0) || ((Flags::SPLITROOTONLY & ce->Flag) != 0))) {
                            // This could run on its own thread.
                            Engine::Split(content, _item.NestMatch, (_item.Offset + _item.OLength),
                                          ((_item.Offset + _item.Length) - (_item.CLength)));
                        }

                        if (items.Size == items.Capacity) {
                            items.ExpandTo((items.Size == 0 ? 3 : (items.Size + 7)));
                        }

                        items.Storage[items.Size] = static_cast<Match &&>(_item);
                        items.Size++;

                        if ((Flags::ONCE & ce->Flag) != 0) {
                            return;
                        }
                    }
                }

                LOCKED = false;
                id     = exprs.Size; // Reset expressions!
            } else {
                ce = ce->Connected;

                if (LOCKED) {
                    _item.OLength = 0;
                } else {
                    LOCKED = true;
                }
            }
        }

        if (LOCKED) {
            // If there is an ongoing match, then move to the next char.
            ++index;
        } else {
            // Switching to the next character if all keywords have been checked.
            if (id == exprs.Size) {
                id = 0;
                ++index;
            }
            // Seting the next keyword for searching.
            ce = exprs.Storage[id++];
        }

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

            // If it has gone too far (linked keyword is missing).
            if (_item.NestMatch.Size == 0) {
                // If it is a missmatch, go back to where it started and continue with the next expression if
                // possible.
                index = _item.Offset;
            } else {
                // if it's a nested search... with matched items move every thing that has been found to the
                // main items' list, to avoid searching them again.
                items.Add(_item.NestMatch, true);

                Match *p_item = &(_item.NestMatch.Storage[(_item.NestMatch.Size - 1)]);
                // Seek the offset to where the last match ended.
                index = p_item->Offset + p_item->Length;

                if (index == limit) {
                    break;
                }

                _item.NestMatch.Clear();
            }

            if (id == exprs.Size) {
                return;
            }

            // House cleaning...
            OVERDRIVE = false;
            LOCKED    = false;

            ce = exprs.Storage[id];
        }
    }

    if (items.Size == 0) {
        if ((Flags::POP & ce->Flag) != 0) {
            Engine::_search(items, content, ce->NestExprs, started, limit, 0, 0);
        }
    } else if ((level == 0) && SPLIT) {
        Engine::Split(content, items, started, limit);
    }

    // AlJumaa, Jamada El Oula 12, 1440
    // Friday, January 18, 2019
}

void Qentem::Engine::Split(const String &content, Array<Match> &items, const UNumber index, const UNumber to) noexcept {
    Match *tmp = nullptr;

    if (items.Size == 1) {
        tmp = &items.Storage[0];
        if (((Flags::SPLITNEST & tmp->Expr->Flag) != 0) && (tmp->NestMatch.Size != 0)) {
            Engine::Split(content, tmp->NestMatch, (tmp->Offset + tmp->OLength),
                          ((tmp->Offset + tmp->Length) - (tmp->CLength)));
            return;
        }

        if ((Flags::SPLIT & tmp->Expr->Flag) == 0) {
            return;
        }
    }

    Array<Match> splitted;
    splitted.SetCapacity(items.Size + 1);
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
            Engine::Split(content, tmp->NestMatch, (tmp->Offset + tmp->OLength),
                          ((tmp->Offset + tmp->Length) - (tmp->CLength)));
        }

        if ((Flags::SPLIT & tmp->Expr->Flag) != 0) {
            _item.Offset = offset;

            if ((Flags::TRIM & _item.Expr->Flag) != 0) {
                while ((content.Str[_item.Offset] == L' ') || (content.Str[_item.Offset] == L'\n')) {
                    _item.Offset++;
                }

                _item.Length = (tmp->Offset - _item.Offset);
                ends         = tmp->Offset;

                if (_item.Length != 0) {
                    while ((content.Str[--ends] == L' ') || (content.Str[ends] == L'\n')) {
                    }
                    ends++;
                    _item.Length = (ends - _item.Offset);
                }
            } else {
                _item.Length = (tmp->Offset - _item.Offset);
            }

            _item.Tag = tmp->Tag;
            _item.ID  = tmp->ID;
            master    = _item.Expr;

            if (((Flags::DROPEMPTY & _item.Expr->Flag) == 0) || (_item.Length != 0)) {
                if (splitted.Size == splitted.Capacity) {
                    splitted.ExpandTo((splitted.Size == 0 ? 3 : (splitted.Size * 2)));
                }

                tmp2 = &splitted.Storage[splitted.Size];
                splitted.Size++;
                *tmp2 = static_cast<Match &&>(_item);

                if (tmp->NestMatch.Size != 0) {
                    tmp2->NestMatch = static_cast<Array<Match> &&>(tmp->NestMatch);
                } else if (nesties.Size != 0) {
                    tmp2->NestMatch = static_cast<Array<Match> &&>(nesties);
                } else if (tmp2->Expr->NestExprs.Size != 0) {
                    Engine::_search(tmp2->NestMatch, content, _item.Expr->NestExprs, _item.Offset,
                                    (_item.Offset + _item.Length), 0, 0);
                }

                if (tmp->SubMatch.Size != 0) {
                    _item.SubMatch = static_cast<Array<Match> &&>(tmp->SubMatch);
                }
            }
            offset = (tmp->Offset + tmp->Length);
        } else {
            if (nesties.Size == nesties.Capacity) {
                nesties.ExpandTo((nesties.Size == 0 ? 3 : (nesties.Size * 2)));
            }

            nesties.Storage[nesties.Size] = static_cast<Match &&>(*tmp);
            nesties.Size++;
        }
    }

    _item.Offset = offset;
    if ((Flags::TRIM & _item.Expr->Flag) != 0) {
        while ((content.Str[_item.Offset] == L' ') || (content.Str[_item.Offset] == L'\n')) {
            _item.Offset++;
        }

        _item.Length = (to - _item.Offset);
        ends         = to;

        if (_item.Length != 0) {
            while ((content.Str[--ends] == L' ') || (content.Str[ends] == L'\n')) {
            }
            ends++;
            _item.Length = (ends - _item.Offset);
        }
    } else {
        _item.Length = (to - _item.Offset);
    }

    if (((Flags::DROPEMPTY & _item.Expr->Flag) == 0) || (_item.Length != 0)) {
        if (splitted.Size == splitted.Capacity) {
            splitted.ExpandTo((splitted.Size == 0 ? 3 : (splitted.Size * 2)));
        }

        if (tmp->SubMatch.Size != 0) {
            _item.SubMatch = static_cast<Array<Match> &&>(tmp->SubMatch);
        }

        tmp2 = &splitted.Storage[splitted.Size];
        splitted.Size++;
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
        items.Clear();
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
        tmp->ID        = _item.ID;
        tmp->Tag       = _item.Tag;
        tmp->NestMatch = static_cast<Array<Match> &&>(splitted);
    }
}

String Qentem::Engine::Parse(const String &content, const Array<Match> &items, UNumber index, UNumber length) noexcept {
    if (length == 0) {
        length = (content.Length - index);
    }

    if (index >= length) {
        return content;
    }
    // Note: Do not return on an empty match as some content is limitted by "index" and length I(will always return part
    // of the string or a copy of it)
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
                    rendered += _item->Expr->ParseCB(
                        Engine::Parse(content, _item->NestMatch, offset, (offset + end_offset)), *_item);
                } else {
                    rendered += _item->Expr->ParseCB(String::Part(content, offset, end_offset), *_item);
                }
            } else {
                rendered += _item->Expr->ParseCB(content, *_item);
            }
        } else {
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
