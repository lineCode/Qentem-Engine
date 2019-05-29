
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
                                    UNumber max) noexcept {
    Array<Match> items;

    if (limit == 0) {
        limit = (content.Length - index); // limit becomes the ending offset here.
    }

    if ((content.Length == 0) || (exprs.Size == 0) || (index >= limit)) {
        return items;
    }

    Engine::_search(items, content, exprs, index, limit, max);

    return items;
}

void Qentem::Engine::_search(Array<Match> &items, const String &content, const Expressions &exprs, UNumber index,
                             UNumber limit, UNumber max, UNumber level) noexcept {
    bool    LOCKED    = false; // To keep matching the end of the current expression.
    bool    SPLIT     = false; // To keep tracking a split match.
    bool    OVERDRIVE = false; // To achieving nesting.
    UNumber counter   = 0;     // Index for counting.

    Match _item;

    const UNumber started = index;
    UNumber       id      = 0; // Expression's id.
    // Seting the value of the current expression.
    Expression *ce = exprs[id++];

    UNumber end_offset  = 0; // Temp offset.
    UNumber nest_offset = 0; // Temp variable for nested matches.

    while (true) {
        if (ce->SearchCB == nullptr) {
            if (content.Str[index] == ce->Keyword.Str[counter]) {
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
        } else {
            counter = ce->SearchCB(content, *ce, &_item, index, end_offset, limit);
        }

        if (counter != 0) {
            counter = 0;
            end_offset += 1;

            if (OVERDRIVE) {
                // If the match is on "OVERDRIVE", then break.
                OVERDRIVE = false;
                // Set the length of the nested match.
                limit = (end_offset - 1);
            } else if (!LOCKED) {
                if (ce->Connected != nullptr) {
                    if ((Flags::TRIM & ce->Connected->Flag) != 0) {
                        while ((content.Str[end_offset] == L' ') || (content.Str[end_offset] == L'\n')) {
                            end_offset++;
                        }
                    }
                    _item.OLength = (end_offset - index);
                }

                _item.Offset = index;

                nest_offset = end_offset;
                index       = nest_offset - 1;
            }

            if (ce->Connected == nullptr) {
                // If it's a nesting expression, search again but inside the current match.
                if ((ce->NestExprs.Size != 0) && (nest_offset != index)) {
                    // Start a new search inside the current match.
                    UNumber _size = _item.NestMatch.Size;
                    Engine::_search(_item.NestMatch, content, ce->NestExprs, nest_offset, index,
                                    ((max != 0) ? max : limit), (level + 1));

                    if (_item.NestMatch.Size != _size) {
                        if (max > limit) {
                            // This is important to have the search look ahead of the limited length
                            // in order to find the entire currect match.
                            limit     = max; // TO THE MAX!
                            OVERDRIVE = true;
                        }

                        Match *p_item = &(_item.NestMatch.Last(false));
                        // Seek to avoid having the same closing/ending keywork matched again.
                        nest_offset = (p_item->Offset + p_item->Length);
                        index       = nest_offset;

                        continue; // Not done matching. Move to the next char.
                    }
                }

                if ((Flags::IGNORE & ce->Flag) == 0) {
                    _item.Length = (end_offset - _item.Offset);

                    if (_item.OLength != 0) {
                        if (((Flags::TRIM & ce->Flag) != 0) && ((index - _item.Offset) != _item.OLength)) {
                            while ((content.Str[--index] == L' ') || (content.Str[index] == L'\n')) {
                            }
                            index++;
                        }
                        _item.CLength = (end_offset - index);
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
                        if ((level == 0) &&
                            (((Flags::SPLITNEST & ce->Flag) != 0) || ((Flags::SPLITROOTONLY & ce->Flag) != 0))) {
                            // This could run on its own thread.
                            Engine::Split(content, _item.NestMatch, (_item.Offset + _item.OLength),
                                          ((_item.Offset + _item.Length) - (_item.CLength)));
                        }

                        items.Last(true).Move(_item);
                        items.Size++;

                        if ((Flags::ONCE & ce->Flag) != 0) {
                            return;
                        }
                    }
                }

                // Prepare for the next match.
                if (_item.NestMatch.Size != 0) {
                    _item.NestMatch.Clear();
                }

                if (_item.SubMatch.Size != 0) {
                    _item.SubMatch.Clear();
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
            ce = exprs[id++];
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

                Match *p_item = &(_item.NestMatch)[(_item.NestMatch.Size - 1)];
                // Seek the offset to where the last match ended.
                index = p_item->Offset + p_item->Length;

                if (index == limit) {
                    break;
                }

                _item.NestMatch.Clear();
            }

            // House cleaning...
            OVERDRIVE = false;
            LOCKED    = false;
            if (id == exprs.Size) {
                return;
            }

            ce = exprs[id];
        }
    }

    if (items.Size == 0) {
        if ((Flags::POP & ce->Flag) != 0) { //&& (ce->NestExprs.Size != 0)
            Engine::_search(items, content, ce->NestExprs, started, limit, 0, 0);
            return;
        }
    } else if ((level == 0) && SPLIT) {
        Engine::Split(content, items, started, limit);
    }

    // AlJumaa, Jamada El Oula 12, 1440
    // Friday, January 18, 2019
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
    Match        _item;
    UNumber      offset = index;
    UNumber      ends;
    Expression * master = nullptr;
    Match *      tmp    = nullptr;
    Match *      tmp2   = nullptr;

    for (UNumber i = 0; i < items.Size; i++) {
        tmp        = &(items[i]);
        _item.Expr = tmp->Expr;

        if ((Flags::SPLITNEST & tmp->Expr->Flag) != 0) {
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
                tmp2 = &splitted.Last(true);
                splitted.Size++;
                tmp2->Move(_item);

                if (tmp->NestMatch.Size != 0) {
                    tmp2->NestMatch.Move(tmp->NestMatch);
                } else if (nesties.Size != 0) {
                    tmp2->NestMatch.Move(nesties);
                } else if (tmp2->Expr->NestExprs.Size != 0) {
                    Engine::_search(tmp2->NestMatch, content, _item.Expr->NestExprs, _item.Offset,
                                    (_item.Offset + _item.Length), 0, 0);
                }

                if (tmp->SubMatch.Size != 0) {
                    _item.SubMatch.Move(tmp->SubMatch);
                }
            }
            offset = (tmp->Offset + tmp->Length);
        } else {
            nesties.Last(true).Move(*tmp);
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
        tmp2 = &splitted.Last(true);
        splitted.Size++;
        tmp2->Move(_item);

        if (_item.Length != 0) {
            if (nesties.Size != 0) {
                tmp2->NestMatch.Move(nesties);
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
        items.Move(splitted);
    } else {
        items.SetCapacity(1);
        items.Size = 1;

        items[0].Offset = index;
        items[0].Length = (to - index);
        items[0].Expr   = master;
        items[0].ID     = _item.ID;
        items[0].Tag    = _item.Tag;
        items[0].NestMatch.Move(splitted);
    }
}

String Qentem::Engine::Parse(const String &content, const Array<Match> &items, UNumber index, UNumber limit) noexcept {
    if (items.Size == 0) {
        return content;
    }

    if (limit == 0) {
        limit = (content.Length - index);
    }

    if (index >= limit) {
        return content;
    }

    StringStream rendered; // Final content
    UNumber      offset;
    UNumber      length;
    Match *      _item;

    for (UNumber id = 0; id < items.Size; id++) {
        // Current match
        _item = &(items[id]);

        if ((Flags::NOPARSE & _item->Expr->Flag) != 0) {
            continue;
        }

        if ((Flags::COMPACT & _item->Expr->Flag) == 0) {
            offset = _item->Offset;
            length = _item->Length;
        } else {
            offset = (_item->Offset + _item->OLength);
            length = (_item->Length - (_item->OLength + _item->CLength));
        }

        // Add any content that comes before...
        if ((index < offset)) {
            rendered += String::Part(content, index, (offset - index));
        }

        if (_item->Expr->ParseCB != nullptr) {
            if ((Flags::BUBBLE & _item->Expr->Flag) != 0) {
                if (_item->NestMatch.Size != 0) {
                    rendered += _item->Expr->ParseCB(Engine::Parse(content, _item->NestMatch, offset, (offset + length)),
                                                     *_item);
                } else {
                    rendered += _item->Expr->ParseCB(String::Part(content, offset, length), *_item);
                }
            } else {
                rendered += _item->Expr->ParseCB(content, *_item);
            }
        } else {
            // Defaults to replace: it might be an empty string.
            rendered += _item->Expr->Replace;
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
