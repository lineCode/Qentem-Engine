
/**
 * Qentem Engine
 *
 * @brief     General purpose syntax parser.
 *
 * @author    Hani Ammar <hani.code@outlook.com>
 * @copyright 2019 Hani Ammar
 * @license   https://opensource.org/licenses/MIT
 */

#ifndef QENTEM_ENGINE_H
#define QENTEM_ENGINE_H

#include "String.hpp"
#include "Array.hpp"

using Qentem::Array;
using Qentem::String;

namespace Qentem {
namespace Engine {
/////////////////////////////////
struct Match;
struct Expression;
/////////////////////////////////
// Expressions def
typedef Array<Expression *> Expressions;
// Search Callback
typedef size_t (*_SEARCHCB)(const String &content, size_t &offset, size_t &limit);
// Parse Callback
typedef String (*_PARSECB)(const String &block, const Match &match);
/////////////////////////////////
// Expressions flags
struct Flags {
    static const unsigned short NOTHING = 0;  // ... What it says.
    static const unsigned short COMPACT = 1;  // Processing the content without Keyword(s).
    static const unsigned short IGNORE  = 2;  // Match a Keyword but don't process it inside Parse().
    static const unsigned short BUBBLE  = 4;  // Parse nested matches.
    static const unsigned short SPLIT   = 8;  // Split a match at a point.
    static const unsigned short POP     = 16; // Search again with NestExprs if the match fails (See ALU.cpp).
};
/////////////////////////////////
struct Expression {
    String Keyword = L"";
    String Replace = L""; // A text to replace the match.
    size_t Flag    = 0;

    Expression *Tail = nullptr; // The ending part of the match (the second keyword).

    Expressions NestExprs; // Expressions for nesting Search().
    Expressions SubExprs;  // Matches other parts of the match, but do not nest.
    // Expressions SplitExprs; // to split a match like if if-else; if-if-else-else.

    _SEARCHCB SearchCB = nullptr; // A callback function for custom lookup.
    _PARSECB  ParseCB  = nullptr; // A callback function for custom rendering.

    // Pocket pointer is a var that can be linked to an object to be used in callback functions insted of relining on
    // static data members, which is not good for multi-threading operations. (See Template.cpp)
    void *Pocket = nullptr;
};
/////////////////////////////////
struct Match {
    size_t Offset  = 0;
    size_t Length  = 0;
    size_t OLength = 0; // Length of opening keyword
    size_t CLength = 0; // Length of closing keyword

    size_t Status = 0; // 1: OverDrive, For matching beyand the length of a match.
    size_t Tag    = 0;

    Expression *Expr = nullptr;

    Array<Match> NestMatch; // To hold sub matches inside a match.

    // SubMatch: To hold matches inside a match; for checking before evaluation nest matches.
    // Its content does not get parse; it would be faster to do a sub search insead of calling back Search() from
    // an outside function
    Array<Match> SubMatch;
};
/////////////////////////////////
Array<Match> Search(const String &content, const Expressions &exprs, size_t from = 0, size_t to = 0,
                    size_t max = 0) noexcept;
String       Parse(const String &content, const Array<Match> &items, size_t offset = 0, size_t length = 0) noexcept;
void         Split(const String &content, Match &item, Array<Match> &items, size_t from, size_t to) noexcept;
/////////////////////////////////
} // namespace Engine
} // namespace Qentem

#endif
