
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

#include "Engine.hpp"
#include "Extintion/ALU.hpp"
#include "Extintion/Document.hpp"

namespace Qentem {

struct Template {
    struct PocketT {
        Document *      Data = nullptr;
        Qentem::ALU _Alu = Qentem::ALU();
        Expressions Tags;
        Expressions TagsQuotes;
        Expressions TagsVars;
    };

    PocketT Pocket;

    Expression TagVar;
    Expression VarNext;

    Expression TagIif;
    Expression IifNext;

    Expression TagQuote;
    Expression QuoteNext;

    Expression LoopsHead;
    Expression LoopsHead_T;

    Expression TagLoop;
    Expression LoopNext;

    Expression iFsHead;
    Expression iFsHead_T;

    Expression TagIf;
    Expression IfNext;

    Expression TagELseIf;
    Expression ELseIfNext;

    explicit Template() noexcept;

    String Render(const String &, Document *data = nullptr) noexcept;

    static String RenderVar(const String &, const Match &) noexcept;
    static bool   EvaluateIF(const String &, const Match &) noexcept;
    static String RenderIF(const String &, const Match &) noexcept;
    static String RenderIIF(const String &, const Match &) noexcept;
    static String RenderLoop(const String &, const Match &) noexcept;
    static String Repeat(const String &, const String &, const String &, Document *) noexcept;
};
} // namespace Qentem

#endif
