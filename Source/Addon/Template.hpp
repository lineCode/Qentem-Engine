
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
#include "ALU.hpp"
#include "QArray.hpp"

using Qentem::QArray;
using Qentem::Engine::Expression;
using Qentem::Engine::Expressions;
using Qentem::Engine::Flags;
using Qentem::Engine::Match;

namespace Qentem {
struct Template {
    struct PocketT {
        QArray *    Data = nullptr;
        Qentem::ALU _Alu = Qentem::ALU();
        Expressions Tags;
        Expressions TagsQuotes;
        Expressions TagsVars;
    } Pocket;

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

    explicit Template() noexcept;
    String Render(const String &content, QArray *data = nullptr) noexcept;

    static String RenderVar(const String &block, const Match &match) noexcept;
    static String RenderIIF(const String &block, const Match &match) noexcept;
    static String RenderIF(const String &block, const Match &match) noexcept;
    static String RenderLoop(const String &block, const Match &match) noexcept;
    static String DoLoop(const String &content, const String &name, const String &id, QArray *storage) noexcept;
};
} // namespace Qentem

#endif
