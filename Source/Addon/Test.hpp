
/**
 * Qentem Engine
 *
 * @brief     For testing Qentem Engine
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

///////////////////////////
#ifndef TESTENGINE_H
#define TESTENGINE_H

#include "Engine.hpp"

using Qentem::Engine::Expression;
using Qentem::Engine::Expressions;
using Qentem::Engine::Flags;
using Qentem::Engine::Match;

namespace Qentem {
namespace Test {

struct TestBit {
    Expressions   Exprs;
    Expressions   Exprslvl0;
    Array<String> Content;
    Array<String> Expected;
    size_t        Line;
};

void           CleanBits(Array<TestBit> &bits) noexcept;
Array<TestBit> GetBits() noexcept;
String         SubMatchZero(const String &block, const Match &match) noexcept;
String         SubMatchNestMatch(const String &block, const Match &match) noexcept;

} // namespace Test
} // namespace Qentem

#endif
