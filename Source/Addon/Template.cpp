
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
#include "StringStream.hpp"

using Qentem::String;
using Qentem::Engine::Flags;

Qentem::Template::Template() noexcept {

    // Variables evaluation.
    VarNext.ParseCB = &(Template::RenderVar);
    VarNext.Pocket  = &(this->Pocket);
    // {v-var_name}
    TagVar.Keyword   = L"{v-";
    VarNext.Keyword  = L"}";
    TagVar.Connected = &VarNext;
    VarNext.Flag     = Flags::TRIM;
    Pocket.Tags.Add(&TagVar);
    Pocket.TagsVars.Add(&TagVar);
    /////////////////////////////////

    // Inline if evaluation.
    IifNext.ParseCB = &(Template::RenderIIF);
    IifNext.Pocket  = &(this->Pocket);
    //{iif case="3 == 3" true="Yes" false="No"}
    TagIif.Keyword   = L"{iif";
    IifNext.Keyword  = L"}";
    TagIif.Connected = &IifNext;
    IifNext.Flag     = Flags::BUBBLE;
    IifNext.NestExprs.Add(&TagVar); // nested by TagVars'
    Pocket.Tags.Add(&TagIif);
    /////////////////////////////////

    // TagsQuotes.
    TagQuote.Keyword   = L"\"";
    QuoteNext.Keyword  = L"\"";
    QuoteNext.Flag     = Flags::TRIM;
    TagQuote.Connected = &QuoteNext;
    QuoteNext.Pocket   = &(this->Pocket);
    Pocket.TagsQuotes.Add(&TagQuote);
    /////////////////////////////////

    // If spliter.
    // <else />
    TagELseIf.Keyword   = L"<else";
    ELseIfNext.Keyword  = L"/>";
    ELseIfNext.Flag     = Flags::SPLIT;
    TagELseIf.Connected = &ELseIfNext;
    ELseIfNext.SubExprs.Add(&TagQuote);
    /////////////////////////////////

    // If's head.
    iFsHead.Keyword   = L"<if";
    iFsHead_T.Keyword = L">";
    iFsHead_T.Flag    = Flags::ONCE;
    iFsHead.Connected = &iFsHead_T;
    // Nest to prevent matching ">" bigger sign inside if statement.
    iFsHead_T.NestExprs.Add(&TagQuote);
    /////////////////////////////////

    // If evaluation.
    IfNext.ParseCB = &(Template::RenderIF);
    IfNext.Pocket  = &(this->Pocket);
    // <if case="{case}">html code</if>
    TagIf.Keyword   = L"<if"; // TODO: Add  shalow if for nesting
    IfNext.Keyword  = L"</if>";
    TagIf.Connected = &IfNext;
    IfNext.Flag     = Flags::SPLITROOTONLY;
    IfNext.SubExprs.Add(&iFsHead);
    IfNext.NestExprs.Add(&TagIf).Add(&TagELseIf);
    Pocket.Tags.Add(&TagIf);
    /////////////////////////////////

    // Loop's head.
    LoopsHead.Keyword   = L"<loop";
    LoopsHead_T.Keyword = L">";
    LoopsHead.Connected = &LoopsHead_T;
    LoopsHead_T.Flag    = Flags::ONCE;
    LoopsHead_T.SubExprs.Add(&TagQuote);
    /////////////////////////////////

    // Loop evaluation.
    LoopNext.ParseCB = &(Template::RenderLoop);
    LoopNext.Pocket  = &(this->Pocket);
    // <loop set="abc2" var="loopId">
    //     <span>loopId): -{v-abc2[loopId]}</span>
    // </loop>
    TagLoop.Keyword   = L"<loop";
    LoopNext.Keyword  = L"</loop>";
    TagLoop.Connected = &LoopNext;
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
    PocketT *_pocket = static_cast<PocketT *>(item.Expr->Pocket);
    String   value;

    if (_pocket->Data->GetString(value, block, (item.Offset + item.OLength),
                                 (item.Length - (item.CLength + item.OLength)))) {
        return value;
    }

    return String::Part(block, (item.Offset + item.OLength), (item.Length - (item.CLength + item.OLength)));
    // return String::Part(block, item.OLength, (block.Length - (item.OLength + item.CLength))); // When Bubble
}

// <if case="{case}">html code</if>
// <if case="{case}">html code1 <else /> html code2</if>
// <if case="{case1}">html code1 <elseif case={case2} /> html code2</if>
// <if case="{case}">html code <if case="{case2}" />additional html code</if></if>
bool Qentem::Template::EvaluateIF(const String &block, const Match &if_case) noexcept {
    UNumber  offset = (if_case.Offset + if_case.OLength);
    UNumber  length = (if_case.Length - (if_case.OLength + if_case.CLength));
    PocketT *pocket = static_cast<PocketT *>(if_case.Expr->Pocket);

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

            // // if_else (splitted content)
            if (item.NestMatch.Size != 0) {
                nm = &(item.NestMatch[0]);
                if ((Flags::SPLIT & nm->Expr->Flag) != 0) {
                    if (_true) {
                        length = (nm->Length - (offset - nm->Offset));
                    } else {
                        for (UNumber i = 1; i < item.NestMatch.Size; i++) {
                            if ((item.NestMatch[i].SubMatch.Size == 0) ||
                                Template::EvaluateIF(block, item.NestMatch[i].SubMatch[0])) {
                                // inner content of the next part.
                                offset = item.NestMatch[i].Offset;
                                length = item.NestMatch[i].Length;
                                _true  = true;
                                break;
                            }
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
    String iif_case;
    String iif_false;
    String iif_true;

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
        String       name;
        String       var_id;
        const Match *sm = &(item.SubMatch[0]);

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
            String         content = Template::Repeat(String::Part(block, offset, length), name, var_id, pocket->Data);

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

String Qentem::Template::Repeat(const String &content, const String &name, const String &var_name,
                                Tree *storage) noexcept {
    Hash *      hash;
    const Tree *_storage = storage->GetInfo(&hash, name, 0, name.Length);

    if (_storage == nullptr) {
        return L"";
    }

    StringStream rendered;
    Expressions  ser;
    Expression   ex;
    ser.Add(&ex);

    ex.Keyword = var_name;

    const Array<Match> items = Engine::Search(content, ser);

    // TODO: restore with a fix
    // if ((hash->Type == VType::OStringsT) || (hash->Type == VType::ONumbersT)) {
    //     const Array<String> *st = &(_storage->OStrings[hash->ExactID]);
    //     if (st != nullptr) {
    //         for (UNumber i = 0; i < st->Size; i++) {
    //             ser[0]->Replace = String::FromNumber(i);
    //             rendered += Engine::Parse(content, items);
    //         }
    //     }
    // } else if (hash->Type == VType::BranchT) {
    //     const Tree *ci = &(_storage->Branches[hash->ExactID]);
    //     if (ci != nullptr) {
    //         for (UNumber i = 0; i < ci->Table.Size; i++) {
    //             ser[0]->Replace = ci->Table[i].Key;
    //             rendered += Engine::Parse(content, items);
    //         }
    //     }
    // }

    return rendered.Eject();
}
