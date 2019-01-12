
/**
 * Qentem Template
 *
 * @brief     Generate dynamic HTML code from a template.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#include "Template.hpp"
#include "ALU.hpp"

Qentem::Template::Template() noexcept {
    Qentem::ALU::Init();
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
String Qentem::Template::ReplaceVar(const String &block, const Match &match) noexcept {
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
    // Ending index is at (offset + length)
    // TODO: Test length
    const Array<Match> items = Engine::Search(block, ((PocketT *)(match.Expr->Pocket))->TagsQuote);
    // auto _cl = block.Part(offset, limit); // For testing only
    if (items.Size() == 0) {
        return L"";
    }

    Match *m;
    String iif_case  = L"";
    String iif_false = L"";
    String iif_true  = L"";

    // case="[statement]" true="[Yes]" false="[No]"
    for (size_t i = 0; i < items.Size(); i++) {
        // With this method, order is not necessary of case=, true=, false=
        m = &(items[i]);
        if (m->Offset > 2) {
            switch (block.Str[(m->Offset - 3)]) {
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

    // ALU::Evaluate(if_case); // Testing only!
    // return if_case;         // Testing only!
    if ((iif_case.Length != 0) && (iif_true.Length != 0) && (ALU::Evaluate(iif_case) != 0)) {
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
    PocketT *pocket = (PocketT *)(match.Expr->Pocket);
    // Nothing is processed inside the if before making sure it will be displayed.
    Array<Match> items = Engine::Search(block, pocket->iFsHead);
    if (items.Size() != 0) {
        // To  replace only  variables inside if statement.
        // After making sure the statement is TRUE!
        const size_t end_if_pos = (items[0].Offset + items[0].Length);
        // String _test = block.Part((items[0].offset + items[0].OLength), end_if_pos);
        String content = Engine::Parse(block, items[0].NestMatch, (items[0].Offset + items[0].OLength), end_if_pos);

        items = Engine::Search(content, pocket->TagsQuote);
        if (items.Size() != 0) {
            content = content.Part((items[0].Offset + items[0].OLength),
                                   (items[0].Length - (items[0].OLength + items[0].CLength)));

            if ((ALU::Evaluate(content) != 0)) {
                // return if_case; // Testing only!
                const size_t if_limit = block.Length - (end_if_pos + match.CLength);

                // Now render content that will be displayed.
                if (match.NestMatch.Size() != 0) {
                    // This will prevent the engine from matching the same keyword over and over, as much as
                    // nested. Do what has been found then process the rest alone when needed.
                    content = Engine::Parse(block, match.NestMatch, end_if_pos, (end_if_pos + if_limit));
                } else {
                    content = block.Part(end_if_pos, if_limit);
                }

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
    PocketT *pocket = (PocketT *)(match.Expr->Pocket);
    // To match: <qt:loop (set="abc2" var="loopId")>
    Array<Match> items = Engine::Search(block, pocket->loopsHead);
    if ((items.Size() != 0) && (items[0].NestMatch.Size() != 0)) {
        Match *m;
        String name   = L"";
        String var_id = L"";
        // set="(array_name)" var="(var_id)"
        for (size_t i = 0; i < items[0].NestMatch.Size(); i++) {
            // With this method, order is not necessary of case=, true=, false=
            m = &(items[0].NestMatch[i]);
            if (m->Offset > 1) {
                switch (block.Str[(m->Offset - 1)]) {
                    case 't': // se[t]
                        name = block.Part((m->Offset + m->OLength), (m->Length - (m->CLength + m->OLength)));
                        break;
                    case 'r': // va[r]
                        var_id = block.Part((m->Offset + m->OLength), (m->Length - (m->CLength + m->OLength)));
                        break;
                }
            }
        }

        // This mathod is to do the opposite of bubbling.
        if ((name.Length != 0) && (var_id.Length != 0)) {
            const size_t end_loop_pos = (items[0].Offset + items[0].Length);
            const String content      = Template::DoLoop(
                block.Part(end_loop_pos, block.Length - (end_loop_pos + match.CLength)), name, var_id, pocket->Data);

            if (content.Length != 0) {
                return Engine::Parse(content, Engine::Search(content, pocket->Tags));
            }
        }
    }

    return L"";
}

String Qentem::Template::DoLoop(const String &content, const String &name, const String &var_name,
                                QArray *v_arr) noexcept {
    String rendered = L"";
    String reminder = L"";
    String id       = L"";

    size_t index = 0;
    VType  type;
    String key = name;

    while (true) {
        if (!(v_arr->DecodeKey(key, id, reminder)) || !(v_arr->GetIndex(key, index)) ||
            ((type = v_arr->Types[index]) == VType::NullT)) {
            return rendered;
        }

        if ((id.Length == 0) || (type != VType::QArrayT)) {
            break;
        }

        key = id + reminder;

        v_arr = &(v_arr->VArray[v_arr->RealID[index]]);
    }

    // TODO: Replace new
    static Expressions ser = Expressions().Add(new Expression());
    ser[0]->Keyword        = var_name;

    const Array<Match> items = Engine::Search(content, ser);
    // Feature: Use StringStream!!!
    if (type == VType::ArrayT) {
        const Array<String> *st = &(v_arr->Arrays[v_arr->RealID[index]]);
        for (size_t i = 0; i < st->Size(); i++) {
            ser[0]->Replace = String::ToString((float)i);
            rendered += Engine::Parse(content, items);
        }
    } else if (type == VType::QArrayT) {
        const Array<String> *va = &(v_arr->VArray[v_arr->RealID[index]].Keys);
        for (size_t i = 0; i < va->Size(); i++) {
            ser[0]->Replace = (*va)[i];
            rendered += Engine::Parse(content, items);
        }
    }

    return rendered;
}
