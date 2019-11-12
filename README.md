# Qentem Engine (v1.3.4)

## Introduction:
Qentem Engine is an independent library that uses a fast algorithm for nest-searching/nest-matching. It can be used to match existing syntaxes or new ones, and with call-backs for post and per-parsing, It's posable to match almost any complex syntax. It is very efficient and has a small footprint on memory, and it's built using modern C++. It can be used to render complex templates that contains nested loop, nested if-else, inline if, math (+ * / - ^ %), logic (&& ||), and/or something simple: like replacing a text or splitting it. Also, it is capable of doing JSON, XML/HTML.

## Built-in:
The library - at the moment - has String class (with number conversion), Array, String Stream, Document (Tree/Map/HashTable), Template generator (HTML friendly syntax), Arithmetic & Logic Unit, and JSON parser (with C style comments). Also, XML/HTML parser.

## Requirements:
* C++ 11 compiler.
* (Optional) Emscripten: to compile into WebAssembly; to be used inside a web browser or with an independent JavaScript engine. This allows a web server to offload its rendering of templates to its clients. Also, I does integer calculations.

## Tests:
This library includes over 350 tests; to insure that it's stable and functional. to compile the tests:
```txt
c++ -I ./Source ./Tests/Test.cpp -o ./Tests/QTest.bin
```

## VS Code configrations:
VS Code configrations are included for compiling and running tests.

## Template Demo:
HTML, JavaScript and WebAssembly: [JQen.zip](https://github.com/HaniAmmar/Qentem-Engine/releases/download/v1.3.3/JQen.zip)

## Examples:
### Document and JSON:
```cpp
#include <Extension/Document.hpp>
using Qentem::Array;
using Qentem::Document;
using Qentem::String;

Document numbers = L"[null,1,2,3,4]"; // New document from JSON text

numbers += Document::FromJSON(L"[5,6]");  // Extanding JSON
numbers += Array<double>().Add(7).Add(8); // Extanding win ordered array

Document doc;              // New document
doc[L"numbers"] = numbers; // assigning ordered numbers
doc[L"numbers"] += 9;      // Add
doc[L"numbers"][0] = 0;    // Override

doc += Document::FromJSON(L"{\"strings\": { }}"); // Expanding document with unordered array

doc[L"strings"][L"a"] = nullptr; // null
doc[L"strings"][L"a"] = L"A";    // Override
doc[L"strings"][L"b"] = true;    // Add & Set
doc[L"strings"][L"b"] = false;   // Override
doc[L"strings"][1]    = L"B";    // Override

// Importing JSON with comments
doc[L"strings"] += Document::FromJSON(L"{\"c\": \"C\", \"d\": \"D\"  /* \"e\": \"E\" */}", true);

doc += Document::FromJSON(L"{\"strings2\": [\"E\", \"F\"]}"); // Ordered strings
doc[L"strings2"] += L"G";
doc[L"strings2"] += Array<String>().Add(L"H").Add(L"I");

String JSON = doc.ToJSON(); // Exporting document
std::wcout << L"JSON:\n" << JSON.Str << L"\n\n";
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

### Template:
```cpp
#include <Extension/Template.hpp>
using Qentem::String;
// Based on the Document example
String content = L"<loop set=\"strings\" value=\"s_value\" key=\"s_key\">s_key: s_value\n</loop>--\n";
content += L"<loop set=\"strings2\" value=\"s_value\" key=\"s_key\">s_key: s_value\n</loop>";
String rendered = Template::Render(content, &doc);
std::wcout << L"Template:\n" << rendered.Str << L'\n';
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
The complete example is located @ [Example/Example1.cpp](https://github.com/HaniAmmar/Qentem-Engine/blob/master/Example/Example1.cpp). For more about template syntax, see [Tests/test.qtml](https://github.com/HaniAmmar/Qentem-Engine/blob/master/Tests/test.qtml).


### Compiling:
The example:
```txt
c++ -I ./Source ./Example/Example1.cpp -o ./Tests/QTest.bin
```

QLib (template library):
```txt
c++ -O3 -shared -fPIC -I ./Source ./Example/QLib.cpp -o ./Build/QLib.so
```

### Engine:
More examples of the engine usage can be found @ [Tests/Test.hpp](https://github.com/HaniAmmar/Qentem-Engine/blob/master/Tests/Test.hpp)
