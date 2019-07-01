
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
    static Document *Data;

    static Expressions const TagsVars;
    static Expressions const TagsQuotes;
    static Expressions const TagsHead;

    static Expressions const TagsAll; // All

    static Expressions _getTagsVara() noexcept {
        static Expression TagVar;
        static Expression VarTail;

        // Variables evaluation.
        VarTail.ParseCB = &(Template::RenderVar);
        // {v:var_name}
        TagVar.Keyword   = L"{v:";
        VarTail.Keyword  = L'}';
        TagVar.Connected = &VarTail;
        VarTail.Flag     = Flags::TRIM;

        return Expressions().Add(&TagVar);
    }

    static Expressions _getTagsQuotes() noexcept {
        static Expression TagQuote;
        static Expression QuoteTail;

        TagQuote.Keyword   = L'"';
        QuoteTail.Keyword  = L'"';
        TagQuote.Connected = &QuoteTail;

        return Expressions().Add(&TagQuote);
    }

    static Expressions _getTagsHead() noexcept {
        static Expression TagHead;
        static Expression TagHead_T;

        TagHead.Keyword   = L"<";
        TagHead_T.Keyword = L'>';
        TagHead_T.Flag    = Flags::ONCE;
        TagHead.Connected = &TagHead_T;
        // Nest to prevent matching ">" bigger sign inside if statement.
        TagHead_T.NestExprs.Add(TagsQuotes);

        return Expressions().Add(&TagHead);
    }

    static Expressions _getTagsAll() noexcept {
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

        // Inline if evaluation.
        IifTail.ParseCB = &(Template::RenderIIF);
        //{iif case="3 == 3" true="Yes" false="No"}
        TagIif.Keyword   = L"{iif";
        IifTail.Keyword  = L'}';
        TagIif.Connected = &IifTail;
        IifTail.Flag     = Flags::BUBBLE;
        IifTail.NestExprs.Add(&TagIif).Add(TagsVars); // Nested by itself and TagVars
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
        ShallowTagIf.Keyword   = L"<if";
        ShallowIfTail.Keyword  = L"</if>";
        ShallowTagIf.Connected = &ShallowIfTail;
        ShallowIfTail.NestExprs.Add(&ShallowTagIf);

        // If evaluation.
        IfTail.ParseCB = &(Template::RenderIF);
        // <if case="{case}">html code</if>
        TagIf.Keyword   = L"<if";
        IfTail.Keyword  = L"</if>";
        TagIf.Connected = &IfTail;
        IfTail.Flag     = Flags::SPLITNEST;
        IfTail.NestExprs.Add(&ShallowTagIf).Add(&TagELseIf);
        /////////////////////////////////

        // Loop evaluation.
        LoopTail.ParseCB = &(Template::RenderLoop);
        // <loop set="abc2" var="loopId">
        //     <span>loopId): -{v:abc2[loopId]}</span>
        // </loop>
        TagLoop.Keyword   = L"<loop";
        LoopTail.Keyword  = L"</loop>";
        TagLoop.Connected = &LoopTail;
        LoopTail.NestExprs.Add(&TagLoop); // Nested by itself
        /////////////////////////////////

        // Math Tag.
        MathTail.ParseCB = &(Template::RenderMath);
        // {math:5+6*8*(8+3)}
        TagMath.Keyword   = L"{math:";
        MathTail.Keyword  = L'}';
        TagMath.Connected = &MathTail;
        MathTail.Flag     = Flags::TRIM;

        Expressions TagsMath = Expressions().Add(&TagMath);
        /////////////////////////////////

        return Expressions().Add(TagsVars).Add(TagsMath).Add(&TagIif).Add(&TagIf).Add(&TagLoop);
    }

    static String Render(String const &content, Document *data) noexcept {
        Template::Data = data;
        return Engine::Parse(content, Engine::Search(content, Template::TagsAll));
    }

    // e.g. {v:var_name}
    // e.g. {v:var_name[id]}
    // Nest: {v:var_{v:var2_{v:var3_id}}}
    static String RenderVar(String const &block, Match const &item) noexcept {
        String value;
        if (Template::Data->GetString(value, block, (item.Offset + item.OLength),
                                      (item.Length - (item.CLength + item.OLength)))) {
            return value;
        }

        return String::Part(block, (item.Offset + item.OLength), (item.Length - (item.CLength + item.OLength)));
        // return String::Part(block, item.OLength, (block.Length - (item.OLength + item.CLength))); // if Bubbling
    }

    static String RenderMath(String const &block, Match const &item) noexcept {
        String value = String::Part(block, (item.Offset + item.OLength), (item.Length - (item.CLength + item.OLength)));
        return String::FromNumber(ALU::Evaluate(value), 1, 0, 3);
    }

    // {iif case="3 == 3" true="Yes" false="No"}
    // {iif case="{v:var_five} == 5" true="5" false="no"}
    // {iif case="{v:var_five} == 5" true="{v:var_five} is equal to 5" false="no"}
    // {iif case="3 == 3" true="Yes" false="No"}
    static String RenderIIF(String const &block, Match const &item) noexcept {
        Array<Match> const &&items = Engine::Search(block, Template::TagsQuotes);
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
                            String::Part(block, (m->Offset + m->OLength), (m->Length - (m->CLength + m->OLength)));
                        break;
                    }

                    if (block[start_at] == L'r') { // t[r]ue
                        iif_true =
                            String::Part(block, (m->Offset + m->OLength), (m->Length - (m->CLength + m->OLength)));
                        break;
                    }

                    if (block[start_at] == L'l') { // fa[l]se
                        iif_false =
                            String::Part(block, (m->Offset + m->OLength), (m->Length - (m->CLength + m->OLength)));
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
    static bool EvaluateIF(String const &block, Match const &if_case) noexcept {
        UNumber const offset = (if_case.Offset + if_case.OLength);
        UNumber const length = (if_case.Length - (if_case.OLength + if_case.CLength));

        String statement = Engine::Parse(block, Engine::Search(block, Template::TagsVars, offset, (offset + length)),
                                         offset, (offset + length));

        return (ALU::Evaluate(statement) != 0.0);
    }

    static String RenderIF(String const &block, Match const &item) noexcept {
        // Nothing is processed inside the match before checking if the condition is TRUE.
        bool _true = false;

        Array<Match> const &&_subMatch =
            Engine::Search(block, Template::TagsHead, item.Offset, (item.Offset + item.Length));

        if (_subMatch.Index != 0) {
            Match *sm = &(_subMatch[0]);

            if (sm->NestMatch.Index != 0) {
                Match *nm = &(sm->NestMatch[0]);
                _true     = Template::EvaluateIF(block, *nm);

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
                                _subMatch2 = Engine::Search(block, Template::TagsHead, offset, item.NestMatch[i].Offset);

                                // inner content of the next part.
                                offset = item.NestMatch[i].Offset;
                                length = item.NestMatch[i].Length;

                                if ((_subMatch2[0].NestMatch.Index == 0) ||
                                    Template::EvaluateIF(block, _subMatch2[0].NestMatch[0])) {
                                    _true = true;
                                    break;
                                }
                            }
                        }
                    }
                }

                if (_true) {
                    return Engine::Parse(block, Engine::Search(block, Template::TagsAll, offset, (offset + length)),
                                         offset, (offset + length));
                }
            }
        }

        return L"";
    }

    // <loop set="abc2" var="loopId">
    //     <span>loopId): -{v:abc2[loopId]}</span>
    // </loop>
    static String RenderLoop(String const &block, Match const &item) noexcept {
        // To match: <loop (set="abc2" var="loopId")>
        Array<Match> const &&_subMatch =
            Engine::Search(block, Template::TagsHead, item.Offset, (item.Offset + item.Length));

        if ((_subMatch.Index != 0) && (_subMatch[0].NestMatch.Index != 0)) {
            String       name;
            String       var_id;
            Match const *sm = &(_subMatch[0]);

            // set="(Array_name)" var="(var_id)"
            Match * m;
            UNumber start_at;

            for (UNumber i = 0; i < sm->NestMatch.Index; i++) {
                m = &(sm->NestMatch[i]);
                if (m->Offset > 5) { // for the length of: set= or var= + 1
                    start_at = m->Offset;

                    while ((start_at <= m->Offset) && (start_at > item.Offset)) {
                        --start_at;

                        if (block[start_at] == L't') { // se[t]
                            name =
                                String::Part(block, (m->Offset + m->OLength), (m->Length - (m->CLength + m->OLength)));
                            break;
                        }

                        if (block[start_at] == L'r') { // va[r]
                            var_id =
                                String::Part(block, (m->Offset + m->OLength), (m->Length - (m->CLength + m->OLength)));
                            break;
                        }
                    }
                }
            }

            if ((name.Length != 0) && (var_id.Length != 0)) {
                String &&_content = Template::Repeat(
                    String::Part(block, (sm->Offset + sm->Length), (item.Length - (sm->Length + item.CLength))), name,
                    var_id, Template::Data);

                if (_content.Length != 0) {
                    return Engine::Parse(_content, Engine::Search(_content, Template::TagsAll));
                }
            }
        }

        return L"";
    }

    static String Repeat(String const &content, String const &name, String const &var_name, Document *storage) noexcept {
        Entry *         _entry;
        Document const *_storage = storage->GetSource(&_entry, name, 0, name.Length);

        if (_storage == nullptr) {
            return L"";
        }

        StringStream rendered;
        Expression   ex;
        ex.Keyword.SetLength(var_name.Length + 2);

        ex.Keyword = L'[';
        ex.Keyword += var_name;
        ex.Keyword += L']';

        Expressions ser;
        ser.Add(&ex);

        Array<Match> const &&items = Engine::Search(content, ser);

        if (_entry->Type == VType::DocumentT) {
            if (_storage->Ordered) {
                if (_storage->Strings.Index != 0) {
                    for (UNumber i = 0; i < _storage->Strings.Index; i++) {
                        ser[0]->Replace = L'[';
                        ser[0]->Replace += String::FromNumber(i);
                        ser[0]->Replace += L']';

                        rendered += Engine::Parse(content, items);
                    }
                } else if (_storage->Numbers.Index != 0) {
                    for (UNumber i = 0; i < _storage->Numbers.Index; i++) {
                        ser[0]->Replace = L'[';
                        ser[0]->Replace += String::FromNumber(i);
                        ser[0]->Replace += L']';

                        rendered += Engine::Parse(content, items);
                    }
                }
            } else {
                for (UNumber i = 0; i < _storage->Keys.Index; i++) {
                    ser[0]->Replace = L'[';
                    ser[0]->Replace += _storage->Keys[i];
                    ser[0]->Replace += L']';

                    rendered += Engine::Parse(content, items);
                }
            }
        }

        return rendered.Eject();
    }
};

Document *Template::Data = nullptr;

Expressions const Template::TagsVars   = Template::_getTagsVara();
Expressions const Template::TagsQuotes = Template::_getTagsQuotes();
Expressions const Template::TagsHead   = Template::_getTagsHead();
Expressions const Template::TagsAll    = Template::_getTagsAll();

} // namespace Qentem
#endif
