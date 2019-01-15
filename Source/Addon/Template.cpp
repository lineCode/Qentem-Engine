
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

Qentem::Template::Template() noexcept {
    // Variables evaluation.
    TagVar.ParseCB = &(Template::RenderVar);
    TagVar.Pocket  = &(this->Pocket);
    // {qt-var_name}
    TagVar.Keyword  = L"{qt-";
    VarTail.Keyword = L"}";
    TagVar.Tail     = &VarTail;
    TagVar.Flag     = Flags::BUBBLE;
    // {qt-var_{qt-var2_{qt-var3_id}}}
    TagVar.NestExprs.Add(&TagVar); // nest itself
    Pocket.Tags.Add(&TagVar);
    Pocket.TagsVars.Add(&TagVar);
    /////////////////////////////////

    // Inline if evaluation.
    TagIif.ParseCB = &(Template::RenderIIF);
    TagIif.Pocket  = &(this->Pocket);
    //{qt:iif case="3 == 3" true="Yes" false="No"}
    TagIif.Keyword  = L"{qt:iif";
    IifTail.Keyword = L"}";
    TagIif.Tail     = &IifTail;
    TagIif.Flag     = Flags::BUBBLE;
    TagIif.NestExprs.Add(&TagVar); // nested by TagVars'
    Pocket.Tags.Add(&TagIif);
    /////////////////////////////////

    // TagsQuotes
    TagQuote.Keyword  = L"\"";
    QuoteTail.Keyword = L"\"";
    TagQuote.Tail     = &QuoteTail;
    Pocket.TagsQuotes.Add(&TagQuote);
    /////////////////////////////////

    // If's head
    iFsHead.Keyword  = L"<qt:if";
    iFsHeadT.Keyword = L">";
    iFsHead.Tail     = &iFsHeadT;
    // Nest to prevent matching > bigger sign in a condition
    iFsHead.NestExprs.Add(&TagQuote);
    /////////////////////////////////

    // If evaluation.
    TagIf.ParseCB = &Template::RenderIF;
    TagIf.Pocket  = &(this->Pocket);
    // <qt:if case="{case}">html code</qt:if>
    TagIf.Keyword  = L"<qt:if";
    IfTail.Keyword = L"</qt:if>";
    TagIf.Tail     = &IfTail;
    TagIf.SubExprs.Add(&iFsHead);
    TagIf.NestExprs.Add(&TagIf);
    Pocket.Tags.Add(&TagIf);
    /////////////////////////////////

    // Loop's head
    LoopsHead.Keyword  = L"<qt:loop";
    LoopsHeadT.Keyword = L">";
    LoopsHead.Tail     = &LoopsHeadT;
    LoopsHead.SubExprs.Add(&TagQuote);
    /////////////////////////////////

    // Loop evaluation.
    TagLoop.ParseCB = &Template::RenderLoop;
    TagLoop.Pocket  = &(this->Pocket);
    // <qt:loop set="abc2" var="loopId">
    //     <span>loopId): -{qt:abc2[loopId]}</span>
    // </qt:loop>
    TagLoop.Keyword  = L"<qt:loop";
    LoopTail.Keyword = L"</qt:loop>";
    TagLoop.Tail     = &LoopTail;
    TagLoop.SubExprs.Add(&LoopsHead);
    TagLoop.NestExprs.Add(&TagLoop); // nested by itself
    Pocket.Tags.Add(&TagLoop);
}

String Qentem::Template::Render(const String &content, QArray *data) noexcept {
    if (data != nullptr) {
        this->Pocket.Data = data;
    }
    return Engine::Parse(content, Engine::Search(content, this->Pocket.Tags));
}

// e.g. {qt-var_name}
// e.g. {qt-var_name[id]}
// Nest: {qt-var_{qt-var2_{qt-var3_id}}}
String Qentem::Template::RenderVar(const String &block, const Match &match) noexcept {
    if (block.Length != 0) {
        String *val = ((Template::PocketT *)match.Expr->Pocket)->Data->GetValue(block);
        if (val != nullptr) {
            return *val;
        }
    }
    return block;
}

// {qt:iif case="3 == 3" true="Yes" false="No"}
// {qt:iif case="{qt-var_five} == 5" true="5" false="no"}
// {qt:iif case="{qt-var_five} == 5" true="{qt-var_five} is equal to 5" false="no"}
// {qt:iif case="3 == 3" true="Yes" false="No"}
String Qentem::Template::RenderIIF(const String &block, const Match &match) noexcept {
    const Array<Match> items = Engine::Search(block, ((PocketT *)(match.Expr->Pocket))->TagsQuotes);
    if (items.Size == 0) {
        return L"";
    }

    Match *m;
    String iif_case  = L"";
    String iif_false = L"";
    String iif_true  = L"";

    // case="[statement]" true="[Yes]" false="[No]"
    for (size_t i = 0; i < items.Size; i++) {
        // With this method, order is not necessary of case=, true=, false=
        m = &(items[i]);
        if (m->Offset > 3) {
            switch (block.Str[(m->Offset - 4)]) {
                case 'a': // c[a]se
                    iif_case = block.Part((m->Offset + m->OLength), (m->Length - (m->CLength + m->OLength)));
                    break;
                case 'r': // t[r]ue
                    iif_true = block.Part((m->Offset + m->OLength), (m->Length - (m->CLength + m->OLength)));
                    break;
                case 'l': // fa[l]se
                    iif_false = block.Part((m->Offset + m->OLength), (m->Length - (m->CLength + m->OLength)));
                    break;
            }
        }
    }

    if ((iif_case.Length != 0) && (iif_true.Length != 0) &&
        (((PocketT *)(match.Expr->Pocket))->_Alu.Evaluate(iif_case) != 0.0f)) {
        return iif_true;
    } else {
        return (iif_false.Length != 0) ? iif_false : L"";
    }
}

// <qt:if case="{case}">html code</qt:if>
// <qt:if case="{case}">html code1 <else> html code2</qt:if>
// <qt:if case="{case1}">html code1 <qt:elseif case={case2}> html code2</qt:if>
// <qt:if case="{case}">html code <qt:if case="{case2}">additional html code</qt:if></qt:if>
String Qentem::Template::RenderIF(const String &block, const Match &match) noexcept {
    // Nothing is processed inside the match before checking if "if-else" is TRUE.
    if (match.SubMatch.Size != 0) {
        Match *sm = &(match.SubMatch[0]);

        if (sm->NestMatch.Size != 0) {
            PocketT *pocket = (PocketT *)(match.Expr->Pocket);

            Match *nm      = &(sm->NestMatch[0]);
            String if_case = block.Part((nm->Offset + nm->OLength), (nm->Length - (nm->OLength + nm->CLength)));

            // To only replace variables inside the statement.
            if_case = Engine::Parse(if_case, Engine::Search(if_case, pocket->TagsVars));
            if ((pocket->_Alu.Evaluate(if_case) != 0)) {
                // inner content of if
                size_t offset  = sm->Offset + sm->Length;
                size_t length  = (match.Length - (sm->Length + match.CLength));
                String content = block.Part(offset, length);

                return Engine::Parse(content, Engine::Search(content, pocket->Tags));
            }
        }
    }
    return L"";
}

// <qt:loop set="abc2" var="loopId">
//     <span>loopId): -{qt:abc2[loopId]}</span>
// </qt:loop>
String Qentem::Template::RenderLoop(const String &block, const Match &match) noexcept {
    // To match: <qt:loop (set="abc2" var="loopId")>
    if ((match.SubMatch.Size != 0) && (match.SubMatch[0].SubMatch.Size != 0)) {
        Match *      m;
        String       name   = L"";
        String       var_id = L"";
        const Match *sm     = &(match.SubMatch[0]);

        // When bubbling
        // adj_offset = ((m->Offset + m->OLength) - (match.Offset + match.OLength));
        // When not
        // adj_offset = (m->Offset + m->OLength);

        // set="(array_name)" var="(var_id)"
        for (size_t i = 0; i < sm->SubMatch.Size; i++) {
            m = &(sm->SubMatch[i]);
            if (m->Offset > 1) {
                switch (block.Str[(m->Offset - 2)]) {
                    case 't': // se[t]
                        name = block.Part((m->Offset + m->OLength), (m->Length - (m->CLength + m->OLength)));
                        break;
                    case 'r': // va[r]
                        var_id = block.Part((m->Offset + m->OLength), (m->Length - (m->CLength + m->OLength)));
                        break;
                }
            }
        }

        if ((name.Length != 0) && (var_id.Length != 0)) {
            const PocketT *pocket  = (PocketT *)(match.Expr->Pocket);
            size_t         offset  = sm->Offset + sm->Length;
            size_t         length  = (match.Length - (sm->Length + match.CLength));
            String         content = Template::DoLoop(block.Part(offset, length), name, var_id, pocket->Data);

            // When bubbling
            // const size_t   offset  = (sm->Length - match.OLength);
            // const size_t   length  = ((match.Length - offset) - (match.OLength + match.CLength));
            // const String   content = Template::DoLoop(block.Part(offset, length), name, var_id, pocket->Data);

            if (content.Length != 0) {
                return Engine::Parse(content, Engine::Search(content, pocket->Tags));
            }
        }
    }

    return L"";
}

String Qentem::Template::DoLoop(const String &content, const String &name, const String &var_name,
                                QArray *storage) noexcept {
    String rendered = L"";
    String reminder = L"";
    String id       = L"";

    size_t index = 0;
    VType  type;
    String key = name;

    while (true) {
        if (!(storage->DecodeKey(key, id, reminder)) || !(storage->GetIndex(key, index)) ||
            ((type = storage->Types[index]) == VType::NullT)) {
            return rendered;
        }

        if ((id.Length == 0) || (type != VType::QArrayT)) {
            break;
        }

        key = id + reminder;

        storage = &(storage->VArray[storage->RealID[index]]);
    }

    Expression  ex  = Expression();
    Expressions ser = Expressions().Add(&ex);
    ser[0]->Keyword = var_name;

    const Array<Match> items = Engine::Search(content, ser);
    // Feature: Use StringStream!!!
    if (type == VType::ArrayT) {
        const Array<String> *st = &(storage->Arrays[storage->RealID[index]]);
        for (size_t i = 0; i < st->Size; i++) {
            ser[0]->Replace = String::ToString((float)i);
            rendered += Engine::Parse(content, items);
        }
    } else if (type == VType::QArrayT) {
        const Array<String> *va = &(storage->VArray[storage->RealID[index]].Keys);
        for (size_t i = 0; i < va->Size; i++) {
            ser[0]->Replace = (*va)[i];
            rendered += Engine::Parse(content, items);
        }
    }

    return rendered;
}
