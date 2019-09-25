
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

struct XMLParser {
    static bool InfinitSpaceCallback(String const &content, UNumber &endAt, Match &item, Array<Match> &items,
                                     Expression **expr, UNumber const level) noexcept {

        while (content[++endAt] == L' ') {
            ++item.Length;
        }
        --endAt;

        items += static_cast<Match &&>(item);

        return true;
    }

    static Expressions const &getXMLExprs() noexcept {
        static Expression xStart;
        static Expression xEnd;

        static Expressions tags;

        if (tags.Index == 0) {
            xStart.Keyword   = L"<";
            xEnd.Keyword     = L">";
            xStart.Connected = &xEnd;

            tags = Expressions().Add(&xStart);
        }

        return tags;
    }

    static Expressions const &getPropertiesExprs() noexcept {
        static Expression equalExpr;
        static Expression spaceExpr;

        static Expression quotExpr;
        static Expression endQuotExpr;

        static Expressions tags;

        if (tags.Index == 0) {
            equalExpr.Keyword = L'=';
            equalExpr.Flag    = Flags::SPLIT | Flags::DROPEMPTY;

            spaceExpr.Keyword = L' ';
            spaceExpr.Flag    = Flags::SPLIT | Flags::DROPEMPTY;
            spaceExpr.MatchCB = &(InfinitSpaceCallback);

            quotExpr.Keyword    = L'"';
            endQuotExpr.Keyword = L'"';
            quotExpr.Connected  = &endQuotExpr;
            endQuotExpr.Flag    = Flags::IGNORE;

            tags = Expressions().Add(&equalExpr).Add(&quotExpr).Add(&spaceExpr);
        }

        return tags;
    }

    static Array<XTag> parseTags(String const &content, Array<Match> const &items, UNumber id,
                                 UNumber const count) noexcept {

        static Expressions const &_propertiesExprs = getPropertiesExprs();

        Array<XTag> tags;
        XTag        _tag;

        UNumber startAt;
        UNumber tagLen;
        UNumber repeated = 0;

        UNumber      matchStart;
        UNumber      tmpIndex;
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
                    if (repeated > 0) {
                        // if it has a chiled of the same tag, then the closing tag is
                        // not for the undergoing tag; pass.
                        --repeated;
                    } else {
                        tagged = false; // The opening and the ending tag has been found.

                        // Subtags
                        UNumber subStart  = matchStart + 1;
                        UNumber subCount  = id - subStart;
                        UNumber lastTag   = (tags.Index - 1);
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

                _properties = Qentem::Engine::Search(content, _propertiesExprs, startIndex, (startIndex + remlen));

                for (UNumber p = 0; p < _properties.Index;) {
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

        Array<Match> items = Qentem::Engine::Search(content, _xmlExprs);
        return parseTags(content, items, 0, items.Index);
    }
};

} // namespace Qentem

#endif
