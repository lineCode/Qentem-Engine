
/**
 * Qentem Template
 *
 * @brief     Generate dynamic HTML code from a template.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Addon/Template.hpp"
#include "Addon/Test.hpp"

using Qentem::String;
using Qentem::Engine::Flags;

Qentem::Template::Template() noexcept {

    // Variables evaluation.
    VarNext.ParseCB = &(Template::RenderVar);
    VarNext.Pocket  = &(this->Pocket);
    // {v-var_name}
    TagVar.Keyword  = L"{v-";
    VarNext.Keyword = L"}";
    TagVar.Next     = &VarNext;
    VarNext.Flag    = Flags::BUBBLE;
    // {v-var_{v-var2_{v-var3_id}}}
    VarNext.NestExprs.Add(&TagVar); // Nest itself
    Pocket.Tags.Add(&TagVar);
    Pocket.TagsVars.Add(&TagVar);
    /////////////////////////////////

    // Inline if evaluation.
    IifNext.ParseCB = &(Template::RenderIIF);
    IifNext.Pocket  = &(this->Pocket);
    //{iif case="3 == 3" true="Yes" false="No"}
    TagIif.Keyword  = L"{iif";
    IifNext.Keyword = L"}";
    TagIif.Next     = &IifNext;
    IifNext.Flag    = Flags::BUBBLE;
    IifNext.NestExprs.Add(&TagVar); // nested by TagVars'
    Pocket.Tags.Add(&TagIif);
    /////////////////////////////////

    // TagsQuotes.
    TagQuote.Keyword  = L"\"";
    QuoteNext.Keyword = L"\"";
    TagQuote.Next     = &QuoteNext;
    QuoteNext.Pocket  = &(this->Pocket);
    Pocket.TagsQuotes.Add(&TagQuote);
    /////////////////////////////////

    // If spliter.
    // <else />
    TagELseIf.Keyword  = L"<else";
    ELseIfNext.Keyword = L"/>";
    ELseIfNext.Flag    = Flags::SPLIT;
    TagELseIf.Next     = &ELseIfNext;
    ELseIfNext.SubExprs.Add(&TagQuote);
    /////////////////////////////////

    // If's head.
    iFsHead.Keyword   = L"<if";
    iFsHead_T.Keyword = L">";
    iFsHead_T.Flag    = Flags::ONCE;
    iFsHead.Next      = &iFsHead_T;
    // Nest to prevent matching ">" bigger sign inside if statement.
    iFsHead_T.NestExprs.Add(&TagQuote);
    /////////////////////////////////

    // If evaluation.
    IfNext.ParseCB = &(Template::RenderIF);
    IfNext.Pocket  = &(this->Pocket);
    // <if case="{case}">html code</if>
    TagIf.Keyword  = L"<if";
    IfNext.Keyword = L"</if>";
    TagIf.Next     = &IfNext;
    IfNext.Flag    = Flags::SPLITNEST;
    IfNext.SubExprs.Add(&iFsHead);
    IfNext.NestExprs.Add(&TagIf).Add(&TagELseIf);
    Pocket.Tags.Add(&TagIf);
    /////////////////////////////////

    // Loop's head.
    LoopsHead.Keyword   = L"<loop";
    LoopsHead_T.Keyword = L">";
    LoopsHead.Next      = &LoopsHead_T;
    LoopsHead_T.Flag    = Flags::ONCE;
    LoopsHead_T.SubExprs.Add(&TagQuote);
    /////////////////////////////////

    // Loop evaluation.
    LoopNext.ParseCB = &(Template::RenderLoop);
    LoopNext.Pocket  = &(this->Pocket);
    // <loop set="abc2" var="loopId">
    //     <span>loopId): -{v-abc2[loopId]}</span>
    // </loop>
    TagLoop.Keyword  = L"<loop";
    LoopNext.Keyword = L"</loop>";
    TagLoop.Next     = &LoopNext;
    LoopNext.SubExprs.Add(&LoopsHead);
    LoopNext.NestExprs.Add(&TagLoop); // nested by itself
    Pocket.Tags.Add(&TagLoop);
    /////////////////////////////////
}

String Qentem::Template::Render(const String &content, Tree *data) noexcept {
    if (data != nullptr) {
        this->Pocket.Data = data;
    }

    return Engine::Parse(content, Engine::Search(content, this->Pocket.Tags));
}

// e.g. {v-var_name}
// e.g. {v-var_name[id]}
// Nest: {v-var_{v-var2_{v-var3_id}}}
String Qentem::Template::RenderVar(const String &block, const Match &item) noexcept {
    String id = String::Part(block, item.OLength, (block.Length - (item.OLength + item.CLength)));
    if (id.Length != 0) {
        String *val = (static_cast<Template::PocketT *>(item.Expr->Pocket))->Data->GetValue(id);
        if (val != nullptr) {
            return *val;
        }
    }
    return id;
}

// <if case="{case}">html code</if>
// <if case="{case}">html code1 <else /> html code2</if>
// <if case="{case1}">html code1 <elseif case={case2} /> html code2</if>
// <if case="{case}">html code <if case="{case2}" />additional html code</if></if>
bool Qentem::Template::EvaluateIF(const String &block, const Match &if_case) noexcept {
    UNumber offset = (if_case.Offset + if_case.OLength);
    UNumber length = (if_case.Length - (if_case.OLength + if_case.CLength));
    auto    pocket = static_cast<PocketT *>(if_case.Expr->Pocket);

    String statement = Engine::Parse(block, Engine::Search(block, pocket->TagsVars, offset, (offset + length)), offset,
                                     (offset + length));

    return (pocket->_Alu.Evaluate(statement) != 0.0);
}

String Qentem::Template::RenderIF(const String &block, const Match &item) noexcept {
    // Nothing is processed inside the match before checking if the condition is TRUE.
    bool _true = false;

    if (item.SubMatch.Size != 0) {
        Match *sm = &(item.SubMatch[0]);

        if (sm->NestMatch.Size != 0) {
            Match *nm = &(sm->NestMatch[0]);
            _true     = Template::EvaluateIF(block, *nm);

            // inner content of if
            UNumber offset = (sm->Offset + sm->Length);
            UNumber length = (item.Length - (sm->Length + item.CLength));

            if (item.NestMatch.Size != 0) {
                nm = &(item.NestMatch[0]);
                if (nm->NestMatch.Size > 0) {
                    // if_else (splitted content)
                    for (UNumber i = 0; i < nm->NestMatch.Size; i++) {
                        if ((nm->NestMatch[i].SubMatch.Size == 0) ||
                            Template::EvaluateIF(block, nm->NestMatch[i].SubMatch[0])) {
                            // inner content of the next part.
                            Match *inm = &(nm->NestMatch[i + 1]);
                            offset     = inm->Offset;
                            length     = inm->Length;
                            _true      = true;
                            break;
                        }
                    }
                }
            }

            if (_true) {
                return Engine::Parse(
                    block,
                    Engine::Search(block, (static_cast<PocketT *>(item.Expr->Pocket))->Tags, offset, (offset + length)),
                    offset, (offset + length));
            }
        }
    }

    return L"";
}

// {iif case="3 == 3" true="Yes" false="No"}
// {iif case="{v-var_five} == 5" true="5" false="no"}
// {iif case="{v-var_five} == 5" true="{v-var_five} is equal to 5" false="no"}
// {iif case="3 == 3" true="Yes" false="No"}
String Qentem::Template::RenderIIF(const String &block, const Match &item) noexcept {
    const Array<Match> items = Engine::Search(block, (static_cast<PocketT *>(item.Expr->Pocket))->TagsQuotes);
    if (items.Size == 0) {
        return L"";
    }

    Match *m;
    String iif_case  = L"";
    String iif_false = L"";
    String iif_true  = L"";

    // case="[statement]" true="[Yes]" false="[No]"
    for (UNumber i = 0; i < items.Size; i++) {
        // With this method, order is not necessary of case=, true=, false=
        m = &(items[i]);
        if (m->Offset > 3) {
            switch (block.Str[(m->Offset - 4)]) {
                case 'a': // c[a]se
                    iif_case = String::Part(block, (m->Offset + m->OLength), (m->Length - (m->CLength + m->OLength)));
                    break;
                case 'r': // t[r]ue
                    iif_true = String::Part(block, (m->Offset + m->OLength), (m->Length - (m->CLength + m->OLength)));
                    break;
                case 'l': // fa[l]se
                    iif_false = String::Part(block, (m->Offset + m->OLength), (m->Length - (m->CLength + m->OLength)));
                    break;
            }
        }
    }

    if ((iif_case.Length != 0) && (iif_true.Length != 0) &&
        ((static_cast<PocketT *>(item.Expr->Pocket))->_Alu.Evaluate(iif_case) != 0.0)) {
        return iif_true;
    }

    return (iif_false.Length != 0) ? iif_false : L"";
}

// <loop set="abc2" var="loopId">
//     <span>loopId): -{v-abc2[loopId]}</span>
// </loop>
String Qentem::Template::RenderLoop(const String &block, const Match &item) noexcept {
    // To match: <loop (set="abc2" var="loopId")>
    if ((item.SubMatch.Size != 0) && (item.SubMatch[0].SubMatch.Size != 0)) {
        Match *      m;
        String       name   = L"";
        String       var_id = L"";
        const Match *sm     = &(item.SubMatch[0]);

        // When bubbling
        // adj_offset = ((m->Offset + m->OLength) - (item.Offset + item.OLength));
        // When not
        // adj_offset = (m->Offset + m->OLength);

        // set="(Array_name)" var="(var_id)"
        for (UNumber i = 0; i < sm->SubMatch.Size; i++) {
            m = &(sm->SubMatch[i]);
            if (m->Offset > 1) {
                switch (block.Str[(m->Offset - 2)]) {
                    case 't': // se[t]
                        name = String::Part(block, (m->Offset + m->OLength), (m->Length - (m->CLength + m->OLength)));
                        break;
                    case 'r': // va[r]
                        var_id = String::Part(block, (m->Offset + m->OLength), (m->Length - (m->CLength + m->OLength)));
                        break;
                }
            }
        }

        if ((name.Length != 0) && (var_id.Length != 0)) {
            const PocketT *pocket  = static_cast<PocketT *>(item.Expr->Pocket);
            UNumber        offset  = sm->Offset + sm->Length;
            UNumber        length  = (item.Length - (sm->Length + item.CLength));
            String         content = Template::DoLoop(String::Part(block, offset, length), name, var_id, pocket->Data);

            // When bubbling
            // const UNumber   offset  = (sm->Length - item.OLength);
            // const UNumber   length  = ((item.Length - offset) - (item.OLength + item.CLength));
            // const String   content = Template::DoLoop(String::Part(block,offset, length), name, var_id,
            // pocket->Data);

            if (content.Length != 0) {
                return Engine::Parse(content, Engine::Search(content, pocket->Tags));
            }
        }
    }

    return L"";
}

String Qentem::Template::DoLoop(const String &content, const String &name, const String &var_name,
                                Tree *storage) noexcept {
    String rendered = L"";
    String reminder = L"";
    String id       = L"";

    UNumber index = 0;
    VType   type;
    String  key = name;

    while (true) {
        if (!(Qentem::Tree::DecodeKey(key, id, reminder)) || !(storage->GetIndex(key, index)) ||
            ((type = storage->Types[index]) == VType::NullT)) {
            return rendered;
        }

        if ((id.Length == 0) || (type != VType::TreeT)) {
            break;
        }

        key = id + reminder;

        storage = &(storage->VArray[storage->ExactID[index]]);
    }

    Expression ex   = Expression();
    ex.Keyword      = var_name;
    Expressions ser = Expressions();
    ser.Add(&ex);

    const Array<Match> items = Engine::Search(content, ser);
    // Feature: Use StringStream!!!
    if (type == VType::ArrayT) {
        const Array<String> *st = &(storage->Arrays[storage->ExactID[index]]);
        for (UNumber i = 0; i < st->Size; i++) {
            ser[0]->Replace = String::FromNumber(static_cast<double>(i));
            rendered += Engine::Parse(content, items);
        }
    } else if (type == VType::TreeT) {
        const Array<String> *va = &(storage->VArray[storage->ExactID[index]].Keys);
        for (UNumber i = 0; i < va->Size; i++) {
            ser[0]->Replace = (*va)[i];
            rendered += Engine::Parse(content, items);
        }
    }

    return rendered;
}
