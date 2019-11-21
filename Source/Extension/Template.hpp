/**
 * Qentem Template
 *
 * @brief     Generates dynamic HTML code from a template.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Extension/ALE.hpp"
#include "Extension/Document.hpp"

#ifndef QENTEM_TEMPLATE_H
#define QENTEM_TEMPLATE_H

namespace Qentem {
namespace Template {

using Qentem::Engine::Flags;

static const Expressions &getExpres() noexcept;
static const Expressions &getVarExpres() noexcept;
static const Expressions &getQuotesExpres() noexcept;
static const Expressions &getHeadExpres() noexcept;

static String Render(const char *content, const UNumber offset, const UNumber limit, void *data) noexcept {
    return Engine::Parse(Engine::Match(getExpres(), content, offset, limit), content, offset, limit, data);
}

inline static String Render(const String &content, Document *data) noexcept {
    return Render(content.Str, 0, content.Length, data);
}

// e.g. {v:var_name}
// e.g. {v:var_name[id]}
// Nest: {v:var_{v:var2_{v:var3_id}}}
static String RenderVar(const char *block, const MatchBit &item, const UNumber length, void *other) noexcept {
    String value;

    if (!(static_cast<Document *>(other))->GetString(value, block, (item.Offset + 3), (item.Length - 4))) {
        value = String::Part(block, (item.Offset + 3), (item.Length - 4));
    }

    return value;
}

static String RenderMath(const char *block, const MatchBit &item, const UNumber length, void *other) noexcept {
    return String::FromNumber(ALE::Evaluate(block, 6, (length - 7)), 1, 0, 3);
}

// {iif case="3 == 3" true="Yes" false="No"}
// {iif case="{v:var_five} == 5" true="5" false="no"}
// {iif case="{v:var_five} == 5" true="{v:var_five} is equal to 5" false="no"}
// {iif case="3 == 3" true="Yes" false="No"}
static String RenderIIF(const char *block, const MatchBit &item, const UNumber length, void *other) noexcept {
    const Array<MatchBit> items(Engine::Match(getQuotesExpres(), block, 0, length));

    bool            iif_case = false;
    const MatchBit *m;
    const MatchBit *iif_true  = nullptr;
    const MatchBit *iif_false = nullptr;
    UNumber         start_at;

    // case="[statement]" true="[Yes]" false="[No]"
    for (UNumber i = 0; i < items.Size; i++) {
        // With this method, order is not necessary of case=, true=, false=
        m = &(items[i]);
        if (m->Offset > 5) {
            start_at = m->Offset - 3;

            while ((start_at <= m->Offset) && (start_at != 0)) {
                --start_at;

                if (block[start_at] == 'a') { // c[a]se
                    iif_case = (ALE::Evaluate(block, (m->Offset + 1), (m->Length - 2)) > 0.0);
                    break;
                }

                if (block[start_at] == 'r') { // t[r]ue
                    iif_true = m;
                    break;
                }

                if (block[start_at] == 'l') { // fa[l]se
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

static bool EvaluateIF(const char *block, const MatchBit &item, void *other) noexcept {
    const UNumber offset = (item.Offset + 1);
    const UNumber limit  = (item.Length - 2);

    const String content(Engine::Parse(Engine::Match(getVarExpres(), block, offset, limit), block, offset, limit, other));

    return (ALE::Evaluate(content.Str, 0, content.Length) > 0.0);
}

// <if case="{case}">html code</if>
// <if case="{case}">html code1 <else /> html code2</if>
// <if case="{case1}">html code1 <elseif case={case2} /> html code2</if>
// <if case="{case}">html code <if case="{case2}" />additional html code</if></if>
static String RenderIF(const char *block, const MatchBit &item, const UNumber length, void *other) noexcept {
    // Nothing is processed inside the match before checking if the condition is TRUE.
    bool is_true = false;

    Array<MatchBit> subMatch(Engine::Match(getHeadExpres(), block, item.Offset, item.Length));

    if (subMatch.Size != 0) {
        MatchBit *sm = &(subMatch[0]);

        if (sm->NestMatch.Size != 0) {
            const MatchBit *nm = &(sm->NestMatch[0]);
            is_true            = Template::EvaluateIF(block, *nm, other);

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
                        subMatch = Engine::Match(getHeadExpres(), block, offset, (length - nm->Length));

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

static String Repeat(const char *block, const UNumber offset, const UNumber limit, Expression &key_expr, Expression &value_expr,
                     const MatchBit *set_, void *other) noexcept {
    Expressions loop_expres(2);

    if (key_expr.Head != nullptr) {
        loop_expres.Add(&key_expr);
    }

    loop_expres.Add(&value_expr);

    const Array<MatchBit> items(Engine::Match(loop_expres, block, offset, limit));

    StringStream  rendered;
    const String *str_ptr;
    String        value;
    String        key;

    const Entry *   entry;
    const Document *storage = (static_cast<Document *>(other))->GetDocument(block, (set_->Offset + 1), (set_->Length - 2));

    if (storage != nullptr) {
        for (UNumber i = 0; i < storage->Entries.Size; i++) {
            entry = &(storage->Entries[i]);

            if (key_expr.Head != nullptr) {
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

            if (value_expr.Head != nullptr) {
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
                        value_expr.ReplaceWith = "false";
                        value_expr.RLength     = 5;
                        break;
                    }
                    case VType::TrueT: {
                        value_expr.ReplaceWith = "true";
                        value_expr.RLength     = 4;
                        break;
                    }
                    case VType::NullT: {
                        value_expr.ReplaceWith = "null";
                        value_expr.RLength     = 4;
                        break;
                    }
                    default: {
                        break;
                    }
                }
            }

            rendered += Engine::Parse(items, block, offset, limit);
        }
    }

    return rendered.ToString();
}

// <loop set="abc2" value="s_value" key="s_key">
//     <span>s_key: s_value</span>
// </loop>
static String RenderLoop(const char *block, const MatchBit &item, const UNumber length, void *other) noexcept {
    // To match: <loop (set="abc2" value="s_value" key="s_key")>
    const Array<MatchBit> subMatch(Engine::Match(getHeadExpres(), block, item.Offset, item.Length));

    if ((subMatch.Size != 0) && (subMatch[0].NestMatch.Size != 0)) {
        const MatchBit *set_ = nullptr;
        Expression      key_expr;
        Expression      value_expr;

        const MatchBit *sm = &(subMatch[0]);

        // set="(Array_name)" value="s_value" key="s_key"
        const MatchBit *m;
        UNumber         start_at;

        for (UNumber i = 0; i < sm->NestMatch.Size; i++) {
            m = &(sm->NestMatch[i]);
            if (m->Offset > 5) { // for the length of: set= or var= + 1
                start_at = m->Offset - 1;

                while ((start_at <= m->Offset) && (start_at > item.Offset)) {
                    --start_at;

                    if (block[start_at] == 't') { // se[t]
                        set_ = m;
                        break;
                    }

                    if (block[start_at] == 'e') { // valu[e]
                        value_expr.Head    = &(block[(m->Offset + 1)]);
                        value_expr.HLength = (m->Length - 2);
                        break;
                    }

                    if (block[start_at] == 'y') { // ke[y]
                        key_expr.Head    = &(block[(m->Offset + 1)]);
                        key_expr.HLength = (m->Length - 2);
                        break;
                    }
                }
            }
        }

        if (((value_expr.Head != nullptr) || (key_expr.Head != nullptr)) && (set_ != nullptr)) {
            String n_content(Repeat(block, (sm->Offset + sm->Length), (item.Length - (sm->Length + 7)), key_expr, value_expr, set_, other));
            return Render(n_content.Str, 0, n_content.Length, other);
        }
    }

    return String();
}

static const Expressions &getVarExpres() noexcept {
    static Expressions expres(1);

    if (expres.Size == 0) {
        // {v:var_name}
        static Expression var_;
        var_.SetHead("{v:");
        var_.SetTail("}");
        var_.Flag    = Flags::TRIM;
        var_.ParseCB = &(Template::RenderVar);

        expres.Add(&var_);
    }

    return expres;
}

static const Expressions &getQuotesExpres() noexcept {
    static Expressions expres(1);

    if (expres.Size == 0) {
        static Expression quote;
        quote.SetHead("\"");
        quote.SetTail("\"");

        expres.Add(&quote);
    }

    return expres;
}

static const Expressions &getHeadExpres() noexcept {
    static Expressions expres(1);

    if (expres.Size == 0) {
        static Expression tag_head;
        tag_head.SetHead("<");
        tag_head.SetTail(">");
        tag_head.Flag = Flags::ONCE;
        // Nest to prevent matching ">" bigger sign inside if statement.
        tag_head.NestExpres = getQuotesExpres();
        expres.Add(&tag_head);
    }

    return expres;
}

static const Expressions &getExpres() noexcept {
    static Expressions expres(5);

    if (expres.Size == 0) {
        //{iif case="3 == 3" true="Yes" false="No"}
        static Expression tag_iif;
        tag_iif.SetHead("{iif");
        tag_iif.SetTail("}");
        tag_iif.Flag    = Flags::BUBBLE;
        tag_iif.ParseCB = &(Template::RenderIIF);
        tag_iif.NestExpres.SetCapacity(1);
        tag_iif.NestExpres.Add(getVarExpres());
        /////////////////////////////////

        // <else />
        static Expression tag_else_if;
        tag_else_if.SetHead("<else");
        tag_else_if.SetTail("/>");
        tag_else_if.Flag = Flags::SPLIT;
        /////////////////////////////////

        // Shallow IF
        // To not match anything inside inner if until it's needed.
        static Expression tag_empty_if;
        tag_empty_if.SetHead("<if");
        tag_empty_if.SetTail("</if>");
        tag_empty_if.Flag = Flags::IGNORE;
        tag_empty_if.NestExpres.SetCapacity(1);
        tag_empty_if.NestExpres.Add(&tag_empty_if);
        /////////////////////////////////

        // <if case="{case}">html code</if>
        static Expression tag_if;
        // tag_if.SetHead("<if");
        tag_if.Head    = tag_empty_if.Head;
        tag_if.HLength = tag_empty_if.HLength;
        // tag_if.SetTail("</if>");
        tag_if.Tail    = tag_empty_if.Tail;
        tag_if.TLength = tag_empty_if.TLength;
        tag_if.ParseCB = &(Template::RenderIF);
        tag_if.NestExpres.Add(&tag_empty_if).Add(&tag_else_if);
        /////////////////////////////////

        // <loop set="abc2" var="loopId">
        //     <span>loopId): -{v:abc2[loopId]}</span>
        // </loop>
        static Expression tag_loop;
        tag_loop.SetHead("<loop");
        tag_loop.SetTail("</loop>");
        tag_loop.ParseCB = &(Template::RenderLoop);
        tag_loop.NestExpres.SetCapacity(1);
        tag_loop.NestExpres.Add(&tag_loop); // Nested by itself
        /////////////////////////////////

        // Math Tag.
        // {math:5+6*8*(8+3)}
        static Expression tag_math;
        tag_math.SetHead("{math:");
        tag_math.SetTail("}");
        tag_math.Flag    = Flags::TRIM | Flags::BUBBLE;
        tag_math.ParseCB = &(Template::RenderMath);
        tag_math.NestExpres.SetCapacity(1);
        tag_math.NestExpres.Add(getVarExpres());
        /////////////////////////////////

        expres.Add(getVarExpres()).Add(&tag_math).Add(&tag_iif).Add(&tag_if).Add(&tag_loop);
    }

    return expres;
}

} // namespace Template
} // namespace Qentem

#endif
