
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
#include "Addon/ALU.hpp"
#include "Addon/QArray.hpp"

namespace Qentem {

struct Template {
    struct PocketT {
        QArray *    Data = nullptr;
        Qentem::ALU _Alu = Qentem::ALU();
        Expressions Tags;
        Expressions TagsQuotes;
        Expressions TagsVars;
    };

    PocketT Pocket;

    Expression TagVar;
    Expression VarTail;

    Expression TagIif;
    Expression IifTail;

    Expression TagQuote;
    Expression QuoteTail;

    Expression LoopsHead;
    Expression LoopsHeadT;

    Expression TagLoop;
    Expression LoopTail;

    Expression iFsHead;
    Expression iFsHeadT;

    Expression TagIf;
    Expression IfTail;

    Expression TagELseIf;
    Expression ELseIfTail;

    explicit Template() noexcept;
    String Render(const String &, QArray *data = nullptr) noexcept;

    static String RenderVar(const String &, const Match &) noexcept;
    static bool   EvaluateIF(const String &, const Match &) noexcept;
    static String RenderIF(const String &, const Match &) noexcept;

    static String RenderIIF(const String &, const Match &) noexcept;
    static String RenderLoop(const String &, const Match &) noexcept;
    static String DoLoop(const String &, const String &, const String &, QArray *) noexcept;
};
} // namespace Qentem

#endif
