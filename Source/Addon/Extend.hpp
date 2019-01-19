
/**
 * Qentem Engine Extended functions
 *
 * @brief     For implenting other patent.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#ifndef QENTEM_EXTEND_H
#define QENTEM_EXTEND_H

#include "Engine.hpp"

namespace Qentem {
namespace Extend {

using Qentem::Engine::Expression;

size_t RegexOR(const String &, const Expression &, size_t &, size_t &) noexcept;

} // namespace Extend
} // namespace Qentem

#endif
