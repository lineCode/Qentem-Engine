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

static Expressions const &_getTagsAll() noexcept;
static Expressions const &_getTagsVara() noexcept;
static Expressions const &_getTagsQuotes() noexcept;
static Expressions const &_getTagsHead() noexcept;

static String Render(wchar_t const *content, UNumber const offset, UNumber const limit, void *data) noexcept {
    static Expressions const &_tagsAll = _getTagsAll();
    return Engine::Parse(content, Engine::Search(content, _tagsAll, offset, limit), offset, limit, static_cast<void *>(data));
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
    static Expressions const &_tagsQuotes = _getTagsQuotes();

    Array<Match> const items(Engine::Search(block, _tagsQuotes, 0, length));

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
    static Expressions const &_tagsVars = _getTagsVara();

    UNumber const offset = (item.Offset + 1);
    UNumber const limit  = (item.Length - 2);

    String const content(Engine::Parse(block, Engine::Search(block, _tagsVars, offset, limit), offset, limit, other));

    return (ALU::Evaluate(content.Str, 0, content.Length) > 0.0);
}

static String RenderIF(wchar_t const *block, Match const &item, UNumber const length, void *other) noexcept {
    // Nothing is processed inside the match before checking if the condition is TRUE.
    bool _true = false;

    static Expressions const &_tagsHead = _getTagsHead();

    Array<Match> _subMatch(Engine::Search(block, _tagsHead, item.Offset, item.Length));

    if (_subMatch.Size != 0) {
        Match *sm = &(_subMatch[0]);

        if (sm->NestMatch.Size != 0) {
            Match *nm = &(sm->NestMatch[0]);
            _true     = Template::EvaluateIF(block, *nm, other);

            // inner content of if
            UNumber offset = (sm->Offset + sm->Length);
            UNumber limit  = (item.Length - (sm->Length + 5));

            // // if_else (splitted content)
            if (item.NestMatch.Size != 0) {
                nm = &(item.NestMatch[0]);
                if (_true) {
                    limit = (nm->Length - (offset - nm->Offset));
                } else {
                    for (UNumber i = 1; i < item.NestMatch.Size; i++) {
                        offset    = (nm->Offset + nm->Length);
                        _subMatch = Engine::Search(block, _tagsHead, offset, (length - nm->Length));

                        // inner content of the next part.
                        nm     = &(item.NestMatch[i]);
                        offset = nm->Offset;
                        limit  = nm->Length;

                        if ((_subMatch.Size != 0) &&
                            ((_subMatch[0].NestMatch.Size == 0) || Template::EvaluateIF(block, _subMatch[0].NestMatch[0], other))) {
                            _true = true;
                            break;
                        }
                    }
                }
            }

            if (_true) {
                return Render(block, offset, limit, other);
            }
        }
    }

    return String();
}

static String Repeat(String const &content, String const &name, String const &value_id, String const &key_id, void *other) noexcept {
    Entry *   _entry;
    Document *_storage = (static_cast<Document *>(other))->GetSource(&_entry, name.Str, 0, name.Length);

    if (_storage == nullptr) {
        return String();
    }

    StringStream rendered;
    Expressions  loop_exp;

    Expression value_ex;
    value_ex.Keyword = value_id.Str;
    value_ex.Length  = value_id.Length;

    Expression key_ex;
    key_ex.Keyword = key_id.Str;
    key_ex.Length  = key_id.Length;

    loop_exp.Add(&value_ex);

    if (key_id.Length != 0) {
        loop_exp.Add(&key_ex);
    }

    Array<Match> const items(Engine::Search(content.Str, loop_exp, 0, content.Length));
    if (_entry->Type == VType::DocumentT) {
        String tmpstr;
        String tmpstr2;

        if (_storage->Ordered) {
            if (_storage->Strings.Size != 0) {
                for (UNumber i = 0; i < _storage->Strings.Size; i++) {
                    if (key_id.Length != 0) {
                        tmpstr                   = String::FromNumber(i);
                        loop_exp[1]->ReplaceWith = tmpstr.Str;
                        loop_exp[1]->RLength     = tmpstr.Length;
                    }

                    loop_exp[0]->ReplaceWith = _storage->Strings[i].Str;
                    loop_exp[0]->RLength     = _storage->Strings[i].Length;

                    rendered += Engine::Parse(content.Str, items, 0, content.Length);
                }
            } else if (_storage->Numbers.Size != 0) {
                for (UNumber i = 0; i < _storage->Numbers.Size; i++) {
                    if (key_id.Length != 0) {
                        tmpstr                   = String::FromNumber(i);
                        loop_exp[1]->ReplaceWith = tmpstr.Str;
                        loop_exp[1]->RLength     = tmpstr.Length;
                    }

                    tmpstr2                  = String::FromNumber(_storage->Numbers[i], 1, 0, 3);
                    loop_exp[0]->ReplaceWith = tmpstr2.Str;
                    loop_exp[0]->RLength     = tmpstr2.Length;

                    rendered += Engine::Parse(content.Str, items, 0, content.Length, other);
                }
            }
        } else {
            for (UNumber i = 0; i < _storage->Keys.Size; i++) {
                if (key_id.Length != 0) {
                    loop_exp[1]->ReplaceWith = _storage->Keys[i].Str;
                    loop_exp[1]->RLength     = _storage->Keys[i].Length;
                }

                _storage->GetString(tmpstr2, _storage->Keys[i].Str, 0, _storage->Keys[i].Length);
                loop_exp[0]->ReplaceWith = tmpstr2.Str;
                loop_exp[0]->RLength     = tmpstr2.Length;

                rendered += Engine::Parse(content.Str, items, 0, content.Length, other);
            }
        }
    }

    return rendered.Eject();
}

// <loop set="abc2" value="s_value" key="s_key">
//     <span>s_key: s_value</span>
// </loop>
static String RenderLoop(wchar_t const *block, Match const &item, UNumber const length, void *other) noexcept {
    // To match: <loop (set="abc2" value="s_value" key="s_key")>
    static Expressions const &_tagsHead = _getTagsHead();

    Array<Match> const _subMatch(Engine::Search(block, _tagsHead, item.Offset, item.Length));

    if ((_subMatch.Size != 0) && (_subMatch[0].NestMatch.Size != 0)) {
        String       name;
        String       value_id;
        String       key_id;
        Match const *sm = &(_subMatch[0]);

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
                        name = String::Part(block, (m->Offset + 1), (m->Length - 2));
                        break;
                    }

                    if (block[start_at] == L'e') { // valu[e]
                        value_id = String::Part(block, (m->Offset + 1), (m->Length - 2));
                        break;
                    }

                    if (block[start_at] == L'y') { // ke[y]
                        key_id = String::Part(block, (m->Offset + 1), (m->Length - 2));
                        break;
                    }
                }
            }
        }

        if ((name.Length != 0) && (value_id.Length != 0)) {
            String _content =
                Repeat(String::Part(block, (sm->Offset + sm->Length), (item.Length - (sm->Length + 7))), name, value_id, key_id, other);
            return Render(_content.Str, 0, _content.Length, other);
        }
    }

    return String();
}

static Expressions const &_getTagsVara() noexcept {
    static Expression  TagVar;
    static Expression  VarTail;
    static Expressions tags;

    if (tags.Size == 0) {
        // Variables evaluation.
        VarTail.ParseCB = &(Template::RenderVar);
        // {v:var_name}
        TagVar.SetKeyword(L"{v:");
        VarTail.SetKeyword(L"}");
        TagVar.Connected = &VarTail;
        VarTail.Flag     = Flags::TRIM;

        tags.SetCapacity(1);
        tags.Add(&TagVar);
    }

    return tags;
}

static Expressions const &_getTagsQuotes() noexcept {
    static Expression  TagQuote;
    static Expression  QuoteTail;
    static Expressions tags;

    if (tags.Size == 0) {
        TagQuote.SetKeyword(L"\"");
        QuoteTail.SetKeyword(L"\"");
        TagQuote.Connected = &QuoteTail;

        tags.SetCapacity(1);
        tags.Add(&TagQuote);
    }

    return tags;
}

static Expressions const &_getTagsHead() noexcept {
    static Expression  TagHead;
    static Expression  TagHead_T;
    static Expressions tags;

    if (tags.Size == 0) {
        TagHead.SetKeyword(L"<");
        TagHead_T.SetKeyword(L">");
        TagHead_T.Flag    = Flags::ONCE;
        TagHead.Connected = &TagHead_T;
        // Nest to prevent matching ">" bigger sign inside if statement.
        TagHead_T.NestExprs = _getTagsQuotes();

        tags.SetCapacity(1);
        tags.Add(&TagHead);
    }

    return tags;
}

static Expressions const &_getTagsAll() noexcept {
    static Expression TagIif;
    static Expression IifTail;

    static Expression TagIf;
    static Expression IfTail;

    static Expression ShallowTagIf;
    static Expression ShallowIfTail;

    static Expression TagELseIf;
    static Expression ELseIfTail;

    static Expression TagLoop;
    static Expression LoopTail;

    static Expression TagMath;
    static Expression MathTail;

    static Expressions tags;

    if (tags.Size == 0) {
        // Inline if evaluation.
        IifTail.ParseCB = &(Template::RenderIIF);
        //{iif case="3 == 3" true="Yes" false="No"}
        TagIif.SetKeyword(L"{iif");
        IifTail.SetKeyword(L"}");
        TagIif.Connected = &IifTail;
        IifTail.Flag     = Flags::BUBBLE;
        IifTail.NestExprs.Add(&TagIif).Add(_getTagsVara()); // Nested by itself and TagVars
        /////////////////////////////////

        // If spliter.
        // <else />
        TagELseIf.SetKeyword(L"<else");
        ELseIfTail.SetKeyword(L"/>");
        ELseIfTail.Flag     = Flags::SPLIT;
        TagELseIf.Connected = &ELseIfTail;
        /////////////////////////////////

        // Shallow IF
        // To not match anything inside inner if until it's needed.
        ShallowTagIf.SetKeyword(L"<if");
        ShallowIfTail.SetKeyword(L"</if>");
        ShallowTagIf.Connected = &ShallowIfTail;
        ShallowIfTail.NestExprs.SetCapacity(1);
        ShallowIfTail.NestExprs.Add(&ShallowTagIf);

        // If evaluation.
        IfTail.ParseCB = &(Template::RenderIF);
        // <if case="{case}">html code</if>
        TagIf.SetKeyword(L"<if");
        IfTail.SetKeyword(L"</if>");
        TagIf.Connected = &IfTail;
        IfTail.NestExprs.Add(&ShallowTagIf).Add(&TagELseIf);
        /////////////////////////////////

        // Loop evaluation.
        LoopTail.ParseCB = &(Template::RenderLoop);
        // <loop set="abc2" var="loopId">
        //     <span>loopId): -{v:abc2[loopId]}</span>
        // </loop>
        TagLoop.SetKeyword(L"<loop");
        LoopTail.SetKeyword(L"</loop>");
        TagLoop.Connected = &LoopTail;
        LoopTail.NestExprs.SetCapacity(1);
        LoopTail.NestExprs.Add(&TagLoop); // Nested by itself
        /////////////////////////////////

        // Math Tag.
        MathTail.ParseCB = &(Template::RenderMath);
        // {math:5+6*8*(8+3)}
        TagMath.SetKeyword(L"{math:");
        MathTail.SetKeyword(L"}");
        TagMath.Connected = &MathTail;
        MathTail.Flag     = Flags::TRIM | Flags::BUBBLE;
        MathTail.NestExprs.SetCapacity(1);
        MathTail.NestExprs.Add(_getTagsVara());
        /////////////////////////////////

        tags.SetCapacity(5);
        tags.Add(_getTagsVara()).Add(&TagMath).Add(&TagIif).Add(&TagIf).Add(&TagLoop);
    }

    return tags;
}

} // namespace Template
} // namespace Qentem

#endif
