
/**
 * Qentem Engine Regex extintion
 *
 * @brief     For implenting other patent.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#ifndef QENTEM_QRegex_H
#define QENTEM_QRegex_H

#include "Engine.hpp"

namespace Qentem {
namespace QRegex {

using Qentem::Engine::Expression;
using Qentem::Engine::Match;
using Qentem::Engine::String;

UNumber OR(const String &, const Expression &, Match *, UNumber &, UNumber &, UNumber) noexcept;
String  Replace(const String &content, const String &, const String &) noexcept;
} // namespace QRegex
} // namespace Qentem

#endif
