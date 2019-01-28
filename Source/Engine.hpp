
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

namespace Qentem {
namespace Engine {

using Qentem::Array;
using Qentem::String;
/////////////////////////////////
struct Match;
struct Expression;
/////////////////////////////////
// Expressions def
using Expressions = Array<Expression *>;
// Search Callback
using _SEARCHCB = UNumber(const String &, const Expression &, Match *, UNumber &, UNumber &, UNumber limit);
// Parse Callback
using _PARSECB = String(const String &, const Match &);
/////////////////////////////////
// Expressions flags
struct Flags {
    static const unsigned short NOTHING       = 0;    // ... NAN.
    static const unsigned short COMPACT       = 1;    // Processing only the content inside Keywords Parse().
    static const unsigned short NOPARSE       = 2;    // Match a Keyword but don't process it inside Parse().
    static const unsigned short IGNORE        = 4;    // Match a Keyword but don't process it inside Parse().
    static const unsigned short TRIM          = 8;    // Trim the match before adding it.
    static const unsigned short ONCE          = 16;   // Will stop searching after matching.
    static const unsigned short POP           = 32;   // Search again with NestExprs if the match fails (See ALU.cpp).
    static const unsigned short BUBBLE        = 64;   // Parse nested matches.
    static const unsigned short SPLIT         = 128;  // Split a match at a keyword.
    static const unsigned short SPLITNEST     = 256;  // Split a Nested match.
    static const unsigned short GROUPSPLIT    = 512;  // Puts split matches into NestMatch, for one callback execution.
    static const unsigned short SPLITROOTONLY = 1024; // e.g. IF-ELSE (Template.cpp)
    static const unsigned short DROPEMPTY     = 2048; // Trim the match before adding it.
};
/////////////////////////////////
struct Expression {
    String  Keyword = L""; // What to search for.
    String  Replace;       // A text to replace the match.
    UNumber Flag = 0;

    Expression *Connected = nullptr; // The next part of the match (the next keyword).

    Expressions NestExprs; // Expressions for nesting Search().
    Expressions SubExprs;  // Matches other parts of the match, but do not nest.
    // Expressions SplitExprs; // to split a match like if if-else; if-if-else-else.

    _SEARCHCB *SearchCB = nullptr; // A callback function for custom lookup.
    _PARSECB * ParseCB  = nullptr; // A callback function for custom rendering.

    // Pocket pointer is a var that can be linked to an object to be used in callback functions insted of relining on
    // static data members, which is not good for multi-threading operations. (See Template.cpp)
    void *Pocket = nullptr;
};
/////////////////////////////////
struct Match {
    UNumber Offset  = 0;
    UNumber Length  = 0;
    UNumber OLength = 0; // Length of opening keyword
    UNumber CLength = 0; // Length of closing keyword

    UNumber     Tag  = 0; // To Mark a match when using callback search (for later sorting, See ALU.cpp).
    UNumber     Id   = 0; // for personal use.
    Expression *Expr = nullptr;

    Array<Match> NestMatch; // To hold sub matches inside a match.

    // SubMatch: To hold matches inside a match; for checking before evaluation nest matches.
    // Its content does not get parse; it would be faster to do a sub search insead of calling back Search() from
    // an outside function, when the CPU cache already holds the text.
    Array<Match> SubMatch;
};
/////////////////////////////////
Array<Match> Search(const String &, const Expressions &, UNumber = 0, UNumber = 0, const UNumber = 0,
                    const UNumber = 0) noexcept;
void         Split(const String &, Array<Match> &, UNumber, UNumber) noexcept;
String       Parse(const String &, const Array<Match> &, UNumber = 0, UNumber = 0) noexcept;
/////////////////////////////////
} // namespace Engine
} // namespace Qentem

#endif
