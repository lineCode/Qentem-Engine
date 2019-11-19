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
using Qentem::Engine::MatchBit;

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

static void InfinitSpaceCallback(const char *content, UNumber &offset, const UNumber endOffset, MatchBit &item,
                                 Array<MatchBit> &items) noexcept {
    while (content[offset] == ' ') {
        ++offset;
        ++item.Length;
    }

    items += static_cast<MatchBit &&>(item);
}

static const Expressions &getXMLExpres() noexcept {
    static Expressions expres(1);

    if (expres.Size == 0) {
        static Expression tag_end;
        tag_end.SetKeyword(">");

        static Expression tag_start;
        tag_start.SetKeyword("<");
        tag_start.Connected = &tag_end;

        expres += &tag_start;
    }

    return expres;
}

static const Expressions &getPropertiesExpres() noexcept {
    static Expressions expres(3);

    if (expres.Size == 0) {
        static Expression equal;
        equal.SetKeyword("=");
        equal.Flag = Flags::SPLIT | Flags::DROPEMPTY;

        static Expression space;
        space.SetKeyword(" ");
        space.Flag    = Flags::SPLIT | Flags::DROPEMPTY;
        space.MatchCB = &(InfinitSpaceCallback);

        static Expression quot_end;
        quot_end.SetKeyword("\"");
        quot_end.Flag = Flags::IGNORE;

        static Expression quot;
        quot.SetKeyword("\"");
        quot.Connected = &quot_end;

        expres.Add(&equal).Add(&quot).Add(&space);
    }

    return expres;
}

static Array<XTag> parseTags(const String &content, const Array<MatchBit> &items, UNumber id, const UNumber count) noexcept {

    static const Expressions &propertiesExpres = getPropertiesExpres();

    Array<XTag> tags;
    XTag        x_tag;

    UNumber startAt;
    UNumber tagLen;
    UNumber repeated = 0;

    UNumber         matchStart = 0;
    UNumber         tmpIndex   = 0;
    UNumber         remlen;
    Array<MatchBit> x_properties;

    MatchBit *item;
    bool      tagged = false;

    for (UNumber index = 0; index < count;) {
        item = &items[id];

        startAt = (item->Offset + 1);
        tagLen  = 0;

        // Finding the name of the tag
        for (UNumber x = startAt; ((content[x] != ' ') && (content[x] != '>')); ++x) {
            ++tagLen;
        }

        if (tagged) {
            if (content[startAt] != '/') {                          // If it's not a closing tag.
                if (x_tag.Name.Compare(content, startAt, tagLen)) { // If it's equal to the current one.
                    ++repeated;                                     // then it's a chiled tag.
                }
            } else if (x_tag.Name.Compare(content, (startAt + 1), (tagLen - 1))) {
                // A closing tag and it's equal to the current one.
                if (repeated != 0) {
                    // if it has a chiled of the same tag, then the closing tag is
                    // not for the undergoing tag; pass.
                    --repeated;
                } else {
                    tagged = false; // The opening and the ending tag has been found.

                    // Subtags
                    UNumber   subStart  = matchStart + 1;
                    UNumber   subCount  = id - subStart;
                    UNumber   lastTag   = (tags.Size - 1);
                    MatchBit *headItem  = &(items[matchStart]);
                    UNumber   headStart = (headItem->Offset + headItem->Length);

                    tags[lastTag].InnerText = String::Part(content.Str, headStart, (items[id].Offset - headStart));

                    if (subCount != 0) {
                        tags[lastTag].InnerNodes += parseTags(content, items, subStart, subCount);
                    }
                }
            }
        } else if (content[startAt] != '/') { // A new tag that is not a closing one.
            x_tag.Name = String::Part(content.Str, startAt, tagLen);
            // TODO: Add inline HTML tag list

            matchStart = id;
            tmpIndex   = index;

            // Paraperties
            remlen = item->Length - tagLen - 2; // Remaining text.
            tagged = (content[((item->Offset + item->Length) - 2)] != '/');

            if (!tagged) {
                // Inline tag.
                --remlen; // Forward slash
            }

            UNumber   startIndex = (startAt + tagLen);
            XProperty xp;
            MatchBit *xpMatch;

            x_properties = Qentem::Engine::Match(propertiesExpres, content.Str, startIndex, remlen);

            for (UNumber p = 0; p < x_properties.Size;) {
                xpMatch = &x_properties[p];
                xp.Name = String::Part(content.Str, xpMatch->Offset, xpMatch->Length);
                ++p;

                xpMatch = &x_properties[p];
                if (content[xpMatch->Offset] == '"') {
                    xp.Value = String::Part(content.Str, (xpMatch->Offset + 1), (xpMatch->Length - 2));
                } else {
                    xp.Value = String::Part(content.Str, xpMatch->Offset, xpMatch->Length);
                }

                ++p;
                x_tag.Properties.Add(xp);
            }

            tags.Add(x_tag);
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

static Array<XTag> Parse(const String &content) noexcept {
    const Array<MatchBit> items = Qentem::Engine::Match(getXMLExpres(), content.Str, 0, content.Length);

    return parseTags(content, items, 0, items.Size);
}

} // namespace XMLParser
} // namespace Qentem

#endif
