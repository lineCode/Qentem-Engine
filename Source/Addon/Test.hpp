
/**
 * Qentem Test
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

namespace Qentem {
namespace Test {

using Qentem::Engine::Expression;
using Qentem::Engine::Expressions;
using Qentem::Engine::Match;

struct TestBit {
    Expressions   Exprs;
    Expressions   Exprslvl0;
    Array<String> Content;
    Array<String> Expected;
    size_t        Line = 0;
};

void          CleanBits(Array<TestBit> &) noexcept;
Array<String> Extract(const String &, const Array<Match> &) noexcept;

String         DumbExpressions(const Expressions &, const String &, size_t = 0, Expression * = nullptr) noexcept;
String         DumbMatches(const String &, const Array<Match> &, const String &, size_t = 0) noexcept;
Array<TestBit> GetBits() noexcept;
String         SubMatchZero(const String &, const Match &) noexcept;
String         SubMatchNestMatch(const String &, const Match &) noexcept;
String         FlipSplit(const String &, const Match &) noexcept;

} // namespace Test
} // namespace Qentem

#endif
