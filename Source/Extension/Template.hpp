
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

struct Template {
    static String Render(String const &content, Document *data) noexcept {
        static Expressions const &_tagsAll = _getTagsAll();
        return Engine::Parse(content, Engine::Search(content, _tagsAll), 0, 0, static_cast<void *>(data));
    }

    // e.g. {v:var_name}
    // e.g. {v:var_name[id]}
    // Nest: {v:var_{v:var2_{v:var3_id}}}
    static String RenderVar(String const &block, Match const &item, void *other) noexcept {
        String value;
        if ((static_cast<Document *>(other))
                ->GetString(value, block, (item.Offset + item.OLength), (item.Length - (item.CLength + item.OLength)))) {
            return value;
        }

        return String::Part(block.Str, (item.Offset + item.OLength), (item.Length - (item.CLength + item.OLength)));
    }

    static String RenderMath(String const &block, Match const &item, void *other) noexcept {
        String value = String::Part(block.Str, item.OLength, (block.Length - (item.OLength + item.CLength)));
        return String::FromNumber(ALU::Evaluate(value), 1, 0, 3);
    }

    // {iif case="3 == 3" true="Yes" false="No"}
    // {iif case="{v:var_five} == 5" true="5" false="no"}
    // {iif case="{v:var_five} == 5" true="{v:var_five} is equal to 5" false="no"}
    // {iif case="3 == 3" true="Yes" false="No"}
    static String RenderIIF(String const &block, Match const &item, void *other) noexcept {
        static Expressions const _tagsQuotes = _getTagsQuotes();

        Array<Match> const &&items = Engine::Search(block, _tagsQuotes);
        if (items.Index == 0) {
            return L"";
        }

        Match * m;
        String  iif_case;
        String  iif_false;
        String  iif_true;
        UNumber start_at;

        // case="[statement]" true="[Yes]" false="[No]"
        for (UNumber i = 0; i < items.Index; i++) {
            // With this method, order is not necessary of case=, true=, false=
            m = &(items[i]);
            if (m->Offset > 5) { // for the length of: set= or var=
                start_at = m->Offset;

                while ((start_at <= m->Offset) && (start_at > 0)) {
                    --start_at;

                    if (block[start_at] == L'a') { // c[a]se
                        iif_case =
                            String::Part(block.Str, (m->Offset + m->OLength), (m->Length - (m->CLength + m->OLength)));
                        break;
                    }

                    if (block[start_at] == L'r') { // t[r]ue
                        iif_true =
                            String::Part(block.Str, (m->Offset + m->OLength), (m->Length - (m->CLength + m->OLength)));
                        break;
                    }

                    if (block[start_at] == L'l') { // fa[l]se
                        iif_false =
                            String::Part(block.Str, (m->Offset + m->OLength), (m->Length - (m->CLength + m->OLength)));
                        break;
                    }
                }
            }
        }

        if ((iif_case.Length != 0) && (ALU::Evaluate(iif_case) != 0.0)) {
            return (iif_true.Length != 0) ? iif_true : L"";
        }

        return (iif_false.Length != 0) ? iif_false : L"";
    }

    // <if case="{case}">html code</if>
    // <if case="{case}">html code1 <else /> html code2</if>
    // <if case="{case1}">html code1 <elseif case={case2} /> html code2</if>
    // <if case="{case}">html code <if case="{case2}" />additional html code</if></if>
    static bool EvaluateIF(String const &block, Match const &item, void *other) noexcept {
        static Expressions const _tagsVars = _getTagsVara();

        UNumber const offset = (item.Offset + item.OLength);
        UNumber const length = (item.Length - (item.OLength + item.CLength));

        String statement = Engine::Parse(block, Engine::Search(block, _tagsVars, offset, (offset + length)), offset,
                                         (offset + length), other);

        return (ALU::Evaluate(statement) != 0.0);
    }

    static String RenderIF(String const &block, Match const &item, void *other) noexcept {
        // Nothing is processed inside the match before checking if the condition is TRUE.
        bool _true = false;

        static Expressions const &_tagsAll  = _getTagsAll();
        static Expressions const &_tagsHead = _getTagsHead();

        Array<Match> const &&_subMatch = Engine::Search(block, _tagsHead, item.Offset, (item.Offset + item.Length));

        if (_subMatch.Index != 0) {
            Match *sm = &(_subMatch[0]);

            if (sm->NestMatch.Index != 0) {
                Match *nm = &(sm->NestMatch[0]);
                _true     = Template::EvaluateIF(block, *nm, other);

                // inner content of if
                UNumber offset = (sm->Offset + sm->Length);
                UNumber length = (item.Length - (sm->Length + item.CLength));

                // // if_else (splitted content)
                if (item.NestMatch.Index != 0) {
                    nm = &(item.NestMatch[0]);
                    if ((Flags::SPLIT & nm->Expr->Flag) != 0) {
                        if (_true) {
                            length = (nm->Length - (offset - nm->Offset));
                        } else {
                            Array<Match> _subMatch2;

                            for (UNumber i = 1; i < item.NestMatch.Index; i++) {
                                _subMatch2 = Engine::Search(block, _tagsHead, offset, item.NestMatch[i].Offset);

                                // inner content of the next part.
                                offset = item.NestMatch[i].Offset;
                                length = item.NestMatch[i].Length;

                                if ((_subMatch2[0].NestMatch.Index == 0) ||
                                    Template::EvaluateIF(block, _subMatch2[0].NestMatch[0], other)) {
                                    _true = true;
                                    break;
                                }
                            }
                        }
                    }
                }

                if (_true) {
                    return Engine::Parse(block, Engine::Search(block, _tagsAll, offset, (offset + length)), offset,
                                         (offset + length), other);
                }
            }
        }

        return L"";
    }

    // <loop set="abc2" value="s_value" key="s_key">
    //     <span>s_key: s_value</span>
    // </loop>
    static String RenderLoop(String const &block, Match const &item, void *other) noexcept {
        // To match: <loop (set="abc2" value="s_value" key="s_key")>
        static Expressions const &_tagsAll  = _getTagsAll();
        static Expressions const &_tagsHead = _getTagsHead();

        Array<Match> const &&_subMatch = Engine::Search(block, _tagsHead, item.Offset, (item.Offset + item.Length));

        if ((_subMatch.Index != 0) && (_subMatch[0].NestMatch.Index != 0)) {
            String       name;
            String       value_id;
            String       key_id;
            Match const *sm = &(_subMatch[0]);

            // set="(Array_name)" value="s_value" key="s_key"
            Match * m;
            UNumber start_at;

            for (UNumber i = 0; i < sm->NestMatch.Index; i++) {
                m = &(sm->NestMatch[i]);
                if (m->Offset > 5) { // for the length of: set= or var= + 1
                    start_at = m->Offset;

                    while ((start_at <= m->Offset) && (start_at > item.Offset)) {
                        --start_at;

                        if (block[start_at] == L't') { // se[t]
                            name = String::Part(block.Str, (m->Offset + m->OLength),
                                                (m->Length - (m->CLength + m->OLength)));
                            break;
                        }

                        if (block[start_at] == L'e') { // valu[e]
                            value_id = String::Part(block.Str, (m->Offset + m->OLength),
                                                    (m->Length - (m->CLength + m->OLength)));
                            break;
                        }

                        if (block[start_at] == L'y') { // ke[y]
                            key_id = String::Part(block.Str, (m->Offset + m->OLength),
                                                  (m->Length - (m->CLength + m->OLength)));
                            break;
                        }
                    }
                }
            }

            if ((name.Length != 0) && (value_id.Length != 0)) {
                String &&_content = Template::Repeat(
                    String::Part(block.Str, (sm->Offset + sm->Length), (item.Length - (sm->Length + item.CLength))),
                    name, value_id, key_id, other);

                if (_content.Length != 0) {
                    return Engine::Parse(_content, Engine::Search(_content, _tagsAll), 0, 0, other);
                }
            }
        }

        return L"";
    }

    static String Repeat(String const &content, String const &name, String const &value_id, String const &key_id,
                         void *other) noexcept {
        Entry *   _entry;
        Document *_storage = (static_cast<Document *>(other))->GetSource(&_entry, name, 0, name.Length);

        if (_storage == nullptr) {
            return L"";
        }

        StringStream rendered;
        Expressions  loop_exp;
        String       num;

        Expression value_ex;
        value_ex.Keyword = value_id;

        Expression key_ex;
        key_ex.Keyword = key_id;

        loop_exp.Add(&value_ex);

        if (key_id.Length != 0) {
            loop_exp.Add(&key_ex);
        }

        Array<Match> const items = Engine::Search(content, loop_exp);
        if (_entry->Type == VType::DocumentT) {
            if (_storage->Ordered) {
                if (_storage->Strings.Index != 0) {
                    for (UNumber i = 0; i < _storage->Strings.Index; i++) {
                        num = String::FromNumber(i);

                        if (key_id.Length != 0) {
                            loop_exp[1]->Replace = num;
                        }

                        loop_exp[0]->Replace = _storage->Strings[i];

                        rendered += Engine::Parse(content, items);
                    }
                } else if (_storage->Numbers.Index != 0) {
                    for (UNumber i = 0; i < _storage->Numbers.Index; i++) {
                        num = String::FromNumber(i);

                        if (key_id.Length != 0) {
                            loop_exp[1]->Replace = num;
                        }

                        loop_exp[0]->Replace = String::FromNumber(_storage->Numbers[i], 1, 0, 3);

                        rendered += Engine::Parse(content, items, 0, 0, other);
                    }
                }
            } else {
                String value;
                for (UNumber i = 0; i < _storage->Keys.Index; i++) {
                    if (key_id.Length != 0) {
                        loop_exp[1]->Replace = _storage->Keys[i];
                    }

                    _storage->GetString(value, _storage->Keys[i]);
                    loop_exp[0]->Replace = value;

                    rendered += Engine::Parse(content, items, 0, 0, other);
                }
            }
        }

        return rendered.Eject();
    }

    static Expressions const &_getTagsVara() noexcept {
        static Expression  TagVar;
        static Expression  VarTail;
        static Expressions tags;

        if (tags.Index == 0) {
            // Variables evaluation.
            VarTail.ParseCB = &(Template::RenderVar);
            // {v:var_name}
            TagVar.Keyword   = L"{v:";
            VarTail.Keyword  = L'}';
            TagVar.Connected = &VarTail;
            VarTail.Flag     = Flags::TRIM;
            tags             = Expressions().Add(&TagVar);
        }

        return tags;
    }

    static Expressions const &_getTagsQuotes() noexcept {
        static Expression  TagQuote;
        static Expression  QuoteTail;
        static Expressions tags;

        if (tags.Index == 0) {
            TagQuote.Keyword   = L'"';
            QuoteTail.Keyword  = L'"';
            TagQuote.Connected = &QuoteTail;
            tags               = Expressions().Add(&TagQuote);
        }

        return tags;
    }

    static Expressions const &_getTagsHead() noexcept {
        static Expression         TagHead;
        static Expression         TagHead_T;
        static Expressions        tags;
        static Expressions const &_tagsQuotes = _getTagsQuotes();

        if (tags.Index == 0) {
            TagHead.Keyword   = L"<";
            TagHead_T.Keyword = L'>';
            TagHead_T.Flag    = Flags::ONCE;
            TagHead.Connected = &TagHead_T;
            // Nest to prevent matching ">" bigger sign inside if statement.
            TagHead_T.NestExprs = _tagsQuotes;
            tags                = Expressions().Add(&TagHead);
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

        static Expressions const &_tagsVars = _getTagsVara();

        static Expressions tags;

        if (tags.Index == 0) {
            // Inline if evaluation.
            IifTail.ParseCB = &(Template::RenderIIF);
            //{iif case="3 == 3" true="Yes" false="No"}
            TagIif.Keyword    = L"{iif";
            IifTail.Keyword   = L'}';
            TagIif.Connected  = &IifTail;
            IifTail.Flag      = Flags::BUBBLE;
            IifTail.NestExprs = Expressions().Add(&TagIif).Add(_tagsVars); // Nested by itself and TagVars
            /////////////////////////////////

            // If spliter.
            // <else />
            TagELseIf.Keyword   = L"<else";
            ELseIfTail.Keyword  = L"/>";
            ELseIfTail.Flag     = Flags::SPLIT;
            TagELseIf.Connected = &ELseIfTail;
            /////////////////////////////////

            // Shallow IF
            // To not match anything inside inner if until it's needed.
            ShallowTagIf.Keyword    = L"<if";
            ShallowIfTail.Keyword   = L"</if>";
            ShallowTagIf.Connected  = &ShallowIfTail;
            ShallowIfTail.NestExprs = Expressions().Add(&ShallowTagIf);

            // If evaluation.
            IfTail.ParseCB = &(Template::RenderIF);
            // <if case="{case}">html code</if>
            TagIf.Keyword    = L"<if";
            IfTail.Keyword   = L"</if>";
            TagIf.Connected  = &IfTail;
            IfTail.Flag      = Flags::SPLITNEST;
            IfTail.NestExprs = Expressions().Add(&ShallowTagIf).Add(&TagELseIf);
            /////////////////////////////////

            // Loop evaluation.
            LoopTail.ParseCB = &(Template::RenderLoop);
            // <loop set="abc2" var="loopId">
            //     <span>loopId): -{v:abc2[loopId]}</span>
            // </loop>
            TagLoop.Keyword    = L"<loop";
            LoopTail.Keyword   = L"</loop>";
            TagLoop.Connected  = &LoopTail;
            LoopTail.NestExprs = Expressions().Add(&TagLoop); // Nested by itself
            /////////////////////////////////

            // Math Tag.
            MathTail.ParseCB = &(Template::RenderMath);
            // {math:5+6*8*(8+3)}
            TagMath.Keyword    = L"{math:";
            MathTail.Keyword   = L'}';
            TagMath.Connected  = &MathTail;
            MathTail.Flag      = Flags::TRIM | Flags::BUBBLE;
            MathTail.NestExprs = Expressions().Add(_tagsVars);
            /////////////////////////////////

            tags = Expressions().Add(_tagsVars).Add(&TagMath).Add(&TagIif).Add(&TagIf).Add(&TagLoop);
        }

        return tags;
    }
};

} // namespace Qentem
#endif
