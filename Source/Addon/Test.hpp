
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
    UNumber       Line = 0;
    Expressions   Exprs;
    Expressions   Collect;
    Array<String> Content;
    Array<String> Expected;

    void Move(TestBit &src) noexcept {
        if (this != &src) {
            this->Line = src.Line;
            this->Exprs.Move(src.Exprs);
            this->Collect.Move(src.Collect);
            this->Content.Move(src.Content);
            this->Expected.Move(src.Expected);
        }
    }

    void Copy(const TestBit &src) noexcept {
        if (this != &src) {
            this->Line     = src.Line;
            this->Exprs    = src.Exprs;
            this->Collect  = src.Collect;
            this->Content  = src.Content;
            this->Expected = src.Expected;
        }
    }

    explicit TestBit() = default;

    TestBit(TestBit &&src) noexcept {
        Move(src);
    }

    TestBit(const TestBit &src) noexcept {
        Copy(src);
    }

    TestBit &operator=(TestBit &&src) noexcept {
        Move(src);
        return *this;
    }

    TestBit &operator=(const TestBit &src) noexcept {
        Copy(src);
        return *this;
    }
};

void          CleanBits(Array<TestBit> &) noexcept;
Array<String> Extract(const String &, const Array<Match> &) noexcept;

String         DumbExpressions(const Expressions &, const String &, UNumber = 0, Expression * = nullptr) noexcept;
String         DumbMatches(const String &, const Array<Match> &, const String &, UNumber = 0) noexcept;
Array<TestBit> GetBits() noexcept;

String SubMatchZero(const String &, const Match &) noexcept;
String SubMatchNestMatch(const String &, const Match &) noexcept;
String FlipSplit(const String &, const Match &) noexcept;

} // namespace Test
} // namespace Qentem

#endif
