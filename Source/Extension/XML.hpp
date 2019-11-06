/**
 * Qentem XML
 *
 * @brief     For persing XML or HTML string
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Engine.hpp"

#ifndef QENTEM_XML_H
#define QENTEM_XML_H

namespace Qentem {
namespace XMLParser {

using Qentem::Engine::Expression;
using Qentem::Engine::Expressions;
using Qentem::Engine::Flags;
using Qentem::Engine::Match;

struct XProperty {
    String Name;
    String Value;
};

struct XTag {
    String           Name;
    Array<XProperty> Properties;
    String           InnerText;
    Array<XTag>      InnerNodes;
};

static void InfinitSpaceCallback(wchar_t const *content, UNumber &offset, UNumber const endOffset, Match &item,
                                 Array<Match> &items) noexcept {
    while (content[offset] == L' ') {
        ++offset;
        ++item.Length;
    }

    items += static_cast<Match &&>(item);
}

static Expressions const &getXMLExprs() noexcept {
    static Expression xStart;
    static Expression xEnd;

    static Expressions tags;

    if (tags.Size == 0) {
        xStart.SetKeyword(L"<");
        xEnd.SetKeyword(L">");
        xStart.Connected = &xEnd;

        tags.SetCapacity(1);
        tags += &xStart;
    }

    return tags;
}

static Expressions const &getPropertiesExprs() noexcept {
    static Expression equalExpr;
    static Expression spaceExpr;

    static Expression quotExpr;
    static Expression endQuotExpr;

    static Expressions tags;

    if (tags.Size == 0) {
        equalExpr.SetKeyword(L"=");
        equalExpr.Flag = Flags::SPLIT | Flags::DROPEMPTY;

        spaceExpr.SetKeyword(L" ");
        spaceExpr.Flag    = Flags::SPLIT | Flags::DROPEMPTY;
        spaceExpr.MatchCB = &(InfinitSpaceCallback);

        quotExpr.SetKeyword(L"\"");
        endQuotExpr.SetKeyword(L"\"");
        quotExpr.Connected = &endQuotExpr;
        endQuotExpr.Flag   = Flags::IGNORE;

        tags.SetCapacity(3);
        tags.Add(&equalExpr).Add(&quotExpr).Add(&spaceExpr);
    }

    return tags;
}

static Array<XTag> parseTags(String const &content, Array<Match> const &items, UNumber id, UNumber const count) noexcept {

    static Expressions const &_propertiesExprs = getPropertiesExprs();

    Array<XTag> tags;
    XTag        _tag;

    UNumber startAt;
    UNumber tagLen;
    UNumber repeated = 0;

    UNumber      matchStart = 0;
    UNumber      tmpIndex   = 0;
    UNumber      remlen;
    Array<Match> _properties;

    Match *item;
    bool   tagged = false;

    for (UNumber index = 0; index < count;) {
        item = &items[id];

        startAt = (item->Offset + 1);
        tagLen  = 0;

        // Finding the name of the tag
        for (UNumber x = startAt; ((content[x] != L' ') && (content[x] != L'>')); ++x) {
            ++tagLen;
        }

        if (tagged) {
            if (content[startAt] != L'/') {                        // If it's not a closing tag.
                if (_tag.Name.Compare(content, startAt, tagLen)) { // If it's equal to the current one.
                    ++repeated;                                    // then it's a chiled tag.
                }
            } else if (_tag.Name.Compare(content, (startAt + 1), (tagLen - 1))) {
                // A closing tag and it's equal to the current one.
                if (repeated != 0) {
                    // if it has a chiled of the same tag, then the closing tag is
                    // not for the undergoing tag; pass.
                    --repeated;
                } else {
                    tagged = false; // The opening and the ending tag has been found.

                    // Subtags
                    UNumber subStart  = matchStart + 1;
                    UNumber subCount  = id - subStart;
                    UNumber lastTag   = (tags.Size - 1);
                    Match * headItem  = &(items[matchStart]);
                    UNumber headStart = (headItem->Offset + headItem->Length);

                    tags[lastTag].InnerText = String::Part(content.Str, headStart, (items[id].Offset - headStart));

                    if (subCount != 0) {
                        tags[lastTag].InnerNodes += parseTags(content, items, subStart, subCount);
                    }
                }
            }
        } else if (content[startAt] != L'/') { // A new tag that is not a closing one.
            _tag.Name = String::Part(content.Str, startAt, tagLen);
            // TODO: Add inline HTML tag list

            matchStart = id;
            tmpIndex   = index;

            // Paraperties
            remlen = item->Length - tagLen - 2; // Remaining text.
            tagged = (content[((item->Offset + item->Length) - 2)] != L'/');

            if (!tagged) {
                // Inline tag.
                --remlen; // Forward slash
            }

            UNumber   startIndex = (startAt + tagLen);
            XProperty xp;
            Match *   xpMatch;

            _properties = Qentem::Engine::Search(content.Str, _propertiesExprs, startIndex, remlen);

            for (UNumber p = 0; p < _properties.Size;) {
                xpMatch = &_properties[p];
                xp.Name = String::Part(content.Str, xpMatch->Offset, xpMatch->Length);
                ++p;

                xpMatch = &_properties[p];
                if (content[xpMatch->Offset] == L'"') {
                    xp.Value = String::Part(content.Str, (xpMatch->Offset + 1), (xpMatch->Length - 2));
                } else {
                    xp.Value = String::Part(content.Str, xpMatch->Offset, xpMatch->Length);
                }

                ++p;
                _tag.Properties.Add(xp);
            }

            tags.Add(_tag);
        }

        ++index;

        if ((index == count) && tagged) {
            // Inline tag, or a tag without an ending tag.
            tagged   = false;
            index    = (tmpIndex + 1);
            id       = matchStart;
            repeated = 0; // Clear subtag count; it's not a parent.
        }

        ++id;
    }

    return tags;
}

static Array<XTag> Parse(String const &content) noexcept {
    static Expressions const &_xmlExprs = getXMLExprs();

    Array<Match> const items = Qentem::Engine::Search(content.Str, _xmlExprs, 0, content.Length);

    return parseTags(content, items, 0, items.Size);
}

} // namespace XMLParser
} // namespace Qentem

#endif
