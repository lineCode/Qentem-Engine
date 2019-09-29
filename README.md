# Qentem Engine (v0.9.8.8)

## Introduction:
Qentem Engine is an independent library that uses a fast algorithm for nest-searching and nest-matching. It can be used to match existing syntaxes or new ones, and with call-backs for post and per-parsing, It's posable to match almost any complex syntax. It's highly efficient and has small footprint on memory, and built using modern C++. It can be used to render complex templates that contains nested loop, nested if-else, inline if, math (+ * / - ^ %), logic (&& ||), and/or something simple; like text replacing/splitting. Also, it can be used for JSON, XML ,HTML... etc.

## Built-in:
The library - at the moment - has String class (with number conversion), Array<Type>, String Stream, Document (Tree/Map/Hash-Table), Template generator, Arithmetic and Logic Unit, and JSON parser (with C style comments). Also, XML/HTML parser.

## Tests:
This library includes over 350 tests; to insure that it's stable and functional. to compile the tests, the fowlling command can be  used:
```txt
CXX -I ./Source ./Tests/Test.cpp -o ./Tests/QTest.bin
```
After That, ./Tests/QTest.bin can be run in a terminal.

## Requirements:
* C++ 11 compiler.
* (Optional) Emscripten: to compile into WebAssembly; to be used inside a web browser or with an independent JavaScript engine. This allows a web server to offload the rendering of an html/text to its clients. Also, I does intger calculations.

## Preset editor:
VS Code configrations are included for compiling and running tests.

## Template Demo:
HTML, JavaScript and WebAssembly: [JQen.zip](https://github.com/HaniAmmar/Qentem-Engine/files/3366315/JQen.zip)

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
doc["strings"][1]    = L"B"; // Override

// Importing JSON with comments
doc["strings"] += Document::FromJSON(L"{\"c\": \"C\", \"d\": \"D\"  /* \"e\": \"E\" */}", true);

doc += L"{\"strings2\": [\"E\", \"F\"]}"; // Ordered strings
doc["strings2"] += L"G";
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
String content = L"<loop set=\"strings\" value=\"s_value\" key=\"s_key\">s_key: s_value\n</loop>--\n";
content += L"<loop set=\"strings2\" value=\"s_value\" key=\"s_key\">s_key: s_value\n</loop>";
String rendered = Template::Render(content, &doc);
```
#### Output:
```txt
a: A
b: B
c: C
d: D
--
0: E
1: F
2: G
3: H
4: I
```

### Note:
The complete example is located @ [Example/Example1.cpp](https://github.com/HaniAmmar/Qentem-Engine/blob/master/Example/Example1.cpp). For more about template syntax, see [Tests/temp.qtml](https://github.com/HaniAmmar/Qentem-Engine/blob/master/Tests/temp.qtml).


### Compiling:
For the example:
```txt
CXX -I ./Source ./Example/Example1.cpp -o ./Tests/QTest.bin
```

For the template library:
```txt
CXX -O3 -std=c++11 -shared -fPIC -I ./Source ./Example/QLib.cpp -o ./Build/QLib.so
```

```txt
where "./" being the path to Qentem Engine, and "CXX" is either clang or gcc.
-For Windows, see .sln files.
```

### Engine:
More examples of the engine usage can be found @ [Tests/Test.hpp](https://github.com/HaniAmmar/Qentem-Engine/blob/master/Tests/Test.hpp) and [Tests/Test.cpp](https://github.com/HaniAmmar/Qentem-Engine/blob/master/Tests/Test.cpp).
