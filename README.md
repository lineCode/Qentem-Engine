# Qentem Engine (v0.9.8.0)
Qentem is a stand-alone library built to provide a simple way to match existing syntaxes or new ones. It also provides call-backs for post or per-parsing. Built for templates, then rewritten for general-propose syntax matching.

## Introduction:
Qentem is a way of proforming nest-searching, and since regular expressions is limited to a basic matching, Qentem Engine was created to fill a gab and provide an easy way to deal with complex syntaxes, as efficient as possible.

## The Engine:
The engine is build in modern C++ and uses wide character (wchar_t). It does not depend on any external library. This makes it easy to be ported to other programing languages and be cross platform by default.

## Usage:
It can be used to render complex templates that contains nested loop, nested if-else, inline if, liner algebra math (+ * / - ^ %), logic (&& ||), and/or something simple; like text replacing/splitting. Most of that uses the engines search & parse functions, and booth of them support custom callbacks to extend their existing functions.

## Built-in:
The library at the moment has String class (with number conversion), Array<Type>, String Stream, Document(Tree/map/hash-table), Template generator, Arithmetic and Logic Unit, and JSON parser (with C style comments).

## Requirements:
* C++ 11 compiler.
* (Optional) Emscripten: to compile into WebAssembly; to be used inside a web browser or with an independent JavaScript engine. This alow a web server to offload the rendering of an html/text to the client.

## The Project:
The project started as a template generator for JavaScript and PHP, and because of the performance impact of managed languages, it became a necessary to move it to a native one.  JSON was not planed for but it was implemented for the convenience of it, and because Document was implemented already. The template engine was the engine itself before it was rewritten for C++ and merged into one function (Search) to make it a general syntax parser.

## Template Demo:
JavaScript and WebAssembly:

## Examples:
### Document and JSON:
```cpp
#include <Extension/Document.hpp>
using Qentem::Array;
using Qentem::Document;
using Qentem::String;
using Qentem::UNumber;

Document numbers = L"[1,1,2,3,4]"; // New document from a JSON text

numbers += L"[5,6]";                      // Extanding JSON
numbers += Array<double>().Add(7).Add(8); // Extanding win an ordered array

Document doc;             // New document
doc["numbers"] = numbers; // assigning an ordered numbers
doc["numbers"] += 9;
doc["numbers"][0] = 0;

doc += L"{\"strings\": { }}"; // Expanding the document with an unordered array
doc["strings"][L"a"] = L"A";
doc["strings"][L"b"] = L"O";
doc["strings"][1]    = L"B";

// Importing JSON with comments
doc["strings"] += Document::FromJSON(L"{\"c\": \"C\", \"d\": \"D\"  /* \"e\": \"E\" */}", true);

doc += L"{\"strings2\": [\"E\", \"F\"]}"; // Ordered strings
doc["strings2"] += L"[\"G\"]";
doc["strings2"] += Array<String>().Add(L"H").Add(L"I");

String JSON = doc.ToJSON(); // Exporting the document
```
#### Output:
```json
{
    "numbers": [
        0,
        1,
        2,
        3,
        4,
        5,
        6,
        7,
        8,
        9
    ],
    "strings": {
        "a": "A",
        "b": "B",
        "c": "C",
        "d": "D"
    },
    "strings2": [
        "E",
        "F",
        "G",
        "H",
        "I"
    ]
}
```

### Template (HTML friendly):
```cpp
#include <Extension/Template.hpp>
// Based on the Document example
String content = L"<loop set=\"strings\" var=\"s_id\">[s_id]: {v:strings[s_id]}\n</loop>--\n";
content += L"<loop set=\"strings2\" var=\"s_id\">[s_id]: {v:strings2[s_id]}\n</loop>";
String rendered = Template::Render(content, &doc);
```
#### Output:
```txt
[a]: A
[b]: B
[c]: C
[d]: D
--
[0]: E
[1]: F
[2]: G
[3]: H
[4]: I
```

### Note:
The complete example is located @ [Example/Example1.cpp](https://github.com/HaniAmmar/Qentem-Engine/blob/master/Example/Example1.cpp). For more about template syntax, see [Tests/temp.qtml](https://github.com/HaniAmmar/Qentem-Engine/blob/master/Tests/temp.qtml).


**To compile:**
```txt
CXX -I ./Source ./Example/Example1.cpp -o ./Tests/QTest.bin
```
```txt
where "./" being the path to Qentem Engine, and "CXX" is either clang or gcc.
for Windows, see .sln files.
```

### Engine:
The usage of the engine can be found @ [Tests/Test.cpp](https://github.com/HaniAmmar/Qentem-Engine/blob/master/Tests/Test.cpp).
