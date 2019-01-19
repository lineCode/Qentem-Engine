
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
    TagVar.ParseCB = &(Template::RenderVar);
    TagVar.Pocket  = &(this->Pocket);
    // {qtv-var_name}
    TagVar.Keyword  = L"{qtv-";
    VarTail.Keyword = L"}";
    TagVar.Tail     = &VarTail;
    TagVar.Flag     = Flags::BUBBLE;
    // {qtv-var_{qtv-var2_{qtv-var3_id}}}
    TagVar.NestExprs.Add(&TagVar); // nest itself
    Pocket.Tags.Add(&TagVar);
    Pocket.TagsVars.Add(&TagVar);
    /////////////////////////////////

    // Inline if evaluation.
    TagIif.ParseCB = &(Template::RenderIIF);
    TagIif.Pocket  = &(this->Pocket);
    //{qt-iif case="3 == 3" true="Yes" false="No"}
    TagIif.Keyword  = L"{qt-iif";
    IifTail.Keyword = L"}";
    TagIif.Tail     = &IifTail;
    TagIif.Flag     = Flags::BUBBLE;
    TagIif.NestExprs.Add(&TagVar); // nested by TagVars'
    Pocket.Tags.Add(&TagIif);
    /////////////////////////////////

    // TagsQuotes.
    TagQuote.Keyword  = L"\"";
    QuoteTail.Keyword = L"\"";
    TagQuote.Tail     = &QuoteTail;
    TagQuote.Pocket   = &(this->Pocket);
    Pocket.TagsQuotes.Add(&TagQuote);
    /////////////////////////////////

    // If spliter.
    // <qt-else />
    TagELseIf.Keyword  = L"<qt-else";
    ELseIfTail.Keyword = L"/>";
    TagELseIf.Flag     = Flags::SPLIT;
    TagELseIf.Tail     = &ELseIfTail;
    TagELseIf.SubExprs.Add(&TagQuote);
    /////////////////////////////////

    // If's head.
    iFsHead.Keyword  = L"<qt-if";
    iFsHeadT.Keyword = L">";
    iFsHead.Flag     = Flags::ONCE;
    iFsHead.Tail     = &iFsHeadT;
    // Nest to prevent matching ">" bigger sign inside if statement.
    iFsHead.NestExprs.Add(&TagQuote);
    /////////////////////////////////

    // If evaluation.
    TagIf.ParseCB = &(Template::RenderIF);
    TagIf.Pocket  = &(this->Pocket);
    // <qt-if case="{case}">html code</qt-if>
    TagIf.Keyword  = L"<qt-if";
    IfTail.Keyword = L"</qt-if>";
    TagIf.Flag     = Flags::NESTSPLIT;
    TagIf.Tail     = &IfTail;
    TagIf.SubExprs.Add(&iFsHead);
    TagIf.NestExprs.Add(&TagIf).Add(&TagELseIf);
    Pocket.Tags.Add(&TagIf);
    /////////////////////////////////

    // Loop's head.
    LoopsHead.Keyword  = L"<qt-loop";
    LoopsHeadT.Keyword = L">";
    LoopsHead.Flag     = Flags::ONCE;
    LoopsHead.Tail     = &LoopsHeadT;
    LoopsHead.SubExprs.Add(&TagQuote);
    /////////////////////////////////

    // Loop evaluation.
    TagLoop.ParseCB = &(Template::RenderLoop);
    TagLoop.Pocket  = &(this->Pocket);
    // <qt-loop set="abc2" var="loopId">
    //     <span>loopId): -{qtv-abc2[loopId]}</span>
    // </qt-loop>
    TagLoop.Keyword  = L"<qt-loop";
    LoopTail.Keyword = L"</qt-loop>";
    TagLoop.Tail     = &LoopTail;
    TagLoop.SubExprs.Add(&LoopsHead);
    TagLoop.NestExprs.Add(&TagLoop); // nested by itself
    Pocket.Tags.Add(&TagLoop);
    /////////////////////////////////
}

String Qentem::Template::Render(const String &content, QArray *data) noexcept {
    if (data != nullptr) {
        this->Pocket.Data = data;
    }

    return Engine::Parse(content, Engine::Search(content, this->Pocket.Tags));
}

// <qt-if case="{case}">html code</qt-if>
// <qt-if case="{case}">html code1 <qt-else /> html code2</qt-if>
// <qt-if case="{case1}">html code1 <qt-elseif case={case2} /> html code2</qt-if>
// <qt-if case="{case}">html code <qt-if case="{case2}" />additional html code</qt-if></qt-if>
bool Qentem::Template::EvaluateIF(const String &block, const Match &if_case) noexcept {
    size_t offset = (if_case.Offset + if_case.OLength);
    size_t length = (if_case.Length - (if_case.OLength + if_case.CLength));
    auto   pocket = static_cast<PocketT *>(if_case.Expr->Pocket);

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
            size_t offset = (sm->Offset + sm->Length);
            size_t length = (item.Length - (sm->Length + item.CLength));

            if (item.NestMatch.Size != 0) {
                nm = &(item.NestMatch[0]);
                if (nm->NestMatch.Size > 0) {
                    // if_else (splitted content)
                    for (size_t i = 0; i < nm->NestMatch.Size; i++) {
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

// e.g. {qtv-var_name}
// e.g. {qtv-var_name[id]}
// Nest: {qtv-var_{qtv-var2_{qtv-var3_id}}}
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

// {qt-iif case="3 == 3" true="Yes" false="No"}
// {qt-iif case="{qtv-var_five} == 5" true="5" false="no"}
// {qt-iif case="{qtv-var_five} == 5" true="{qtv-var_five} is equal to 5" false="no"}
// {qt-iif case="3 == 3" true="Yes" false="No"}
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
    for (size_t i = 0; i < items.Size; i++) {
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

// <qt-loop set="abc2" var="loopId">
//     <span>loopId): -{qtv-abc2[loopId]}</span>
// </qt-loop>
String Qentem::Template::RenderLoop(const String &block, const Match &item) noexcept {
    // To match: <qt-loop (set="abc2" var="loopId")>
    if ((item.SubMatch.Size != 0) && (item.SubMatch[0].SubMatch.Size != 0)) {
        Match *      m;
        String       name   = L"";
        String       var_id = L"";
        const Match *sm     = &(item.SubMatch[0]);

        // When bubbling
        // adj_offset = ((m->Offset + m->OLength) - (item.Offset + item.OLength));
        // When not
        // adj_offset = (m->Offset + m->OLength);

        // set="(array_name)" var="(var_id)"
        for (size_t i = 0; i < sm->SubMatch.Size; i++) {
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
            size_t         offset  = sm->Offset + sm->Length;
            size_t         length  = (item.Length - (sm->Length + item.CLength));
            String         content = Template::DoLoop(String::Part(block, offset, length), name, var_id, pocket->Data);

            // When bubbling
            // const size_t   offset  = (sm->Length - item.OLength);
            // const size_t   length  = ((item.Length - offset) - (item.OLength + item.CLength));
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
                                QArray *storage) noexcept {
    String rendered = L"";
    String reminder = L"";
    String id       = L"";

    size_t index = 0;
    VType  type;
    String key = name;

    while (true) {
        if (!(Qentem::QArray::DecodeKey(key, id, reminder)) || !(storage->GetIndex(key, index)) ||
            ((type = storage->Types[index]) == VType::NullT)) {
            return rendered;
        }

        if ((id.Length == 0) || (type != VType::QArrayT)) {
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
        for (size_t i = 0; i < st->Size; i++) {
            ser[0]->Replace = String::ToString(static_cast<double>(i));
            rendered += Engine::Parse(content, items);
        }
    } else if (type == VType::QArrayT) {
        const Array<String> *va = &(storage->VArray[storage->ExactID[index]].Keys);
        for (size_t i = 0; i < va->Size; i++) {
            ser[0]->Replace = (*va)[i];
            rendered += Engine::Parse(content, items);
        }
    }

    return rendered;
}
