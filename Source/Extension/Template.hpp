/**
 * Qentem Template
 *
 * @brief     Generates dynamic HTML code from a template.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Extension/ALU.hpp"
#include "Extension/Document.hpp"

#ifndef QENTEM_TEMPLATE_H
#define QENTEM_TEMPLATE_H

namespace Qentem {
namespace Template {

using Qentem::Engine::Flags;

static Expressions const &getTagsAll() noexcept;
static Expressions const &getTagsVara() noexcept;
static Expressions const &getTagsQuotes() noexcept;
static Expressions const &getTagsHead() noexcept;

static String Render(wchar_t const *content, UNumber const offset, UNumber const limit, void *data) noexcept {
    static Expressions const &tagsAll = getTagsAll();
    return Engine::Parse(content, Engine::Search(content, tagsAll, offset, limit), offset, limit, static_cast<void *>(data));
}

inline static String Render(String const &content, Document *data) noexcept {
    return Render(content.Str, 0, content.Length, data);
}

// e.g. {v:var_name}
// e.g. {v:var_name[id]}
// Nest: {v:var_{v:var2_{v:var3_id}}}
static String RenderVar(wchar_t const *block, Match const &item, UNumber const length, void *other) noexcept {
    String value;

    if (!(static_cast<Document *>(other))->GetString(value, block, (item.Offset + 3), (item.Length - 4))) {
        value = String::Part(block, (item.Offset + 3), (item.Length - 4));
    }

    return value;
}

static String RenderMath(wchar_t const *block, Match const &item, UNumber const length, void *other) noexcept {
    return String::FromNumber(ALU::Evaluate(block, 6, (length - 7)), 1, 0, 3);
}

// {iif case="3 == 3" true="Yes" false="No"}
// {iif case="{v:var_five} == 5" true="5" false="no"}
// {iif case="{v:var_five} == 5" true="{v:var_five} is equal to 5" false="no"}
// {iif case="3 == 3" true="Yes" false="No"}
static String RenderIIF(wchar_t const *block, Match const &item, UNumber const length, void *other) noexcept {
    static Expressions const &tagsQuotes = getTagsQuotes();

    Array<Match> const items(Engine::Search(block, tagsQuotes, 0, length));

    Match * m;
    bool    iif_case  = false;
    Match * iif_true  = nullptr;
    Match * iif_false = nullptr;
    UNumber start_at;

    // case="[statement]" true="[Yes]" false="[No]"
    for (UNumber i = 0; i < items.Size; i++) {
        // With this method, order is not necessary of case=, true=, false=
        m = &(items[i]);
        if (m->Offset > 5) {
            start_at = m->Offset - 3;

            while ((start_at <= m->Offset) && (start_at != 0)) {
                --start_at;

                if (block[start_at] == L'a') { // c[a]se
                    iif_case = (ALU::Evaluate(block, (m->Offset + 1), (m->Length - 2)) > 0.0);
                    break;
                }

                if (block[start_at] == L'r') { // t[r]ue
                    iif_true = m;
                    break;
                }

                if (block[start_at] == L'l') { // fa[l]se
                    iif_false = m;
                    break;
                }
            }
        }
    }

    if (iif_case) {
        if (iif_true != nullptr) {
            return String::Part(block, (iif_true->Offset + 1), (iif_true->Length - 2));
        }
    } else if (iif_false != nullptr) {
        return String::Part(block, (iif_false->Offset + 1), (iif_false->Length - 2));
    }

    return String();
}

// <if case="{case}">html code</if>
// <if case="{case}">html code1 <else /> html code2</if>
// <if case="{case1}">html code1 <elseif case={case2} /> html code2</if>
// <if case="{case}">html code <if case="{case2}" />additional html code</if></if>
static bool EvaluateIF(wchar_t const *block, Match const &item, void *other) noexcept {
    static Expressions const &tagsVars = getTagsVara();

    UNumber const offset = (item.Offset + 1);
    UNumber const limit  = (item.Length - 2);

    String const content(Engine::Parse(block, Engine::Search(block, tagsVars, offset, limit), offset, limit, other));

    return (ALU::Evaluate(content.Str, 0, content.Length) > 0.0);
}

static String RenderIF(wchar_t const *block, Match const &item, UNumber const length, void *other) noexcept {
    // Nothing is processed inside the match before checking if the condition is TRUE.
    bool is_true = false;

    static Expressions const &tagsHead = getTagsHead();

    Array<Match> subMatch(Engine::Search(block, tagsHead, item.Offset, item.Length));

    if (subMatch.Size != 0) {
        Match *sm = &(subMatch[0]);

        if (sm->NestMatch.Size != 0) {
            Match *nm = &(sm->NestMatch[0]);
            is_true   = Template::EvaluateIF(block, *nm, other);

            // inner content of if
            UNumber offset = (sm->Offset + sm->Length);
            UNumber limit  = (item.Length - (sm->Length + 5));

            // // if_else (splitted content)
            if (item.NestMatch.Size != 0) {
                nm = &(item.NestMatch[0]);
                if (is_true) {
                    limit = (nm->Length - (offset - nm->Offset));
                } else {
                    for (UNumber i = 1; i < item.NestMatch.Size; i++) {
                        offset   = (nm->Offset + nm->Length);
                        subMatch = Engine::Search(block, tagsHead, offset, (length - nm->Length));

                        // inner content of the next part.
                        nm     = &(item.NestMatch[i]);
                        offset = nm->Offset;
                        limit  = nm->Length;

                        if ((subMatch.Size != 0) &&
                            ((subMatch[0].NestMatch.Size == 0) || Template::EvaluateIF(block, subMatch[0].NestMatch[0], other))) {
                            is_true = true;
                            break;
                        }
                    }
                }
            }

            if (is_true) {
                return Render(block, offset, limit, other);
            }
        }
    }

    return String();
}

static String Repeat(wchar_t const *block, UNumber offset, UNumber limit, Expression &key_expr, Expression &value_expr,
                     Document const *storage) noexcept {
    Expressions loop_exprs(2);

    if (key_expr.Keyword != nullptr) {
        loop_exprs.Add(&key_expr);
    }

    loop_exprs.Add(&value_expr);

    Array<Match> const items(Engine::Search(block, loop_exprs, offset, limit));

    StringStream rendered;
    String *     str_ptr;
    String       value;
    String       key;

    Entry *entry;
    for (UNumber i = 0; i < storage->Entries.Size; i++) {
        entry = &(storage->Entries[i]);

        if (key_expr.Keyword != nullptr) {
            if (storage->Ordered) {
                key                  = String::FromNumber(i);
                key_expr.ReplaceWith = key.Str;
                key_expr.RLength     = key.Length;
            } else {
                str_ptr              = &(storage->Keys[entry->KeyID]);
                key_expr.ReplaceWith = str_ptr->Str;
                key_expr.RLength     = str_ptr->Length;
            }
        }

        switch (entry->Type) {
            case VType::NumberT: {
                value                  = String::FromNumber(storage->Numbers[entry->ArrayID], 1, 0, 3);
                value_expr.ReplaceWith = value.Str;
                value_expr.RLength     = value.Length;
                break;
            }
            case VType::StringT: {
                str_ptr                = &(storage->Strings[entry->ArrayID]);
                value_expr.ReplaceWith = str_ptr->Str;
                value_expr.RLength     = str_ptr->Length;
                break;
            }
            case VType::FalseT: {
                value_expr.ReplaceWith = L"false";
                value_expr.RLength     = 5;
                break;
            }
            case VType::TrueT: {
                value_expr.ReplaceWith = L"true";
                value_expr.RLength     = 4;
                break;
            }
            case VType::NullT: {
                value_expr.ReplaceWith = L"null";
                value_expr.RLength     = 4;
                break;
            }
            default: {
                continue;
            }
        }

        rendered += Engine::Parse(block, items, offset, limit);
    }

    return rendered.Eject();
}

// <loop set="abc2" value="s_value" key="s_key">
//     <span>s_key: s_value</span>
// </loop>
static String RenderLoop(wchar_t const *block, Match const &item, UNumber const length, void *other) noexcept {
    // To match: <loop (set="abc2" value="s_value" key="s_key")>
    static Expressions const &tagsHead = getTagsHead();

    Array<Match> const subMatch(Engine::Search(block, tagsHead, item.Offset, item.Length));

    if ((subMatch.Size != 0) && (subMatch[0].NestMatch.Size != 0)) {
        Document * storage = nullptr;
        Expression key_expr;
        Expression value_expr;

        Match const *sm = &(subMatch[0]);

        // set="(Array_name)" value="s_value" key="s_key"
        Match * m;
        UNumber start_at;

        for (UNumber i = 0; i < sm->NestMatch.Size; i++) {
            m = &(sm->NestMatch[i]);
            if (m->Offset > 5) { // for the length of: set= or var= + 1
                start_at = m->Offset - 1;

                while ((start_at <= m->Offset) && (start_at > item.Offset)) {
                    --start_at;

                    if (block[start_at] == L't') { // se[t]
                        storage = (static_cast<Document *>(other))->GetDocument(block, (m->Offset + 1), (m->Length - 2));
                        break;
                    }

                    if (block[start_at] == L'e') { // valu[e]
                        value_expr.Keyword = &(block[(m->Offset + 1)]);
                        value_expr.Length  = (m->Length - 2);
                        break;
                    }

                    if (block[start_at] == L'y') { // ke[y]
                        key_expr.Keyword = &(block[(m->Offset + 1)]);
                        key_expr.Length  = (m->Length - 2);
                        break;
                    }
                }
            }
        }

        if ((value_expr.Keyword != nullptr) && (storage != nullptr)) {
            String n_content(Repeat(block, (sm->Offset + sm->Length), (item.Length - (sm->Length + 7)), key_expr, value_expr, storage));
            return Render(n_content.Str, 0, n_content.Length, other);
        }
    }

    return String();
}

static Expressions const &getTagsVara() noexcept {
    static Expression  TagVar;
    static Expression  VarEnd;
    static Expressions tags(1);

    if (tags.Size == 0) {
        // Variables evaluation.
        VarEnd.ParseCB = &(Template::RenderVar);
        // {v:var_name}
        TagVar.SetKeyword(L"{v:");
        VarEnd.SetKeyword(L"}");
        TagVar.Connected = &VarEnd;
        VarEnd.Flag      = Flags::TRIM;

        tags.Add(&TagVar);
    }

    return tags;
}

static Expressions const &getTagsQuotes() noexcept {
    static Expression  TagQuote;
    static Expression  QuoteEnd;
    static Expressions tags(1);

    if (tags.Size == 0) {
        TagQuote.SetKeyword(L"\"");
        QuoteEnd.SetKeyword(L"\"");
        TagQuote.Connected = &QuoteEnd;

        tags.Add(&TagQuote);
    }

    return tags;
}

static Expressions const &getTagsHead() noexcept {
    static Expression  TagHead;
    static Expression  TagHead_T;
    static Expressions tags(1);

    if (tags.Size == 0) {
        TagHead.SetKeyword(L"<");
        TagHead_T.SetKeyword(L">");
        TagHead_T.Flag    = Flags::ONCE;
        TagHead.Connected = &TagHead_T;
        // Nest to prevent matching ">" bigger sign inside if statement.
        TagHead_T.NestExprs = getTagsQuotes();

        tags.Add(&TagHead);
    }

    return tags;
}

static Expressions const &getTagsAll() noexcept {
    static Expression TagIif;
    static Expression IifEnd;

    static Expression TagIf;
    static Expression IfEnd;

    static Expression ShallowTagIf;
    static Expression ShallowIfEnd;

    static Expression TagELseIf;
    static Expression ELseIfEnd;

    static Expression TagLoop;
    static Expression LoopEnd;

    static Expression TagMath;
    static Expression MathEnd;

    static Expressions tags(5);

    if (tags.Size == 0) {
        // Inline if evaluation.
        IifEnd.ParseCB = &(Template::RenderIIF);
        //{iif case="3 == 3" true="Yes" false="No"}
        TagIif.SetKeyword(L"{iif");
        IifEnd.SetKeyword(L"}");
        TagIif.Connected = &IifEnd;
        IifEnd.Flag      = Flags::BUBBLE;
        IifEnd.NestExprs.Add(&TagIif).Add(getTagsVara()); // Nested by itself and TagVars
        /////////////////////////////////

        // If spliter.
        // <else />
        TagELseIf.SetKeyword(L"<else");
        ELseIfEnd.SetKeyword(L"/>");
        ELseIfEnd.Flag      = Flags::SPLIT;
        TagELseIf.Connected = &ELseIfEnd;
        /////////////////////////////////

        // Shallow IF
        // To not match anything inside inner if until it's needed.
        ShallowTagIf.SetKeyword(L"<if");
        ShallowIfEnd.SetKeyword(L"</if>");
        ShallowTagIf.Connected = &ShallowIfEnd;
        ShallowIfEnd.NestExprs.SetCapacity(1);
        ShallowIfEnd.NestExprs.Add(&ShallowTagIf);

        // If evaluation.
        IfEnd.ParseCB = &(Template::RenderIF);
        // <if case="{case}">html code</if>
        TagIf.SetKeyword(L"<if");
        IfEnd.SetKeyword(L"</if>");
        TagIf.Connected = &IfEnd;
        IfEnd.NestExprs.Add(&ShallowTagIf).Add(&TagELseIf);
        /////////////////////////////////

        // Loop evaluation.
        LoopEnd.ParseCB = &(Template::RenderLoop);
        // <loop set="abc2" var="loopId">
        //     <span>loopId): -{v:abc2[loopId]}</span>
        // </loop>
        TagLoop.SetKeyword(L"<loop");
        LoopEnd.SetKeyword(L"</loop>");
        TagLoop.Connected = &LoopEnd;
        LoopEnd.NestExprs.SetCapacity(1);
        LoopEnd.NestExprs.Add(&TagLoop); // Nested by itself
        /////////////////////////////////

        // Math Tag.
        MathEnd.ParseCB = &(Template::RenderMath);
        // {math:5+6*8*(8+3)}
        TagMath.SetKeyword(L"{math:");
        MathEnd.SetKeyword(L"}");
        TagMath.Connected = &MathEnd;
        MathEnd.Flag      = Flags::TRIM | Flags::BUBBLE;
        MathEnd.NestExprs.SetCapacity(1);
        MathEnd.NestExprs.Add(getTagsVara());
        /////////////////////////////////

        tags.Add(getTagsVara()).Add(&TagMath).Add(&TagIif).Add(&TagIf).Add(&TagLoop);
    }

    return tags;
}

} // namespace Template
} // namespace Qentem

#endif
