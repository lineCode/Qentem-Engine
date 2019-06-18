
/**
 * Qentem Template
 *
 * @brief     Generate dynamic HTML code from a template.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#ifndef QENTEM_TEMPLATE_H
#define QENTEM_TEMPLATE_H

#include "Extension/ALU.hpp"
#include "Extension/Document.hpp"

namespace Qentem {

struct Template {
    static Document *  Data;
    static Expressions Tags;

    static Expressions TagsVars;
    static Expressions TagsQuotes;
    static Expressions TagsHead;

    static void SetTags() noexcept {
        static Expression TagVar;
        static Expression VarNext;

        static Expression TagIif;
        static Expression IifNext;

        static Expression TagQuote;
        static Expression QuoteNext;

        static Expression TagHead;
        static Expression TagHead_T;

        static Expression TagIf;
        static Expression IfNext;

        static Expression TagELseIf;
        static Expression ELseIfNext;

        static Expression TagLoop;
        static Expression LoopNext;

        // Variables evaluation.
        VarNext.ParseCB = &(Template::RenderVar);
        // {v:var_name}
        TagVar.Keyword   = L"{v:";
        VarNext.Keyword  = L'}';
        TagVar.Connected = &VarNext;
        VarNext.Flag     = Flags::TRIM;
        Tags.Add(&TagVar);
        TagsVars.Add(&TagVar);
        /////////////////////////////////

        // Inline if evaluation.
        IifNext.ParseCB = &(Template::RenderIIF);
        //{iif case="3 == 3" true="Yes" false="No"}
        TagIif.Keyword   = L"{iif";
        IifNext.Keyword  = L'}';
        TagIif.Connected = &IifNext;
        IifNext.Flag     = Flags::BUBBLE;
        IifNext.NestExprs.Add(&TagIif).Add(&TagVar); // nested by itself and TagVars
        Tags.Add(&TagIif);
        /////////////////////////////////

        // TagsQuotes.
        TagQuote.Keyword   = L'"';
        QuoteNext.Keyword  = L'"';
        TagQuote.Connected = &QuoteNext;
        TagsQuotes.Add(&TagQuote);
        /////////////////////////////////

        // If spliter.
        // <else />
        TagELseIf.Keyword   = L"<else";
        ELseIfNext.Keyword  = L"/>";
        ELseIfNext.Flag     = Flags::SPLIT;
        TagELseIf.Connected = &ELseIfNext;
        // ELseIfNext.SubExprs.Add(&TagQuote);
        /////////////////////////////////

        TagHead.Keyword   = L"<";
        TagHead_T.Keyword = L'>';
        TagHead_T.Flag    = Flags::ONCE;
        TagHead.Connected = &TagHead_T;
        // Nest to prevent matching ">" bigger sign inside if statement.
        TagHead_T.NestExprs.Add(&TagQuote);
        TagsHead.Add(&TagHead);
        /////////////////////////////////

        // If evaluation.
        IfNext.ParseCB = &(Template::RenderIF);
        // <if case="{case}">html code</if>
        TagIf.Keyword   = L"<if";
        IfNext.Keyword  = L"</if>";
        TagIf.Connected = &IfNext;
        IfNext.Flag     = Flags::SPLITROOTONLY;
        IfNext.NestExprs.Add(&TagIf).Add(&TagELseIf);
        Tags.Add(&TagIf);
        /////////////////////////////////

        // Loop evaluation.
        LoopNext.ParseCB = &(Template::RenderLoop);
        // <loop set="abc2" var="loopId">
        //     <span>loopId): -{v:abc2[loopId]}</span>
        // </loop>
        TagLoop.Keyword   = L"<loop";
        LoopNext.Keyword  = L"</loop>";
        TagLoop.Connected = &LoopNext;
        LoopNext.NestExprs.Add(&TagLoop); // nested by itself
        Tags.Add(&TagLoop);
        /////////////////////////////////
    }

    static String Render(const String &content, Document *data) noexcept {
        if (Template::Tags.Size == 0) {
            Template::SetTags();
        }

        Template::Data = data;
        return Engine::Parse(content, Engine::Search(content, Template::Tags));
    }

    // e.g. {v:var_name}
    // e.g. {v:var_name[id]}
    // Nest: {v:var_{v:var2_{v:var3_id}}}
    static String RenderVar(const String &block, const Match &item) noexcept {
        String value;
        if (Template::Data->GetString(value, block, (item.Offset + item.OLength),
                                      (item.Length - (item.CLength + item.OLength)))) {
            return value;
        }

        return String::Part(block, (item.Offset + item.OLength), (item.Length - (item.CLength + item.OLength)));
        // return String::Part(block, item.OLength, (block.Length - (item.OLength + item.CLength))); // if Bubbling
    }

    // {iif case="3 == 3" true="Yes" false="No"}
    // {iif case="{v:var_five} == 5" true="5" false="no"}
    // {iif case="{v:var_five} == 5" true="{v:var_five} is equal to 5" false="no"}
    // {iif case="3 == 3" true="Yes" false="No"}
    static String RenderIIF(const String &block, const Match &item) noexcept {
        const Array<Match> &&items = Engine::Search(block, Template::TagsQuotes);
        if (items.Size == 0) {
            return L"";
        }

        Match *m;
        String iif_case;
        String iif_false;
        String iif_true;

        // case="[statement]" true="[Yes]" false="[No]"
        for (UNumber i = 0; i < items.Size; i++) {
            // With this method, order is not necessary of case=, true=, false=
            m = &(items.Storage[i]);
            if (m->Offset > 3) {
                switch (block.Str[(m->Offset - 4)]) {
                    case 'a': // c[a]se
                        iif_case =
                            String::Part(block, (m->Offset + m->OLength), (m->Length - (m->CLength + m->OLength)));
                        break;
                    case 'r': // t[r]ue
                        iif_true =
                            String::Part(block, (m->Offset + m->OLength), (m->Length - (m->CLength + m->OLength)));
                        break;
                    case 'l': // fa[l]se
                        iif_false =
                            String::Part(block, (m->Offset + m->OLength), (m->Length - (m->CLength + m->OLength)));
                        break;
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
    static bool EvaluateIF(const String &block, const Match &if_case) noexcept {
        const UNumber offset = (if_case.Offset + if_case.OLength);
        const UNumber length = (if_case.Length - (if_case.OLength + if_case.CLength));

        String statement = Engine::Parse(block, Engine::Search(block, Template::TagsVars, offset, (offset + length)),
                                         offset, (offset + length));

        return (ALU::Evaluate(statement) != 0.0);
    }

    static String RenderIF(const String &block, const Match &item) noexcept {
        // Nothing is processed inside the match before checking if the condition is TRUE.
        bool _true = false;

        const Array<Match> &&_subMatch =
            Engine::Search(block, Template::TagsHead, item.Offset, (item.Offset + item.Length));

        if (_subMatch.Size != 0) {
            Match *sm = &(_subMatch.Storage[0]);

            if (sm->NestMatch.Size != 0) {
                Match *nm = &(sm->NestMatch.Storage[0]);
                _true     = Template::EvaluateIF(block, *nm);

                // inner content of if
                UNumber offset = (sm->Offset + sm->Length);
                UNumber length = (item.Length - (sm->Length + item.CLength));

                // // if_else (splitted content)
                if (item.NestMatch.Size != 0) {
                    nm = &(item.NestMatch.Storage[0]);
                    if ((Flags::SPLIT & nm->Expr->Flag) != 0) {
                        if (_true) {
                            length = (nm->Length - (offset - nm->Offset));
                        } else {
                            Array<Match> _subMatch2 = Array<Match>();
                            for (UNumber i = 1; i < item.NestMatch.Size; i++) {
                                _subMatch2 =
                                    Engine::Search(block, Template::TagsHead, offset, item.NestMatch.Storage[i].Offset);
                                // return String::Part(block, (_subMatch2.Storage[0].NestMatch.Storage[0].Offset),
                                //                     (_subMatch2.Storage[0].NestMatch.Storage[0].Length));

                                // inner content of the next part.
                                offset = item.NestMatch.Storage[i].Offset;
                                length = item.NestMatch.Storage[i].Length;

                                if ((_subMatch2.Storage[0].NestMatch.Size == 0) ||
                                    Template::EvaluateIF(block, _subMatch2.Storage[0].NestMatch.Storage[0])) {
                                    _true = true;
                                    break;
                                }
                            }
                        }
                    }
                }

                if (_true) {
                    return Engine::Parse(block, Engine::Search(block, Template::Tags, offset, (offset + length)), offset,
                                         (offset + length));
                }
            }
        }

        return L"";
    }

    // <loop set="abc2" var="loopId">
    //     <span>loopId): -{v:abc2[loopId]}</span>
    // </loop>
    static String RenderLoop(const String &block, const Match &item) noexcept {
        // To match: <loop (set="abc2" var="loopId")>
        const Array<Match> &&_subMatch =
            Engine::Search(block, Template::TagsHead, item.Offset, (item.Offset + item.Length));

        if ((_subMatch.Size != 0) && (_subMatch.Storage[0].NestMatch.Size != 0)) {
            String       name;
            String       var_id;
            const Match *sm = &(_subMatch.Storage[0]);

            // set="(Array_name)" var="(var_id)"
            Match *m;
            for (UNumber i = 0; i < sm->NestMatch.Size; i++) {
                m = &(sm->NestMatch.Storage[i]);
                if (m->Offset > 1) {
                    switch (block.Str[(m->Offset - 2)]) {
                        case 't': // se[t]
                            name =
                                String::Part(block, (m->Offset + m->OLength), (m->Length - (m->CLength + m->OLength)));
                            break;
                        case 'r': // va[r]
                            var_id =
                                String::Part(block, (m->Offset + m->OLength), (m->Length - (m->CLength + m->OLength)));
                            break;
                    }
                }
            }

            if ((name.Length != 0) && (var_id.Length != 0)) {
                String &&_content = Template::Repeat(
                    String::Part(block, (sm->Offset + sm->Length), (item.Length - (sm->Length + item.CLength))), name,
                    var_id, Template::Data);

                if (_content.Length != 0) {
                    return Engine::Parse(_content, Engine::Search(_content, Template::Tags));
                }
            }
        }

        return L"";
    }

    static String Repeat(const String &content, const String &name, const String &var_name, Document *storage) noexcept {
        Entry *         _entry;
        const Document *_storage = storage->GetSource(&_entry, name, 0, name.Length);

        if (_storage == nullptr) {
            return L"";
        }

        StringStream rendered;
        Expression   ex;
        ex.Keyword.SetLength(var_name.Length + 2);

        ex.Keyword += L'[';
        ex.Keyword += var_name;
        ex.Keyword += L']';

        Expressions ser;
        ser.Add(&ex);

        const Array<Match> &&items = Engine::Search(content, ser);

        if (_entry->Type == VType::DocumentT) {
            if (_storage->Ordered) {
                if (_storage->Strings.Size != 0) {
                    for (UNumber i = 0; i < _storage->Strings.Size; i++) {
                        ser.Storage[0]->Replace += L'[';
                        ser.Storage[0]->Replace += String::FromNumber(i);
                        ser.Storage[0]->Replace += L']';

                        rendered += Engine::Parse(content, items);
                        ser.Storage[0]->Replace.Length = 0;
                    }
                } else if (_storage->Numbers.Size != 0) {
                    for (UNumber i = 0; i < _storage->Numbers.Size; i++) {
                        ser.Storage[0]->Replace += L'[';
                        ser.Storage[0]->Replace += String::FromNumber(i);
                        ser.Storage[0]->Replace += L']';

                        rendered += Engine::Parse(content, items);
                        ser.Storage[0]->Replace.Length = 0;
                    }
                }
            } else {
                for (UNumber i = 0; i < _storage->Keys.Size; i++) {
                    ser.Storage[0]->Replace += L'[';
                    ser.Storage[0]->Replace = _storage->Keys.Storage[i];
                    ser.Storage[0]->Replace += L']';

                    rendered += Engine::Parse(content, items);
                    ser.Storage[0]->Replace.Length = 0;
                }
            }
        }

        return rendered.Eject();
    }
};

Document *  Template::Data = nullptr;
Expressions Template::Tags = Engine::Expressions();

Expressions Template::TagsVars   = Engine::Expressions();
Expressions Template::TagsQuotes = Engine::Expressions();
Expressions Template::TagsHead   = Engine::Expressions();

} // namespace Qentem
#endif
