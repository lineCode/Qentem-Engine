
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
        Expressions Tags;
        Expressions TagsQuote;
        Expressions iFsHead;
        Expressions loopsHead;
    } Pocket;

    explicit Template() noexcept;
    String Render(const String &content, QArray *data = nullptr) noexcept;

    static String ReplaceVar(const String &block, const Match &match) noexcept;
    static String RenderIIF(const String &block, const Match &match) noexcept;
    static String RenderIF(const String &block, const Match &match) noexcept;
    static String RenderLoop(const String &block, const Match &match) noexcept;
    static String DoLoop(const String &content, const String &name, const String &id, QArray *v_arr) noexcept;
};
} // namespace Qentem

#endif
