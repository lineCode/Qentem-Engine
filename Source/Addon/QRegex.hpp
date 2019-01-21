
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

UNumber OR(const String &, const Expression &, UNumber &, UNumber &) noexcept;

} // namespace QRegex
} // namespace Qentem

#endif
